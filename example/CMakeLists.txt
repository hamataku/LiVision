# 親プロジェクトと異なる名前を使用
add_executable(livision_example main.cpp)

target_include_directories(livision_example 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# 親プロジェクトのライブラリをリンク
target_link_libraries(livision_example PRIVATE livision)