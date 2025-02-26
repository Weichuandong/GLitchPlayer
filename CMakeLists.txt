cmake_minimum_required(VERSION 3.10)
project(VideoPlayer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 查找依赖包
find_package(FFmpeg REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)

# 添加可执行文件
add_executable(${PROJECT_NAME}
        src/play.cpp
        src/FFmpegDecoder.cpp
        src/SDLRenderer.cpp
        src/VideoPlayer.cpp
        src/TextRenderer.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${SDL2_INCLUDE_DIRS}
        ${ffmpeg_INCLUDE_DIRS}
        ${SDL2_ttf_INCLUDE_DIRS}
)

# macOS 需要额外链接 CoreFoundation 和 FFmpeg 的特定库
target_link_libraries(${PROJECT_NAME} PRIVATE
        ${SDL2_LIBRARIES}
        ${SDL2_ttf_LIBRARIES}
        ${ffmpeg_LIBRARIES}
        "-framework CoreFoundation"  # 必须添加
)

file(COPY resources/fonts DESTINATION ${CMAKE_BINARY_DIR})