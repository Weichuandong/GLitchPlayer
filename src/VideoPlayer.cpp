//
// Created by WeiChuandong on 2025/2/26.
//

#include "video/VideoPlayer.h"

namespace video {
    VideoPlayer::VideoPlayer(const std::string& filepath)
        : decoder(std::make_unique<FFmpegDecoder>(filepath)),
          //renderer(std::make_unique<SDLRenderer>(decoder->width(), decoder->height())),
          gl_renderer(std::make_unique<GLRenderer>(decoder->width(), decoder->height())),
          rgb_buffer(new uint8_t[decoder->width() * decoder->height() * 3]) {
        /* 初始化解码器和渲染器 */
        duration = decoder->duration();

        // 绑定键盘事件回调
        gl_renderer->setEventCallback([this](SDL_Keycode key) {
            this->handleKeyPress(key);
        });
    }

    void VideoPlayer::run() {
        /* 主循环：解码 + 渲染 */
        while (!shouldQuit && gl_renderer->handle_events()) {
            FFmpegDecoder::YUVData yuvData{};

            if (!is_paused && decoder->get_next_frame(yuvData)) {
                gl_renderer->render_frame(yuvData.y_plane, yuvData.u_plane, yuvData.v_plane,
                                          yuvData.y_width, yuvData.y_height,
                                          yuvData.uv_width, yuvData.uv_height);
                SDL_Delay(33);
            } else if (is_paused) {
                SDL_Delay(100);
            } else {
                break;
            }
        }
//        while (true) {
//             PlayerEvent event = renderer->handle_events();
//
//            switch (event) {
//                case PlayerEvent::PlayPause: {
//                    is_paused = !is_paused;
//                    break;
//                }
//                case PlayerEvent::SeekBackWard_5: {
//                    double current_time = decoder->get_current_pts();
//                    decoder->seek(std::max(0.0, current_time - 5.0));
//                    break;
//                }
//                case PlayerEvent::SeekForward_5: {
//                    double current_time = decoder->get_current_pts();
//                    decoder->seek(std::min(duration, current_time + 5.0));
//                    break;
//                }
//                case PlayerEvent::Restart: {
//                    decoder->seek(0.0);
//                    break;
//                }
//                case PlayerEvent::Quit:
//                    return;
//                case PlayerEvent::None:
//                    break;
//            }
//        }
    }

    void VideoPlayer::handleKeyPress(SDL_Keycode key) {
        switch (key) {
            case SDLK_SPACE: {
                is_paused = !is_paused;
                if (is_paused) LOG_INFO("播放器暂停");
                else LOG_INFO("继续播放");
                break;
            }

            case SDLK_LEFT: {
                double current_time = decoder->get_current_pts();
                decoder->seek(std::max(0.0, current_time - 5.0));
                LOG_DEBUG("duration = {}, current_time = {}, 后退5S", duration, current_time);
                break;
            }
            case SDLK_RIGHT: {
                double current_time = decoder->get_current_pts();
                decoder->seek(std::min(duration, current_time + 5.0));
                LOG_DEBUG("duration = {}, current_time = {}, 前进5S", duration, current_time);
                break;
            }
            case SDLK_ESCAPE: {
                shouldQuit = true;
                LOG_DEBUG("ESC 退出");
                break;
            }
            case SDLK_BACKSPACE: {
                decoder->seek(0.0);
                LOG_DEBUG("从头播放");
                break;
            }
            default:
                break;
        }
    }

} // namespace video
