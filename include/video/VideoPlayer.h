#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <memory>
#include <string>
#include "video/FFmpegDecoder.h"
#include "video/SDLRenderer.h"
#include "video/GLRenderer.h"
#include "logger.h"
#include "video/filters/FlipFilter.h"
#include "video/filters/MirrorFilter.h"
#include "video/filters/GrayscaleFilter.h"

namespace video {

    class VideoPlayer {
    public:
        explicit VideoPlayer(const std::string& filepath);
        void run(); // 启动播放循环

    private:
        std::unique_ptr<FFmpegDecoder> decoder;
        std::unique_ptr<GLRenderer> gl_renderer;
        std::unique_ptr<uint8_t[]> rgb_buffer;

        void handleKeyPress(SDL_Keycode key); // 新增键盘处理函数
        void handleSeek(float ration);

        bool is_paused = false; // 暂停状态
        double duration = 0.0;  // 视频总时长
        bool shouldQuit = false; //是否退出
        bool shouldDebug = true; //调试信息显示开关

        // 前进后退逻辑
        void step_forward_frame();
        void step_back_frame();
    };

} // namespace video

#endif //VIDEOPLAYER_H
