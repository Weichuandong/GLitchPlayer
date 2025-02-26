//
// Created by WeiChuandong on 2025/2/26.
//

#include "video/SDLRenderer.h"

namespace video {

    SDLRenderer::SDLRenderer(int width, int height) :
        width(width),
        height(height){
        /* 初始化 SDL 窗口和渲染器 */
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            throw std::runtime_error(SDL_GetError());
        }

        window = SDL_CreateWindow(
            "视频播放器",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            width, height,
            SDL_WINDOW_SHOWN
        );
        if (!window) {
            throw std::runtime_error(SDL_GetError());
        }

        renderer = SDL_CreateRenderer(window, -1, 0);
        if (!renderer) {
            throw std::runtime_error(SDL_GetError());
        }

        texture = SDL_CreateTexture(
            renderer, SDL_PIXELFORMAT_RGB24,
            SDL_TEXTUREACCESS_STREAMING,
            width, height
        );
        if (!texture) {
            throw std::runtime_error(SDL_GetError());
        }

        text_renderer = std::make_unique<TextRenderer>("fonts/Roboto-Regular.ttf", 20);
    }

    SDLRenderer::~SDLRenderer() {
        /* 释放 SDL 资源 */
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void SDLRenderer::render_frame(const uint8_t* rgb_data, double current_time, double duration) {
        /* 更新纹理并渲染到屏幕 */
        SDL_UpdateTexture(texture, nullptr, rgb_data, width * 3);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);

        // 绘制进度条
        draw_progress_bar(current_time, duration);

        SDL_RenderPresent(renderer);
    }

    PlayerEvent SDLRenderer::handle_events() {
        /* 处理关闭窗口等事件 */
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return PlayerEvent::Quit;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_SPACE : return PlayerEvent::PlayPause;
                    case SDLK_LEFT : return PlayerEvent::SeekBackWard_5;
                    case SDLK_RIGHT : return PlayerEvent::SeekForward_5;
                    case SDLK_ESCAPE : return PlayerEvent::Quit;
                    case SDLK_BACKSPACE : return PlayerEvent::Restart;
                    default: break;
                }
            }
        }
        return PlayerEvent::None;
    }

    // 辅助函数：格式化时间（秒 → "MM:SS"）
    std::string SDLRenderer::format_time(double seconds) {
        int mins = static_cast<int>(seconds) / 60;
        int secs = static_cast<int>(seconds) % 60;
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%02d:%02d", mins, secs);
        return buffer;
    }

    void SDLRenderer::draw_progress_bar(double current_time, double duration) {
        int window_width, window_height;
        SDL_GetWindowSize(window, &window_width, &window_height);

        // ---------- 绘制进度条背景 ----------
        SDL_Rect progress_bar_rect = {0, window_height - 40, window_width, 4};
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255); // 灰色背景
        SDL_RenderFillRect(renderer, &progress_bar_rect);

        // ---------- 绘制进度条前景 ----------
        if (duration > 0) {
            double progress = current_time / duration;
            int progress_width = static_cast<int>(window_width * progress);
            SDL_Rect progress_rect = {0, window_height - 40, progress_width, 4};
            SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255); // 绿色进度条
            SDL_RenderFillRect(renderer, &progress_rect);
        }

        // ---------- 绘制时间文本 ----------
        SDL_Color text_color = {255, 255, 255, 255}; // 白色文本
        std::string time_text = format_time(current_time) + " / " + format_time(duration);
        SDL_Texture* text_texture = text_renderer->render_text(renderer, time_text, text_color);

        if (text_texture) {
            int text_width, text_height;
            SDL_QueryTexture(text_texture, nullptr, nullptr, &text_width, &text_height);
            SDL_Rect text_rect = {10, window_height - 35, text_width, text_height};
            SDL_RenderCopy(renderer, text_texture, nullptr, &text_rect);
            SDL_DestroyTexture(text_texture);
        }
    }


} // namespace video