# 親プロジェクトと異なる名前を使用
add_executable(simulator_example main.cpp)

# 親プロジェクトのライブラリをリンク
target_link_libraries(simulator_example PRIVATE simulator)