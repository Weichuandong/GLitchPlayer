#include <iostream>
#include "video/videoPlayer.h"
#include "logger.h"

void test_asan_use_after_free() {
    int* array = new int[100];
    delete[] array;
    array[0] = 1; // 明显的 Use-after-free
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "用法: " << argv[0] << " <视频文件>" << std::endl;
        return 1;
    }
    test_asan_use_after_free();
    try {
        Logger::init(true);
        LOG_INFO("启动播放器");

        video::VideoPlayer player(argv[1]);
        player.run();

        LOG_INFO("播放器正常退出");
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}