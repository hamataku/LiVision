# 親プロジェクトと異なる名前を使用
add_executable(fastls_example main.cpp)

target_include_directories(fastls_example 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# 親プロジェクトのライブラリをリンク
target_link_libraries(fastls_example PRIVATE fastls)