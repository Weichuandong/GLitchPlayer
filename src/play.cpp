#include <iostream>
#include "video/videoPlayer.h"
#include "logger.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "用法: " << argv[0] << " <视频文件>" << std::endl;
        return 1;
    }
    Logger::init(true);

    LOG_INFO("启动播放器");
    LOG_INFO("当前工作目录: {}", std::filesystem::current_path().string());

    try {
        video::VideoPlayer player(argv[1]);
        player.run();

        LOG_INFO("播放器正常退出");
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}