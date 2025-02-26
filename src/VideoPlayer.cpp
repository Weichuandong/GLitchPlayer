//
// Created by WeiChuandong on 2025/2/26.
//

#include "video/VideoPlayer.h"

namespace video {
    VideoPlayer::VideoPlayer(const std::string& filepath)
        : decoder(std::make_unique<FFmpegDecoder>(filepath)),
          renderer(std::make_unique<SDLRenderer>(decoder->width(), decoder->height())),
          rgb_buffer(new uint8_t[decoder->width() * decoder->height() * 3]) {
        /* 初始化解码器和渲染器 */
        duration = decoder->duration();
    }

    void VideoPlayer::run() {
        /* 主循环：解码 + 渲染 */
        while (true) {
            PlayerEvent event = renderer->handle_events();

            switch (event) {
                case PlayerEvent::PlayPause: {
                    is_paused = !is_paused;
                    break;
                }
                case PlayerEvent::SeekBackWard_5: {
                    double current_time = decoder->get_current_pts();
                    decoder->seek(std::max(0.0, current_time - 5.0));
                    break;
                }
                case PlayerEvent::SeekForward_5: {
                    double current_time = decoder->get_current_pts();
                    decoder->seek(std::min(duration, current_time + 5.0));
                    break;
                }
                case PlayerEvent::Restart: {
                    decoder->seek(0.0);
                    break;
                }
                case PlayerEvent::Quit:
                    return;
                case PlayerEvent::None:
                    break;
            }
            if (!is_paused && decoder->get_next_frame((rgb_buffer.get()))) {
                renderer->render_frame(rgb_buffer.get());
                SDL_Delay(33);
            } else if (is_paused) {
                SDL_Delay(100);
            } else {
                break;
            }
        }
    }

} // namespace video
