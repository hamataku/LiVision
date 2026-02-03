#include "livision/Renderer.hpp"

#include <bgfx/bgfx.h>
#include <bgfx/defines.h>
#include <bgfx/platform.h>
#include <bx/math.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "livision/internal/file_ops.hpp"
#include "livision/internal/mesh_data_access.hpp"

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
};

Renderer::Renderer() : pimpl_(std::make_unique<Impl>()) {}

Renderer::~Renderer() = default;

namespace {
inline bgfx::ShaderHandle CreateShader(const std::string& path,
                                       const char* name) {
  std::string shader;
  if (!internal::file_ops::ReadFile(path, shader)) {
    std::cerr << "Could not find shader: " << name << std::endl;
    std::exit(EXIT_FAILURE);
  }
  const bgfx::Memory* mem = bgfx::copy(shader.data(), shader.size());
  const bgfx::ShaderHandle handle = bgfx::createShader(mem);
  bgfx::setName(handle, name);
  return handle;
}
}  // namespace

void Renderer::Init() {
  const std::string shader_root = "shader/bin/";

#if BX_PLATFORM_WINDOWS
  const std::string plt_name = "win";
#elif BX_PLATFORM_OSX
  const std::string plt_name = "mac";
#elif BX_PLATFORM_LINUX
  const std::string plt_name = "linux";
#endif

  bgfx::ShaderHandle vsh =
      CreateShader(shader_root + "v_simple_" + plt_name + ".bin", "vshader");
  bgfx::ShaderHandle fsh =
      CreateShader(shader_root + "f_simple_" + plt_name + ".bin", "fshader");
  pimpl_->program_ = bgfx::createProgram(vsh, fsh, true);

  bgfx::ShaderHandle vph = CreateShader(
      shader_root + "v_points_" + plt_name + ".bin", "vshader_points");
  bgfx::ShaderHandle fph = CreateShader(
      shader_root + "f_points_" + plt_name + ".bin", "fshader_points");
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

void Renderer::Submit(const MeshData& mesh_data, const Eigen::Affine3d& mtx,
                      const Color& color) {
  bgfx::setState(kAlphaState);

  bgfx::setUniform(pimpl_->u_color_, &color.base);
  float mode_val[4] = {static_cast<float>(static_cast<int>(color.mode)), 0.0F,
                       0.0F, 0.0F};
  float rparams[4] = {color.rainbow.direction.x(), color.rainbow.direction.y(),
                      color.rainbow.direction.z(), color.rainbow.delta};
  bgfx::setUniform(pimpl_->u_color_mode_, mode_val);
  bgfx::setUniform(pimpl_->u_rainbow_params_, rparams);

  const Eigen::Matrix4d& eigen_mtx = mtx.matrix();
  glm::mat4 model_mtx(1.0F);
  for (int col = 0; col < 4; ++col) {
    for (int row = 0; row < 4; ++row) {
      model_mtx[col][row] = static_cast<float>(eigen_mtx(row, col));
    }
  }
  bgfx::setTransform(glm::value_ptr(model_mtx));

  const auto vbh = internal::MeshDataAccess::VertexBuffer(mesh_data);
  const auto ibh = internal::MeshDataAccess::IndexBuffer(mesh_data);
  bgfx::setVertexBuffer(0, vbh);
  bgfx::setIndexBuffer(ibh);
  bgfx::submit(0, pimpl_->program_);
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

  printf("- Vendor: %s (ID: 0x%04x)\n", vendor, caps->vendorId);
  printf("- Device ID: 0x%04x\n", caps->deviceId);

  std::cout << "- Graphic Backend: "
            << bgfx::getRendererName(bgfx::getRendererType()) << std::endl;
}

}  // namespace livision