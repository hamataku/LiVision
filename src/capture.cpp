#include "livision/internal/capture.hpp"

#include <bimg/bimg.h>
#include <bx/file.h>
#include <bx/timer.h>

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <vector>

#include "livision/Log.hpp"

namespace livision::internal {

namespace {

// A frame that arrives after a long stall would otherwise be duplicated
// hundreds of times; cap it so a hitch costs a short freeze, not a huge file.
constexpr uint32_t kMaxDuplicatedFrames = 8;

std::string QuoteForShell(const std::string& path) {
  std::string out = "'";
  for (const char ch : path) {
    if (ch == '\'') {
      out += "'\\''";
    } else {
      out.push_back(ch);
    }
  }
  out.push_back('\'');
  return out;
}

std::string LowerExtension(const std::string& path) {
  std::string ext = std::filesystem::path(path).extension().string();
  std::transform(ext.begin(), ext.end(), ext.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return ext;
}

/**
 * @brief Build the ffmpeg command that reads raw BGRA frames on stdin.
 */
std::string BuildFfmpegCommand(const std::string& path, uint32_t width,
                               uint32_t height, int fps, bool yflip) {
  // yuv420p requires even dimensions, and the capture is bottom-up on some
  // backends, so both fixes go into one filter chain.
  std::string filters;
  if (yflip) {
    filters = "vflip,";
  }
  filters += "scale=trunc(iw/2)*2:trunc(ih/2)*2";

  std::string cmd = "ffmpeg -hide_banner -loglevel error -y";
  cmd += " -f rawvideo -pix_fmt bgra";
  cmd += " -s " + std::to_string(width) + "x" + std::to_string(height);
  cmd += " -r " + std::to_string(fps);
  cmd += " -i - -an";

  if (LowerExtension(path) == ".gif") {
    // A generated palette avoids the muddy colors of the default 216-color one.
    std::string gif_filters = yflip ? "vflip," : "";
    gif_filters += "split[a][b];[a]palettegen[p];[b][p]paletteuse";
    cmd += " -filter_complex " + QuoteForShell(gif_filters);
  } else {
    cmd += " -vf " + QuoteForShell(filters);
    cmd += " -c:v libx264 -preset veryfast -crf 23 -pix_fmt yuv420p";
    cmd += " -movflags +faststart";
  }

  cmd += " " + QuoteForShell(path);
  return cmd;
}

bool EnsureParentDirectory(const std::string& path) {
  const std::filesystem::path parent =
      std::filesystem::path(path).parent_path();
  if (parent.empty()) {
    return true;
  }
  std::error_code ec;
  std::filesystem::create_directories(parent, ec);
  if (ec) {
    LogMessage(LogLevel::Error, "Failed to create directory ", parent.string(),
               ": ", ec.message());
    return false;
  }
  return true;
}

}  // namespace

CaptureCallback::~CaptureCallback() { ClosePipe(); }

void CaptureCallback::BeginRecording(std::string path, int fps) {
  video_path_ = std::move(path);
  fps_ = fps > 0 ? fps : 30;
  recording_ = true;
}

void CaptureCallback::EndRecording() { recording_ = false; }

void CaptureCallback::ClosePipe() {
  if (pipe_ == nullptr) {
    return;
  }
  const int status = pclose(pipe_);
  pipe_ = nullptr;
  if (status != 0) {
    LogMessage(LogLevel::Error, "ffmpeg exited with status ", status,
               " while writing ", video_path_);
    return;
  }
  LogMessage(LogLevel::Info, "Saved recording: ", video_path_, " (",
             frames_written_, " frames)");
}

void CaptureCallback::screenShot(const char* file_path, uint32_t width,
                                 uint32_t height, uint32_t pitch,
                                 const void* data, uint32_t /*size*/,
                                 bool yflip) {
  if (file_path == nullptr || data == nullptr) {
    return;
  }
  if (!EnsureParentDirectory(file_path)) {
    return;
  }

  bx::FileWriter writer;
  bx::Error err;
  if (!bx::open(&writer, file_path, false, &err)) {
    LogMessage(LogLevel::Error, "Failed to open for writing: ", file_path);
    return;
  }

  bimg::imageWritePng(&writer, width, height, pitch, data,
                      bimg::TextureFormat::BGRA8, yflip, &err);
  bx::close(&writer);

  if (!err.isOk()) {
    LogMessage(LogLevel::Error, "Failed to encode screenshot: ", file_path);
    return;
  }
  LogMessage(LogLevel::Info, "Saved screenshot: ", file_path, " (", width, "x",
             height, ")");
}

void CaptureCallback::captureBegin(uint32_t width, uint32_t height,
                                   uint32_t pitch,
                                   bgfx::TextureFormat::Enum format,
                                   bool yflip) {
  ClosePipe();
  width_ = width;
  height_ = height;
  pitch_ = pitch;
  frames_written_ = 0;
  start_us_ = bx::getHPCounter() * 1000000 / bx::getHPFrequency();

  if (!recording_ || video_path_.empty()) {
    return;
  }
  if (format != bgfx::TextureFormat::BGRA8) {
    LogMessage(LogLevel::Error,
               "Unsupported capture format; recording disabled.");
    recording_ = false;
    return;
  }
  if (!EnsureParentDirectory(video_path_)) {
    recording_ = false;
    return;
  }

  const std::string cmd =
      BuildFfmpegCommand(video_path_, width, height, fps_, yflip);
  pipe_ = popen(cmd.c_str(), "w");
  if (pipe_ == nullptr) {
    LogMessage(LogLevel::Error,
               "Failed to start ffmpeg. Is it installed and on PATH?");
    recording_ = false;
    return;
  }
  LogMessage(LogLevel::Info, "Recording to ", video_path_, " at ", fps_,
             " fps (", width, "x", height, ")");
}

void CaptureCallback::captureEnd() { ClosePipe(); }

void CaptureCallback::WriteFrame(const void* data, uint32_t size) {
  if (pitch_ == width_ * 4) {
    std::fwrite(data, 1, size, pipe_);
    return;
  }
  // Strip row padding so ffmpeg receives a tightly packed frame.
  const auto* rows = static_cast<const uint8_t*>(data);
  for (uint32_t y = 0; y < height_; ++y) {
    std::fwrite(rows + (static_cast<size_t>(y) * pitch_), 1,
                static_cast<size_t>(width_) * 4, pipe_);
  }
}

void CaptureCallback::captureFrame(const void* data, uint32_t size) {
  if (pipe_ == nullptr || data == nullptr) {
    return;
  }

  // Pace output on the wall clock so the video plays at real speed whether the
  // scene renders faster or slower than the requested frame rate.
  const int64_t now_us = bx::getHPCounter() * 1000000 / bx::getHPFrequency();
  const int64_t elapsed_us = now_us - start_us_;
  const auto target =
      static_cast<uint32_t>((elapsed_us * fps_) / 1000000) + 1U;

  if (target <= frames_written_) {
    return;
  }
  const uint32_t count =
      std::min(target - frames_written_, kMaxDuplicatedFrames);
  for (uint32_t i = 0; i < count; ++i) {
    WriteFrame(data, size);
  }
  frames_written_ = target;
}

void CaptureCallback::fatal(const char* file_path, uint16_t line,
                            bgfx::Fatal::Enum code, const char* str) {
  if (code == bgfx::Fatal::DebugCheck) {
    LogMessage(LogLevel::Warn, "bgfx debug check: ", str);
    return;
  }
  LogMessage(LogLevel::Error, "bgfx fatal error at ", file_path, ":", line,
             ": ", str);
  abort();
}

void CaptureCallback::traceVargs(const char* /*file_path*/, uint16_t /*line*/,
                                 const char* format, va_list arg_list) {
  if (!ShouldLog(LogLevel::Debug)) {
    return;
  }
  va_list copy;
  va_copy(copy, arg_list);
  const int len = std::vsnprintf(nullptr, 0, format, copy);
  va_end(copy);
  if (len <= 0) {
    return;
  }
  std::vector<char> buf(static_cast<size_t>(len) + 1);
  std::vsnprintf(buf.data(), buf.size(), format, arg_list);
  std::string msg(buf.data());
  while (!msg.empty() && (msg.back() == '\n' || msg.back() == '\r')) {
    msg.pop_back();
  }
  LogMessage(LogLevel::Debug, "bgfx: ", msg);
}

void CaptureCallback::profilerBegin(const char* /*name*/, uint32_t /*abgr*/,
                                    const char* /*file_path*/,
                                    uint16_t /*line*/) {}
void CaptureCallback::profilerBeginLiteral(const char* /*name*/,
                                           uint32_t /*abgr*/,
                                           const char* /*file_path*/,
                                           uint16_t /*line*/) {}
void CaptureCallback::profilerEnd() {}

uint32_t CaptureCallback::cacheReadSize(uint64_t /*id*/) { return 0; }
bool CaptureCallback::cacheRead(uint64_t /*id*/, void* /*data*/,
                                uint32_t /*size*/) {
  return false;
}
void CaptureCallback::cacheWrite(uint64_t /*id*/, const void* /*data*/,
                                 uint32_t /*size*/) {}

}  // namespace livision::internal
