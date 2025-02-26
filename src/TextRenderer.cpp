//
// Created by Weichuandong on 2025/2/26.
//

#include "video/TextRenderer.h"
#include <stdexcept>

namespace video {

    TextRenderer::TextRenderer(const char* font_path, int font_size) {
        if (TTF_Init() != 0) {
            throw std::runtime_error("TTF_Init failed");
        }
        font = TTF_OpenFont(font_path, font_size);
        if (!font) {
            throw std::runtime_error("无法加载字体文件");
        }
    }

    TextRenderer::~TextRenderer() {
        if (font) TTF_CloseFont(font);
        TTF_Quit();
    }

    SDL_Texture* TextRenderer::render_text(SDL_Renderer* renderer, const std::string& text, SDL_Color color) {
        SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
        if (!surface) return nullptr;

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        return texture;
    }
}

