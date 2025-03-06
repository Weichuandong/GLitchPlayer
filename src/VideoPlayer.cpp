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

        gl_renderer->setFrameStepCallback([this](bool forward) {
            if (forward) {
                step_forward_frame();
            } else {
                step_back_frame();
            }
        });

        gl_renderer->setIsPausedCallback([this](){
            return is_paused;
        });

        // 注册滤镜
        decoder->getFilterManager().registerFilter(std::make_shared<FlipFilter>(FlipFilter::VERTICAL));
        decoder->getFilterManager().registerFilter(std::make_shared<FlipFilter>(FlipFilter::HORIZONTAL));
        decoder->getFilterManager().registerFilter(std::make_shared<MirrorFilter>(MirrorFilter::HORIZONTAL));
        decoder->getFilterManager().registerFilter(std::make_shared<MirrorFilter>(MirrorFilter::VERTICAL));
        decoder->getFilterManager().registerFilter(std::make_shared<MirrorFilter>(MirrorFilter::QUAD));
        decoder->getFilterManager().registerFilter(std::make_shared<GrayscaleFilter>(1.0f));
        decoder->getFilterManager().registerFilter(std::make_shared<GrayscaleFilter>(0.5f));

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
                if (!is_paused) {
                    double current_time = decoder->get_current_pts();
                    decoder->seek(std::max(0.0, current_time - 5.0));
                    LOG_INFO("duration = {}, current_time = {}, 后退5S", duration, current_time);
                }
                break;
            }
            case SDLK_RIGHT: {
                if (!is_paused) {
                    double current_time = decoder->get_current_pts();
                    decoder->seek(std::min(duration, current_time + 5.0));
                    LOG_INFO("duration = {}, current_time = {}, 前进5S", duration, current_time);
                }
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
            case SDLK_1: {
                if (decoder->getFilterManager().isFilterExists("vflip")) {
                    decoder->getFilterManager().deactivateFilter("vflip");
                } else {
                    decoder->getFilterManager().activateFilter("vflip");
                }
                break;
            }
            case SDLK_2: {
                if (decoder->getFilterManager().isFilterExists("hflip")) {
                    decoder->getFilterManager().deactivateFilter("hflip");
                } else {
                    decoder->getFilterManager().activateFilter("hflip");
                }
                break;
            }
            case SDLK_3: {
                if (decoder->getFilterManager().isFilterExists("hmirror")) {
                    decoder->getFilterManager().deactivateFilter("hmirror");
                } else {
                    decoder->getFilterManager().activateFilter("hmirror");
                }
                break;
            }
            case SDLK_4: {
                if (decoder->getFilterManager().isFilterExists("vmirror")) {
                    decoder->getFilterManager().deactivateFilter("vmirror");
                } else {
                    decoder->getFilterManager().activateFilter("vmirror");
                }
                break;
            }
            case SDLK_5: {
                if (decoder->getFilterManager().isFilterExists("quadmirror")) {
                    decoder->getFilterManager().deactivateFilter("quadmirror");
                } else {
                    decoder->getFilterManager().activateFilter("quadmirror");
                }
                break;
            }
            case SDLK_6: {
                if (decoder->getFilterManager().isFilterExists("gray1.000000")) {
                    decoder->getFilterManager().deactivateFilter("gray1.000000");
                } else {
                    decoder->getFilterManager().activateFilter("gray1.000000");
                }
                break;
            }
            case SDLK_7: {
                if (decoder->getFilterManager().isFilterExists("gray0.500000")) {
                    decoder->getFilterManager().deactivateFilter("gray0.500000");
                } else {
                    decoder->getFilterManager().activateFilter("gray0.500000");
                }
                break;
            }

            case SDLK_0: {
                decoder->getFilterManager().deactivateAllFilter();
                break;
            }
            default:
                break;
        }
    }

    void VideoPlayer::step_forward_frame() {
        if (!is_paused) return;

        FFmpegDecoder::YUVData yuvData{};
        // 解码一帧并显示
        if (decoder->get_next_frame(yuvData)) {

            // 渲染帧
            gl_renderer->render_frame(
                    yuvData.frame->data[0], yuvData.frame->data[1], yuvData.frame->data[2],
                    yuvData.frame->width, yuvData.frame->height,
                    yuvData.frame->linesize[0], yuvData.frame->linesize[1]
                    );

            // 更新UI
            gl_renderer->render_ui(decoder->get_current_pts() / duration,
                                   decoder->get_current_pts(),
                                   duration,
                                   is_paused,
                                   false);
        }
    }

    void VideoPlayer::step_back_frame() {
        // 计算上一帧的大致位置（当前时间减去一帧的持续时间）
        double frame_duration = 1.0 / decoder->get_current_pts();
        double target_time = decoder->get_current_pts() - frame_duration;

        if (target_time < 0) target_time = 0;

        // 跳转到目标位置
        decoder->seek(target_time);

        // 解码并显示该帧
        FFmpegDecoder::YUVData yuvData{};
        // 解码一帧并显示
        if (decoder->get_next_frame(yuvData)) {

            // 渲染帧
            gl_renderer->render_frame(
                    yuvData.frame->data[0], yuvData.frame->data[1], yuvData.frame->data[2],
                    yuvData.frame->width, yuvData.frame->height,
                    yuvData.frame->linesize[0], yuvData.frame->linesize[1]
            );

            // 更新UI
            gl_renderer->render_ui(decoder->get_current_pts() / duration,
                                   decoder->get_current_pts(),
                                   duration,
                                   is_paused,
                                   false);
        }
    }

} // namespace video
