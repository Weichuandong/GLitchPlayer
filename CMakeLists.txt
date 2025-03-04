cmake_minimum_required(VERSION 3.10)
project(VideoPlayer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 启用ASan检测
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

# 查找依赖包
find_package(FFmpeg REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(GLEW REQUIRED)
find_package(spdlog REQUIRED)


# 添加可执行文件
add_executable(${PROJECT_NAME}
        src/play.cpp
        src/FFmpegDecoder.cpp
        src/SDLRenderer.cpp
        src/VideoPlayer.cpp
        src/TextRenderer.cpp
        src/GLRenderer.cpp
        src/logger.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${SDL2_INCLUDE_DIRS}
        ${ffmpeg_INCLUDE_DIRS}
        ${SDL2_ttf_INCLUDE_DIRS}
        ${glew_INCLUDE_DIRS}
        ${spdlog_INCLUDE_DIRS}
)

# macOS 需要额外链接 CoreFoundation 和 FFmpeg 的特定库
target_link_libraries(${PROJECT_NAME} PRIVATE
        ${SDL2_LIBRARIES}
        ${SDL2_ttf_LIBRARIES}
        ${ffmpeg_LIBRARIES}
        ${glew_LIBRARIES}
        ${spdlog_LIBRARIES}
        "-framework CoreFoundation"  # 必须添加
)

file(COPY resources/fonts DESTINATION ${CMAKE_BINARY_DIR})