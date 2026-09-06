#pragma once

#include <bgfx/bgfx.h>

#include <cstdint>
#include <cstdio>
#include <string>

namespace livision::internal {

/**
 * @brief bgfx callback that writes screenshots and video captures.
 *
 * Screenshots are saved as PNG through bimg. Video frames are streamed to an
 * ffmpeg process over a pipe, so no encoder is linked into the library.
 * bgfx invokes these methods from the render thread during bgfx::frame().
 */
class CaptureCallback : public bgfx::CallbackI {
 public:
  ~CaptureCallback() override;

  /**
   * @brief Prepare to receive capture frames and remember the output path.
   *
   * The ffmpeg process is spawned in captureBegin(), once bgfx reports the
   * frame geometry.
   */
  void BeginRecording(std::string path, int fps);

  /**
   * @brief Stop forwarding frames and close the encoder.
   */
  void EndRecording();

  /**
   * @brief Whether a recording is currently requested or running.
   */
  bool IsRecording() const { return recording_; }

  /**
   * @brief Path of the recording being written, empty when idle.
   */
  const std::string& RecordingPath() const { return video_path_; }

  /**
   * @brief Number of frames handed to the encoder so far.
   */
  uint32_t RecordedFrames() const { return frames_written_; }

  // bgfx::CallbackI
  void fatal(const char* file_path, uint16_t line, bgfx::Fatal::Enum code,
             const char* str) override;
  void traceVargs(const char* file_path, uint16_t line, const char* format,
                  va_list arg_list) override;
  void profilerBegin(const char* name, uint32_t abgr, const char* file_path,
                     uint16_t line) override;
  void profilerBeginLiteral(const char* name, uint32_t abgr,
                            const char* file_path, uint16_t line) override;
  void profilerEnd() override;
  uint32_t cacheReadSize(uint64_t id) override;
  bool cacheRead(uint64_t id, void* data, uint32_t size) override;
  void cacheWrite(uint64_t id, const void* data, uint32_t size) override;
  void screenShot(const char* file_path, uint32_t width, uint32_t height,
                  uint32_t pitch, const void* data, uint32_t size,
                  bool yflip) override;
  void captureBegin(uint32_t width, uint32_t height, uint32_t pitch,
                    bgfx::TextureFormat::Enum format, bool yflip) override;
  void captureEnd() override;
  void captureFrame(const void* data, uint32_t size) override;

 private:
  void ClosePipe();
  void WriteFrame(const void* data, uint32_t size);

  bool recording_ = false;
  std::string video_path_;
  int fps_ = 30;

  std::FILE* pipe_ = nullptr;
  uint32_t width_ = 0;
  uint32_t height_ = 0;
  uint32_t pitch_ = 0;
  uint32_t frames_written_ = 0;
  int64_t start_us_ = 0;
};

}  // namespace livision::internal
