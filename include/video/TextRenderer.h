//
// Created by WeiChuandong on 2025/2/26.
//

#ifndef VIDEOPLAYER_TEXTRENDERER_H
#define VIDEOPLAYER_TEXTRENDERER_H

#include <SDL2/SDL_ttf.h>
#include <string>
#include <memory>

namespace video {

    class TextRenderer {
    public:
        TextRenderer(const char* font_path, int font_size);
        ~TextRenderer();

        SDL_Texture* render_text(SDL_Renderer* renderer, const std::string& text, SDL_Color color);

    private:
        TTF_Font* font = nullptr;
    };
}
#endif //VIDEOPLAYER_TEXTRENDERER_H
