add_executable(${PROJECT_NAME} main.cpp)


# # 親プロジェクトで設定されたインクルードディレクトリを利用
# target_include_directories(${PROJECT_NAME} PRIVATE
#     "${CMAKE_CURRENT_SOURCE_DIR}/../include"
#     "${CMAKE_CURRENT_SOURCE_DIR}/../third-party"
# )

# # 親プロジェクトで設定されたリンクライブラリを利用
# target_link_libraries(${PROJECT_NAME} PRIVATE
#     glm::glm
#     SDL3::SDL3
#     bgfx
#     bx
#     imgui.cmake
# )

# # 必要に応じてコンパイルオプションを追加
# target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)