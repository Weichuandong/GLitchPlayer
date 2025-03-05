//
// Created by Weichuandong on 2025/2/27.
//

#include "video/GLRenderer.h"

#include <iostream>
#include <fstream>
#include <vector>

namespace video {

// Vertex Shader（传递纹理坐标）
    const char* vs_source = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;
out vec2 TexCoord;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)";

// Fragment Shader（YUV→RGB转换）
    const char* fs_source = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D y_tex;
uniform sampler2D u_tex;
uniform sampler2D v_tex;

void main() {
    float y = texture(y_tex, TexCoord).r;
    float u = texture(u_tex, TexCoord).r - 0.5;
    float v = texture(v_tex, TexCoord).r - 0.5;

    float r = y + 1.402 * v;
    float g = y - 0.344136 * u - 0.714136 * v;
    float b = y + 1.772 * u;

    FragColor = vec4(r, g, b, 1.0);
}
)";

    const char* ui_vertex_shader = R"(
#version 330 core
layout(location=0) in vec2 aPos;
uniform mat4 projection;
void main() {
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
}
)";

    const char* ui_fragment_shader = R"(
#version 330 core
uniform vec4 uColor;
out vec4 FragColor;
void main() {
    FragColor = uColor;
}
)";

    // 添加文本渲染着色器
    const char* text_vertex_shader = R"(
