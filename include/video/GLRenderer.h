//
// Created by Weichuandong on 2025/2/27.
//

#ifndef VIDEOPLAYER_GLRENDERER_H
#define VIDEOPLAYER_GLRENDERER_H

#pragma once
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL_ttf.h>
#include <functional>
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
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

        using SeekCallback = std::function<void(float)>;
        void setSeekCallback(const SeekCallback& callback);

        using FrameStepCallback = std::function<void(bool)>;
        void setFrameStepCallback(const FrameStepCallback& callback);
        // 添加获取暂停状态的回调
        using IsPausedCallback = std::function<bool()>;
        void setIsPausedCallback(const IsPausedCallback& callback);

        void render_ui(float progress, double current_time, double total_time,
                       bool is_paused, bool show_debug);
    private:
        void init_gl();
        void compile_shaders();
        void create_textures();

        // 进度条
        void init_ui_resources();
        void create_ui_shaders();
        void render_progress_bar(float progress);

        // 进度时间文本
        void init_text_renderer();
        std::string format_time(double seconds);
        void render_text(const std::string& text, float x, float y, const glm::vec4& color);

        // 窗口大小调整
        void update_projection(int width, int height);

        SDL_Window* window = nullptr;
        SDL_GLContext gl_context = nullptr;

        GLuint program = 0;
        GLuint y_tex = 0, u_tex = 0, v_tex = 0;
        GLuint vao = 0, vbo = 0;

        EventCallback eventCallback;
        SeekCallback seekCallback;
        FrameStepCallback frameStepCallback;
        IsPausedCallback isPausedCallback;

        // ui渲染相关资源
        GLuint ui_vao = 0;
        GLuint ui_vbo = 0;
        GLuint ui_program = 0;

        // 进度条参数
        struct {
            float height = 8.0f;
            glm::vec4 background_color {0.2f, 0.2f, 0.2f, 0.7f};
            glm::vec4 progress_color {0.86f, 0.12f, 0.12f, 1.0f};
        } progress_style;

        // 文本渲染相关
        TTF_Font* font = nullptr;
        GLuint text_texture = 0;
        GLuint text_vao = 0;
        GLuint text_vbo = 0;
        GLuint text_program = 0;


        // 用于调试的彩色矩形渲染方法
        void render_colored_rect(float x, float y, float width, float height, const glm::vec4& color);
    };


} // namespace video


#endif //VIDEOPLAYER_GLRENDERER_H
