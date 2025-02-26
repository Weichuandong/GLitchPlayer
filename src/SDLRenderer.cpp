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
    }

    SDLRenderer::~SDLRenderer() {
        /* 释放 SDL 资源 */
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void SDLRenderer::render_frame(const uint8_t* rgb_data) {
        /* 更新纹理并渲染到屏幕 */
        SDL_UpdateTexture(texture, nullptr, rgb_data, width * 3);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    bool SDLRenderer::handle_events() {
        /* 处理关闭窗口等事件 */
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) return false;
        }
        return true;
    }

} // namespace video_player

