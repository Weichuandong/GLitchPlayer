#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <memory>
#include <string>
#include "video/FFmpegDecoder.h"
#include "video/SDLRenderer.h"

namespace video {

    class VideoPlayer {
    public:
        explicit VideoPlayer(const std::string& filepath);
        void run(); // 启动播放循环

    private:
        std::unique_ptr<FFmpegDecoder> decoder;
        std::unique_ptr<SDLRenderer> renderer;
        std::unique_ptr<uint8_t[]> rgb_buffer;
    };

} // namespace video

#endif //VIDEOPLAYER_H