#version 330 core
layout(location=0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;
uniform mat4 projection;

void main() {
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
)";

    const char* text_fragment_shader = R"(
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D text;
uniform vec4 textColor;

void main() {
    vec4 sampled = texture(text, TexCoords);
    FragColor = vec4(textColor.rgb, textColor.a * sampled.a);
}
)";


    void GLRenderer::init_text_renderer() {
        // 初始化SDL_ttf
        if (TTF_Init() == -1) {
            LOG_ERROR("SDL_ttf初始化失败: {}", TTF_GetError());
            return;
        }

        // 加载字体 - 确保路径正确，这里使用默认大小16pt
        font = TTF_OpenFont("./fonts/Roboto-Regular.ttf", 14);
        if (!font) {
            LOG_ERROR("无法加载字体: {}", TTF_GetError());
            return;
        }

        // 编译着色器
        GLuint vs, fs;
        vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &text_vertex_shader, nullptr);
        glCompileShader(vs);

        // 检查着色器编译
        GLint success;
        glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(vs, 512, nullptr, infoLog);
            LOG_ERROR("文本顶点着色器编译失败: {}", infoLog);
        }

        fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &text_fragment_shader, nullptr);
        glCompileShader(fs);

        glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(fs, 512, nullptr, infoLog);
            LOG_ERROR("文本片段着色器编译失败: {}", infoLog);
        }

        text_program = glCreateProgram();
        glAttachShader(text_program, vs);
        glAttachShader(text_program, fs);
        glLinkProgram(text_program);

        glGetProgramiv(text_program, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(text_program, 512, nullptr, infoLog);
            LOG_ERROR("文本着色器程序链接失败: {}", infoLog);
        }

        glDeleteShader(vs);
        glDeleteShader(fs);

        // 创建VAO/VBO
        glGenVertexArrays(1, &text_vao);
        glGenBuffers(1, &text_vbo);
        glBindVertexArray(text_vao);
        glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // 创建文本纹理
        glGenTextures(1, &text_texture);
        glBindTexture(GL_TEXTURE_2D, text_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    std::string GLRenderer::format_time(double seconds) {
        int total_seconds = static_cast<int>(seconds);
        int hours = total_seconds / 3600;
        int minutes = (total_seconds % 3600) / 60;
        int secs = total_seconds % 60;

        std::stringstream ss;
        if (hours > 0) {
            ss << hours << ":";
        }
        ss << std::setfill('0') << std::setw(2) << minutes << ":"
           << std::setfill('0') << std::setw(2) << secs;
        return ss.str();
    }

    void GLRenderer::render_text(const std::string& text, float x, float y, const glm::vec4& color) {
        if (!font) return;

        // 渲染文本到SDL表面
        SDL_Color sdl_color = {
                static_cast<Uint8>(color.r * 255),
                static_cast<Uint8>(color.g * 255),
                static_cast<Uint8>(color.b * 255),
                static_cast<Uint8>(color.a * 255)
        };


        SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), sdl_color);
        if (!surface) {
            LOG_ERROR("文本渲染失败: {}", TTF_GetError());
            return;
        }

        // 转换为适合OpenGL的格式 (RGBA)
        SDL_Surface* rgbaSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
        if (!rgbaSurface) {
            LOG_ERROR("表面格式转换失败: {}", SDL_GetError());
            SDL_FreeSurface(surface);
            return;
        }

        // 创建纹理
        glBindTexture(GL_TEXTURE_2D, text_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rgbaSurface->w, rgbaSurface->h, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, rgbaSurface->pixels);

        // 获取窗口尺寸
        int w, h;
        SDL_GetWindowSize(window, &w, &h);

        // 设置正交投影
        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(w), static_cast<float>(h), 0.0f);

        // 启用混合
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // 配置着色器
        glUseProgram(text_program);
        glUniform1i(glGetUniformLocation(text_program, "text"), 0);
        glUniformMatrix4fv(glGetUniformLocation(text_program, "projection"), 1, GL_FALSE, &projection[0][0]);
        glUniform4fv(glGetUniformLocation(text_program, "textColor"), 1, &color[0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, text_texture);
        glBindVertexArray(text_vao);

        // 计算文本渲染的顶点
        float xpos = x;
        float ypos = y;
        float w_text = static_cast<float>(rgbaSurface->w);
        float h_text = static_cast<float>(rgbaSurface->h);

        // 更新VBO
        float vertices[6][4] = {
                { xpos,     ypos + h_text,   0.0f, 1.0f },
                { xpos,     ypos,            0.0f, 0.0f },
                { xpos + w_text, ypos,       1.0f, 0.0f },

                { xpos,     ypos + h_text,   0.0f, 1.0f },
                { xpos + w_text, ypos,       1.0f, 0.0f },
                { xpos + w_text, ypos + h_text, 1.0f, 1.0f }
        };

        // 更新VBO内存
        glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // 渲染文本四边形
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 清理
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        SDL_FreeSurface(surface);
        SDL_FreeSurface(rgbaSurface);
    }

    GLRenderer::GLRenderer(int width, int height) {
        // 初始化 SDL 窗口和 OpenGL 上下文
        SDL_Init(SDL_INIT_VIDEO);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

        window = SDL_CreateWindow("OpenGL Video Player",
                                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  width, height,
                                  SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

        gl_context = SDL_GL_CreateContext(window);
        glewInit();

        init_gl();

        init_ui_resources();

        init_text_renderer();
    }

    GLRenderer::~GLRenderer() {
        // 清理UI资源
        glDeleteVertexArrays(1, &ui_vao);
        glDeleteBuffers(1, &ui_vbo);
        glDeleteProgram(ui_program);

        // 清理视频纹理
        glDeleteTextures(1, &y_tex);
        glDeleteTextures(1, &u_tex);
        glDeleteTextures(1, &v_tex);
        glDeleteProgram(program);

        // 清理文本渲染资源
        if (font) {
            TTF_CloseFont(font);
        }
        TTF_Quit();
        glDeleteVertexArrays(1, &text_vao);
        glDeleteBuffers(1, &text_vbo);
        glDeleteTextures(1, &text_texture);
        glDeleteProgram(text_program);

        SDL_GL_DeleteContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void GLRenderer::init_gl() {
        // 编译着色器
        compile_shaders();

        // 创建平面顶点数据
        float vertices[] = {
                // 位置       // 纹理坐标
                -1.0f,  1.0f, 0.0f, 0.0f, // 左上
                1.0f,  1.0f, 1.0f, 0.0f, // 右上
                -1.0f, -1.0f, 0.0f, 1.0f, // 左下
                1.0f, -1.0f, 1.0f, 1.0f  // 右下
        };

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // 位置属性
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)nullptr);
        glEnableVertexAttribArray(0);
        // 纹理坐标属性
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // 创建 YUV 纹理
        create_textures();
    }

    void GLRenderer::compile_shaders() {
        GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vs_source, nullptr);
        glCompileShader(vertex_shader);

        GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fs_source, nullptr);
        glCompileShader(fragment_shader);

        program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);

        // 验证着色器编译是否成功
        GLint success;
        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
        if(!success) {
            char infoLog[512];
            glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog);
            LOG_ERROR("顶点着色器编译失败: {}", infoLog);
        }

        glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
        if(!success) {
            char infoLog[512];
            glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog);
            LOG_ERROR("片段着色器编译失败: {}", infoLog);
        }

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
    }

    void GLRenderer::create_textures() {
        glGenTextures(1, &y_tex);
        glGenTextures(1, &u_tex);
        glGenTextures(1, &v_tex);

        // 配置 Y 纹理 (亮度)
        glBindTexture(GL_TEXTURE_2D, y_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // 配置 UV 纹理（色度）
        glBindTexture(GL_TEXTURE_2D, u_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D, v_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    void GLRenderer::render_frame(const uint8_t* y_plane, const uint8_t* u_plane, const uint8_t* v_plane,
                                  int y_width, int y_height, int y_stride, int uv_stride) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);

        // 更新Y纹理
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, y_tex);

        // 如果存在行对齐问题，逐行上传数据
        if (y_stride == y_width) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, y_width, y_height, 0,
                         GL_RED, GL_UNSIGNED_BYTE, y_plane);
        } else {
            // 先创建空纹理
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, y_width, y_height, 0,
                         GL_RED, GL_UNSIGNED_BYTE, nullptr);
            // 逐行上传，避开可能的padding
            for (int i = 0; i < y_height; i++) {
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, i, y_width, 1,
                                GL_RED, GL_UNSIGNED_BYTE, y_plane + i * y_stride);
            }
        }

        // 计算UV平面尺寸 (YUV 4:2:0格式中，UV平面宽高是Y平面的一半)
        int uv_width = y_width / 2;
        int uv_height = y_height / 2;

        // 更新U纹理
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, u_tex);
        if (uv_stride == uv_width) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, uv_width, uv_height, 0,
                         GL_RED, GL_UNSIGNED_BYTE, u_plane);
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, uv_width, uv_height, 0,
                         GL_RED, GL_UNSIGNED_BYTE, nullptr);
            for (int i = 0; i < uv_height; i++) {
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, i, uv_width, 1,
                                GL_RED, GL_UNSIGNED_BYTE, u_plane + i * uv_stride);
            }
        }

        // 更新V纹理
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, v_tex);
        if (uv_stride == uv_width) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, uv_width, uv_height, 0,
                         GL_RED, GL_UNSIGNED_BYTE, v_plane);
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, uv_width, uv_height, 0,
                         GL_RED, GL_UNSIGNED_BYTE, nullptr);
            for (int i = 0; i < uv_height; i++) {
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, i, uv_width, 1,
                                GL_RED, GL_UNSIGNED_BYTE, v_plane + i * uv_stride);
            }
        }

        // 设置着色器采样器
        glUniform1i(glGetUniformLocation(program, "y_tex"), 0);
        glUniform1i(glGetUniformLocation(program, "u_tex"), 1);
        glUniform1i(glGetUniformLocation(program, "v_tex"), 2);

        // 绘制全屏四边形
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    // 在GLRenderer.cpp的init_gl()后添加
    void GLRenderer::init_ui_resources() {
        // 创建UI着色器
        create_ui_shaders();

        // 进度条顶点数据
        float vertices[] = {
                0.0f, 0.0f,
                1.0f, 0.0f,
                0.0f, 1.0f,
                1.0f, 1.0f
        };

        glGenVertexArrays(1, &ui_vao);
        glGenBuffers(1, &ui_vbo);

        glBindVertexArray(ui_vao);
        glBindBuffer(GL_ARRAY_BUFFER, ui_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }

    void GLRenderer::create_ui_shaders() {
        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &ui_vertex_shader, nullptr);
        glCompileShader(vs);

        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &ui_fragment_shader, nullptr);
        glCompileShader(fs);

        ui_program = glCreateProgram();
        glAttachShader(ui_program, vs);
        glAttachShader(ui_program, fs);
        glLinkProgram(ui_program);

        // 验证着色器编译
        GLint success;
        glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
        if(!success) {
            char infoLog[512];
            glGetShaderInfoLog(vs, 512, NULL, infoLog);
            LOG_ERROR("UI顶点着色器编译失败: {}", infoLog);
        }

        glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
        if(!success) {
            char infoLog[512];
            glGetShaderInfoLog(fs, 512, NULL, infoLog);
            LOG_ERROR("UI片段着色器编译失败: {}", infoLog);
        }

        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    void GLRenderer::render_ui(float progress, double current_time, double total_time,
                               bool is_paused, bool show_debug) {
        // 保存当前OpenGL状态
        GLint last_program;
        glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
        GLboolean depth_test = glIsEnabled(GL_DEPTH_TEST);

        // 配置UI渲染状态
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // 渲染进度条
        render_progress_bar(progress);

        // 获取窗口尺寸
        int w, h;
        SDL_GetWindowSize(window, &w, &h);

        // 计算进度条位置
        float bar_y = h - progress_style.height - 20.0f;

//        // 在渲染文本前先渲染一个半透明背景矩形
//        render_colored_rect(5.0f, bar_y + progress_style.height + 8.0f,
//                            210.0f, 25.0f, glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));

        // 格式化并渲染时间文本
        std::string time_text = format_time(current_time) + "/" + format_time(total_time);
        render_text(time_text, 10.0f, bar_y + progress_style.height + 5.0f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

        // 添加调试坐标系参考
        if (show_debug) {
            // 左上角红色矩形
            render_colored_rect(0.0f, 0.0f, 30.0f, 30.0f, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

            // 右上角绿色矩形
            render_colored_rect(w - 30.0f, 0.0f, 30.0f, 30.0f, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

            // 左下角蓝色矩形
            render_colored_rect(0.0f, h - 30.0f, 30.0f, 30.0f, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));

            // 右下角黄色矩形
            render_colored_rect(w - 30.0f, h - 30.0f, 30.0f, 30.0f, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));

            // 屏幕中央十字参考线
            render_colored_rect(w/2 - 1, 0, 2, h, glm::vec4(0.5f, 0.5f, 0.5f, 0.5f)); // 垂直线
            render_colored_rect(0, h/2 - 1, w, 2, glm::vec4(0.5f, 0.5f, 0.5f, 0.5f)); // 水平线
        }

        // 恢复状态
        depth_test ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
        glUseProgram(last_program);

        SDL_GL_SwapWindow(window); // 确保UI绘制显示出来
    }

    void GLRenderer::render_progress_bar(float progress) {
        glUseProgram(ui_program);

        // 获取窗口尺寸
        int w, h;
        SDL_GetWindowSize(window, &w, &h);

        // 设置正交投影
        glm::mat4 projection = glm::ortho(0.0f, (float)w, (float)h, 0.0f);
        glUniformMatrix4fv(glGetUniformLocation(ui_program, "projection"),
                           1, GL_FALSE, &projection[0][0]);

        // 计算进度条位置
        float bar_y = h - progress_style.height - 20.0f;

        // 绘制背景
        glUniform4fv(glGetUniformLocation(ui_program, "uColor"), 1,
                     &progress_style.background_color[0]);
        glBindVertexArray(ui_vao);

        // 更新顶点数据为整个进度条背景
        float bg_vertices[] = {
                0.0f, bar_y,
                (float)w, bar_y,
                0.0f, bar_y + progress_style.height,
                (float)w, bar_y + progress_style.height
        };
        glBindBuffer(GL_ARRAY_BUFFER, ui_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(bg_vertices), bg_vertices, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // 绘制进度前景
        glUniform4fv(glGetUniformLocation(ui_program, "uColor"), 1,
                     &progress_style.progress_color[0]);

        // 更新顶点数据为进度条填充部分
        float prog_width = w * progress;
        float prog_vertices[] = {
                0.0f, bar_y,
                prog_width, bar_y,
                0.0f, bar_y + progress_style.height,
                prog_width, bar_y + progress_style.height
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(prog_vertices), prog_vertices, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    bool GLRenderer::handle_events() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    return false;
                case SDL_KEYDOWN: {
                    if (eventCallback) {
                        eventCallback(static_cast<SDL_KeyCode>(event.key.keysym.sym));
                    }

                    // 检查是否是方向键且播放器已暂停
                    if (isPausedCallback() && frameStepCallback) {
                        if (event.key.keysym.sym == SDLK_RIGHT) {
                            frameStepCallback(true); // 下一帧
                        }
                        else if (event.key.keysym.sym == SDLK_LEFT) {
                            frameStepCallback(false); // 上一帧
                        }
                    }
                    break;
                }
                case SDL_MOUSEBUTTONDOWN:{
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        int x, y;
                        SDL_GetMouseState(&x, &y);

                        // 判断是否点击进度条区域
                        int h = 0;
                        SDL_GetWindowSize(window, nullptr, &h);
                        const float bar_y = h - progress_style.height - 20.0f;

                        if (y >= bar_y && y <= bar_y + progress_style.height) {
                            int w = 0;
                            SDL_GetWindowSize(window, &w, nullptr);
                            const float progress = x / (float)w;
                            if (seekCallback) {
                                seekCallback(progress);
                            }
                        }
                    }
                    break;
                }
                case SDL_WINDOWEVENT:
                    break;
            }
        }
        return true;
    }

    void GLRenderer::setEventCallback(const EventCallback& callback) {
        eventCallback = callback; // 绑定回调函数
    }

    void GLRenderer::setSeekCallback(const SeekCallback& callback) {
        seekCallback = callback;
    }

    void GLRenderer::render_colored_rect(float x, float y, float width, float height, const glm::vec4& color) {
        // 使用UI着色器程序
        glUseProgram(ui_program);

        // 获取窗口尺寸
        int win_width, win_height;
        SDL_GetWindowSize(window, &win_width, &win_height);

        // 设置正交投影
        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(win_width),
                                          static_cast<float>(win_height), 0.0f);
        glUniformMatrix4fv(glGetUniformLocation(ui_program, "projection"), 1, GL_FALSE, &projection[0][0]);

        // 设置颜色
        glUniform4fv(glGetUniformLocation(ui_program, "uColor"), 1, &color[0]);

        // 定义矩形顶点
        float vertices[] = {
                x,          y,
                x + width,  y,
                x,          y + height,
                x + width,  y + height
        };

        // 绑定VAO和VBO
        glBindVertexArray(ui_vao);
        glBindBuffer(GL_ARRAY_BUFFER, ui_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

        // 绘制矩形
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // 检查OpenGL错误
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            LOG_ERROR("渲染矩形出错: {}", error);
        }
    }

    // 添加设置帧步进回调的方法
    void GLRenderer::setFrameStepCallback(const FrameStepCallback& callback) {
        frameStepCallback = callback;
    }

    void GLRenderer::setIsPausedCallback(const IsPausedCallback& callback) {
        isPausedCallback = callback;
    }

} // namespace video
