//
// Created by Weichuandong on 2025/2/27.
//

#ifndef VIDEOPLAYER_GLRENDERER_H
#define VIDEOPLAYER_GLRENDERER_H

#pragma once
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <functional>

#include "logger.h"

namespace video {

    class GLRenderer {
    public:
        GLRenderer(int width, int height);
        ~GLRenderer();

        void render_frame(const uint8_t* y_plane, const uint8_t* u_plane, const uint8_t* v_plane,
                          int y_width, int y_height, int uv_width, int uv_height);
        bool handle_events();

        using EventCallback = std::function<void(SDL_KeyCode)>;
        void setEventCallback(const EventCallback& callback);

    private:
        void init_gl();
        void compile_shaders();
        void create_textures();

        SDL_Window* window = nullptr;
        SDL_GLContext gl_context = nullptr;

        GLuint program = 0;
        GLuint y_tex = 0, u_tex = 0, v_tex = 0;
        GLuint vao = 0, vbo = 0;

        EventCallback eventCallback;
    };

} // namespace video


#endif //VIDEOPLAYER_GLRENDERER_H
