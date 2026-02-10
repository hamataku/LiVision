rm -rf dist/
paths=(
  CMakeLists.txt
  cmake
  debian
  include
  internal
  src
  shader/bin
  third-party/bgfx-imgui
  third-party/bgfx.cmake/CMakeLists.txt
  third-party/bgfx.cmake/cmake
  third-party/bgfx.cmake/bgfx/LICENSE
  third-party/bgfx.cmake/bgfx/include
  third-party/bgfx.cmake/bgfx/src
  third-party/bgfx.cmake/bgfx/scripts
  third-party/bgfx.cmake/bgfx/3rdparty
  third-party/bgfx.cmake/bimg/LICENSE
  third-party/bgfx.cmake/bimg/include
  third-party/bgfx.cmake/bimg/src
  third-party/bgfx.cmake/bimg/3rdparty
  third-party/bgfx.cmake/bx/LICENSE
  third-party/bgfx.cmake/bx/include
  third-party/bgfx.cmake/bx/src
  third-party/bgfx.cmake/bx/3rdparty
)

{
  for path in "${paths[@]}"; do
    if [[ -d "$path" ]]; then
      find "$path" -mindepth 0 -print0
    elif [[ -f "$path" ]]; then
      printf '%s\0' "$path"
    fi
  done
} | rsync -a --delete --prune-empty-dirs --from0 --files-from=- \
  ./ dist/

tar -cJf livision_0.1.0.orig.tar.xz -C dist ./