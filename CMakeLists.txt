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
find_package(glm REQUIRED)

add_definitions(-D__STDC_CONSTANT_MACROS)
add_definitions(-D__STDC_LIMIT_MACROS)
add_definitions(-D__STDC_FORMAT_MACROS)

# 添加可执行文件
add_executable(${PROJECT_NAME}
        src/play.cpp
        src/FFmpegDecoder.cpp
        src/SDLRenderer.cpp
        src/VideoPlayer.cpp
        src/TextRenderer.cpp
        src/GLRenderer.cpp
        src/logger.cpp
        src/filters/FilterManager.cpp
        src/filters/FlipFilter.cpp
        src/filters/GrayscaleFilter.cpp
        src/filters/MirrorFilter.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
         ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        sdl_ttf::sdl_ttf
        SDL2::SDL2main
        ffmpeg::ffmpeg
        GLEW::GLEW
        spdlog::spdlog
        glm::glm
        "-framework CoreFoundation"  # 必须添加
)

file(COPY resources/fonts DESTINATION ${CMAKE_BINARY_DIR})