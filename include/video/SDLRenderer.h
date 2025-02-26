#ifndef SDLRENDERER_H
#define SDLRENDERER_H

#include <SDL2/SDL.h>
#include <stdexcept>

#include "PlayerEvent.h"

namespace video {

    class SDLRenderer {
    public:
        SDLRenderer(int width, int height);
        ~SDLRenderer();

        void render_frame(const uint8_t* rgb_data);  // 渲染一帧 RGB 数据
        PlayerEvent handle_events();                 // 处理窗口事件

    private:
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;
        SDL_Texture* texture = nullptr;

        uint16_t width;
        uint16_t height;
    };

} // namespace video


#endif //SDLRENDERER_H
