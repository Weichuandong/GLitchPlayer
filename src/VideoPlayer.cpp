//
// Created by WeiChuandong on 2025/2/26.
//

#include "video/VideoPlayer.h"

namespace video {
    VideoPlayer::VideoPlayer(const std::string& filepath)
        : decoder(std::make_unique<FFmpegDecoder>(filepath)),
          gl_renderer(std::make_unique<GLRenderer>(decoder->width(), decoder->height())),
          rgb_buffer(new uint8_t[decoder->width() * decoder->height() * 3]) {
        // 视频时长信息
        duration = decoder->duration();
        // 绑定键盘事件回调
        gl_renderer->setEventCallback([this](SDL_Keycode key) {
            this->handleKeyPress(key);
        });

        gl_renderer->setSeekCallback([this](float ratio) {
            this->handleSeek(ratio);
        });

        LOG_INFO("初始化播放器: {} ({}x{}), 时长: {:.2f}s",
                 filepath,
                 decoder->width(),
                 decoder->height(),
                 duration
        );
    }

    void VideoPlayer::run() {
        /* 主循环：解码 + 渲染 */
        while (!shouldQuit && gl_renderer->handle_events()) {
            FFmpegDecoder::YUVData yuvData{};

            bool frame_available = !is_paused && decoder->get_next_frame(yuvData);
            if (frame_available) {
                gl_renderer->render_frame(yuvData.frame->data[0], yuvData.frame->data[1], yuvData.frame->data[2],
                                          yuvData.frame->width, yuvData.frame->height,
                                          yuvData.frame->linesize[0], yuvData.frame->linesize[1]);

                gl_renderer->render_ui(decoder->get_current_pts() / duration,
                                       decoder->get_current_pts(),
                                       duration,
                                       is_paused,
                                       false);
                SDL_Delay(33);
            } else if (is_paused) {
                SDL_Delay(100);
            } else {
                break;
            }
        }
    }

    void VideoPlayer::handleSeek(float ration) {
        const double target_time = ration * duration;
        decoder->seek(target_time);
        is_paused = false;
        LOG_DEBUG("Seek to: {:.2f}s (ration={})", target_time, ration);
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
                LOG_INFO("duration = {}, current_time = {}, 后退5S", duration, current_time);
                break;
            }
            case SDLK_RIGHT: {
                double current_time = decoder->get_current_pts();
                decoder->seek(std::min(duration, current_time + 5.0));
                LOG_INFO("duration = {}, current_time = {}, 前进5S", duration, current_time);
                break;
            }
            case SDLK_ESCAPE: {
                shouldQuit = true;
                LOG_INFO("ESC 退出");
                break;
            }
            case SDLK_BACKSPACE: {
                decoder->seek(0.0);
                LOG_INFO("从头播放");
                break;
            }
            default:
                break;
        }
    }

} // namespace video
