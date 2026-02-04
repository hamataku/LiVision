#include "livision/Renderer.hpp"

#include <bgfx/bgfx.h>
#include <bgfx/defines.h>
#include <bgfx/platform.h>
#include <bx/math.h>

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "livision/internal/file_ops.hpp"
#include "livision/internal/mesh_buffer_access.hpp"

namespace livision {

static constexpr uint64_t kAlphaState =
    BGFX_STATE_DEFAULT | BGFX_STATE_BLEND_ALPHA;
static constexpr uint64_t kPointState = kAlphaState | BGFX_STATE_PT_POINTS;
static constexpr uint64_t kPointSpriteState =
    kAlphaState | BGFX_STATE_PT_TRISTRIP;

struct Renderer::Impl {
  bgfx::ProgramHandle program_;
  bgfx::ProgramHandle point_program_;

  bgfx::UniformHandle u_color_;
  bgfx::UniformHandle u_color_mode_;
  bgfx::UniformHandle u_rainbow_params_;

  std::vector<std::string> shader_search_paths_;
};

Renderer::Renderer() : pimpl_(std::make_unique<Impl>()) {}

Renderer::~Renderer() = default;

namespace {
std::vector<std::string> SplitPaths(const std::string& paths) {
#if BX_PLATFORM_WINDOWS
  const char delimiter = ';';
#else
  const char delimiter = ':';
#endif

  std::vector<std::string> result;
  std::string current;
  for (const char ch : paths) {
    if (ch == delimiter) {
      if (!current.empty()) {
        result.push_back(current);
        current.clear();
      }
      continue;
    }
    current.push_back(ch);
  }
  if (!current.empty()) {
    result.push_back(current);
  }
  return result;
}

std::vector<std::string> CollectShaderSearchPaths(
    const std::vector<std::string>& user_paths) {
  std::vector<std::string> paths = user_paths;

  if (const char* env = std::getenv("LIVISION_SHADER_PATHS")) {
    const std::vector<std::string> env_paths = SplitPaths(env);
    paths.insert(paths.end(), env_paths.begin(), env_paths.end());
  }

#ifdef LIVISION_SHADER_SOURCE_DIR
  paths.emplace_back(LIVISION_SHADER_SOURCE_DIR);
#endif

#ifdef LIVISION_SHADER_INSTALL_DIR
  paths.emplace_back(LIVISION_SHADER_INSTALL_DIR);
#endif

  paths.emplace_back("shader/bin");
  return paths;
}

bgfx::ShaderHandle CreateShaderFromPaths(
    const std::string& file_name, const char* name,
    const std::vector<std::string>& search_paths) {
  std::string shader;
  for (const std::string& base : search_paths) {
    std::string path = base;
    path += "/";
    path += file_name;
    if (internal::file_ops::ReadFile(path, shader)) {
      const bgfx::Memory* mem = bgfx::copy(shader.data(), shader.size());
      const bgfx::ShaderHandle handle = bgfx::createShader(mem);
      bgfx::setName(handle, name);
      return handle;
    }
  }

  std::cerr << "Could not find shader: " << name << std::endl;
  std::cerr << "Search paths:" << std::endl;
  for (const std::string& base : search_paths) {
    std::cerr << "  - " << base << std::endl;
  }
  std::exit(EXIT_FAILURE);
}
}  // namespace

void Renderer::Init() {
#if BX_PLATFORM_WINDOWS
  const std::string plt_name = "win";
#elif BX_PLATFORM_OSX
  const std::string plt_name = "mac";
#elif BX_PLATFORM_LINUX
  const std::string plt_name = "linux";
#endif

  const std::vector<std::string> search_paths =
      CollectShaderSearchPaths(pimpl_->shader_search_paths_);

  bgfx::ShaderHandle vsh = CreateShaderFromPaths(
      "v_simple_" + plt_name + ".bin", "vshader", search_paths);
  bgfx::ShaderHandle fsh = CreateShaderFromPaths(
      "f_simple_" + plt_name + ".bin", "fshader", search_paths);
  pimpl_->program_ = bgfx::createProgram(vsh, fsh, true);

  bgfx::ShaderHandle vph = CreateShaderFromPaths(
      "v_points_" + plt_name + ".bin", "vshader_points", search_paths);
  bgfx::ShaderHandle fph = CreateShaderFromPaths(
      "f_points_" + plt_name + ".bin", "fshader_points", search_paths);
  if (bgfx::isValid(vph) && bgfx::isValid(fph)) {
    pimpl_->point_program_ = bgfx::createProgram(vph, fph, true);
  }

  PrintBackend();

  pimpl_->u_color_ = bgfx::createUniform("u_color", bgfx::UniformType::Vec4);
  pimpl_->u_color_mode_ =
      bgfx::createUniform("u_color_mode", bgfx::UniformType::Vec4);
  pimpl_->u_rainbow_params_ =
      bgfx::createUniform("u_rainbow_params", bgfx::UniformType::Vec4);
}

void Renderer::DeInit() {
  bgfx::destroy(pimpl_->program_);
  pimpl_->program_ = BGFX_INVALID_HANDLE;
  bgfx::destroy(pimpl_->point_program_);
  pimpl_->point_program_ = BGFX_INVALID_HANDLE;

  bgfx::destroy(pimpl_->u_color_);
  bgfx::destroy(pimpl_->u_color_mode_);
  bgfx::destroy(pimpl_->u_rainbow_params_);
}

void Renderer::SetShaderSearchPaths(std::vector<std::string> paths) {
  pimpl_->shader_search_paths_ = std::move(paths);
}

void Renderer::Submit(MeshBuffer& mesh_buffer, const Eigen::Affine3d& mtx,
                      const Color& color, const Color& wire_color) {
  if (color.mode != Color::ColorMode::InVisible) {
    bgfx::setState(kAlphaState);
    bgfx::setUniform(pimpl_->u_color_, &color.base);
    float mode_val[4] = {static_cast<float>(static_cast<int>(color.mode)), 0.0F,
                         0.0F, 0.0F};
    float rparams[4] = {color.rainbow.direction.x(),
                        color.rainbow.direction.y(),
                        color.rainbow.direction.z(), color.rainbow.delta};
    bgfx::setUniform(pimpl_->u_color_mode_, mode_val);
    bgfx::setUniform(pimpl_->u_rainbow_params_, rparams);

    const Eigen::Matrix4d& eigen_mtx = mtx.matrix();
    float model_mtx[16];
    for (int col = 0; col < 4; ++col) {
      for (int row = 0; row < 4; ++row) {
        model_mtx[(col * 4) + row] = static_cast<float>(eigen_mtx(row, col));
      }
    }
    bgfx::setTransform(model_mtx);
    const auto vbh = internal::MeshBufferAccess::VertexBuffer(mesh_buffer);
    const auto ibh = internal::MeshBufferAccess::IndexBuffer(mesh_buffer);
    bgfx::setVertexBuffer(0, vbh);
    bgfx::setIndexBuffer(ibh);
    bgfx::submit(0, pimpl_->program_);
  }

  if (wire_color.mode != Color::ColorMode::InVisible) {
    bgfx::setState((kAlphaState & ~BGFX_STATE_PT_MASK) | BGFX_STATE_PT_LINES);
    bgfx::setUniform(pimpl_->u_color_, &wire_color.base);
    float mode_val[4] = {static_cast<float>(static_cast<int>(wire_color.mode)),
                         0.0F, 0.0F, 0.0F};
    float rparams[4] = {
        wire_color.rainbow.direction.x(), wire_color.rainbow.direction.y(),
        wire_color.rainbow.direction.z(), wire_color.rainbow.delta};
    bgfx::setUniform(pimpl_->u_color_mode_, mode_val);
    bgfx::setUniform(pimpl_->u_rainbow_params_, rparams);

    const Eigen::Matrix4d& eigen_mtx = mtx.matrix();
    float model_mtx[16];
    for (int col = 0; col < 4; ++col) {
      for (int row = 0; row < 4; ++row) {
        model_mtx[(col * 4) + row] = static_cast<float>(eigen_mtx(row, col));
      }
    }
    bgfx::setTransform(model_mtx);
    const auto vbh = internal::MeshBufferAccess::VertexBuffer(mesh_buffer);
    const auto ibh = internal::MeshBufferAccess::WireIndexBuffer(mesh_buffer);
    bgfx::setVertexBuffer(0, vbh);
    bgfx::setIndexBuffer(ibh);
    bgfx::submit(0, pimpl_->program_);
  }
}

void Renderer::PrintBackend() {
  const bgfx::Caps* caps = bgfx::getCaps();

  // ベンダーIDからベンダー名を特定
  const char* vendor = "Unknown";
  if (caps->vendorId == BGFX_PCI_ID_AMD) {
    vendor = "AMD";
  } else if (caps->vendorId == BGFX_PCI_ID_INTEL) {
    vendor = "Intel";
  } else if (caps->vendorId == BGFX_PCI_ID_NVIDIA) {
    vendor = "NVIDIA";
  } else if (caps->vendorId == BGFX_PCI_ID_MICROSOFT) {
    vendor = "Microsoft";
  } else if (caps->vendorId == BGFX_PCI_ID_ARM) {
    vendor = "ARM";
  } else if (caps->vendorId == BGFX_PCI_ID_APPLE) {
    vendor = "Apple";
  } else if (caps->vendorId == BGFX_PCI_ID_SOFTWARE_RASTERIZER) {
    vendor = "Software Rasterizer";
  }

  printf("[LiVision] Vendor: %s (ID: 0x%04x)\n", vendor, caps->vendorId);
  printf("[LiVision] Device ID: 0x%04x\n", caps->deviceId);

  std::cout << "[LiVision] Graphic Backend: "
            << bgfx::getRendererName(bgfx::getRendererType()) << std::endl;
}

}  // namespace livision