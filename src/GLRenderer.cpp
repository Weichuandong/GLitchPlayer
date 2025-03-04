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
    }

    GLRenderer::~GLRenderer() {
        glDeleteTextures(1, &y_tex);
        glDeleteTextures(1, &u_tex);
        glDeleteTextures(1, &v_tex);
        glDeleteProgram(program);
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

        // 配置 UV 纹理（色度）
        glBindTexture(GL_TEXTURE_2D, u_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, v_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    void GLRenderer::render_frame(const uint8_t* y_plane, const uint8_t* u_plane, const uint8_t* v_plane,
                                  int y_width, int y_height, int uv_width, int uv_height) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);

        // 更新 Y 纹理
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, y_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, y_width, y_height, 0,
                     GL_RED, GL_UNSIGNED_BYTE, y_plane);

        // 更新 U 纹理
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, u_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, uv_width, uv_height, 0,
                     GL_RED, GL_UNSIGNED_BYTE, u_plane);

        // 更新 V 纹理
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, v_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, uv_width, uv_height, 0,
                     GL_RED, GL_UNSIGNED_BYTE, v_plane);

        // 设置纹理单元
        glUniform1i(glGetUniformLocation(program, "y_tex"), 0);
        glUniform1i(glGetUniformLocation(program, "u_tex"), 1);
        glUniform1i(glGetUniformLocation(program, "v_tex"), 2);

        // 绘制矩形
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        SDL_GL_SwapWindow(window);
    }

    bool GLRenderer::handle_events() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    return false;
                case SDL_KEYDOWN:
                    if (eventCallback) {
                        eventCallback(static_cast<SDL_KeyCode>(event.key.keysym.sym));
                    }
                    break;
                case SDL_WINDOWEVENT:
                    break;
            }
        }
        return true;
    }

    void GLRenderer::setEventCallback(const EventCallback& callback) {
        eventCallback = callback; // 绑定回调函数
    }

    

} // namespace video
