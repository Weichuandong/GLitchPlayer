//
// Created by WeiChuandong on 2025/2/26.
//

#include "video/VideoPlayer.h"

namespace video {
    VideoPlayer::VideoPlayer(const std::string& filepath)
        : decoder(std::make_unique<FFmpegDecoder>(filepath)),
          renderer(std::make_unique<SDLRenderer>(decoder->width(), decoder->height())),
          rgb_buffer(new uint8_t[decoder->width() * decoder->height() * 3])
    { /* 初始化解码器和渲染器 */ }

    void VideoPlayer::run() {
        /* 主循环：解码 + 渲染 */
        while (renderer->handle_events()) {
            if (decoder->get_next_frame(rgb_buffer.get())) {
                renderer->render_frame(rgb_buffer.get());
                SDL_Delay(33); // 约30 FPS
            } else {
                break; // 播放结束
            }
        }
    }

} // namespace video
