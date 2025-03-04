#ifndef SDLRENDERER_H
#define SDLRENDERER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <memory>
#include <stdexcept>

#include "video/TextRenderer.h"
#include "PlayerEvent.h"

namespace video {

    class SDLRenderer {
    public:
        SDLRenderer(int width, int height);
        ~SDLRenderer();

        void render_frame(const uint8_t* rgb_data, double current_time, double duration);  // 渲染一帧 RGB 数据
        PlayerEvent handle_events();                 // 处理窗口事件

    private:
        void draw_progress_bar(double current_time, double duration);  // 绘制进度条
        std::string format_time(double seconds);

        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;
        SDL_Texture* texture = nullptr;
        std::unique_ptr<TextRenderer> text_renderer;

        uint16_t width;
        uint16_t height;
    };

} // namespace video


#endif //SDLRENDERER_H
