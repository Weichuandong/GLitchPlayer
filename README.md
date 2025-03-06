### 简单视频播放器

#### 目标

- 通过FFmpeg解析视频文件（支持mp4格式）
- 使用FFmpeg进行解码
- 使用SDL创建播放窗口，管理用户输入并为OpenGL提供渲染画布
- 使用SDL进行渲染上屏（进阶：使用OpenGL进行渲染上屏）
- 比较不同渲染方式的性能差异

#### 技术栈

- FFmpeg
- SDL
- OpenGL

#### 要求

- 使用CMake创建工程
- 除了SDL，FFmpeg，OpenGL外，不引入其他第三方库
- 提供`暂停`，`重新播放`，`停止`功能


#### 播放器功能列表
```angular2html
Level 1：基础功能
1. 播放/暂停/停止 ✅
实现难度：⭐
技术方案：
    监听空格键或按钮事件，切换播放状态（is_playing 布尔值控制解码循环）
    停止时调用 avformat_close_input 释放资源

2. 逐帧播放（Frame Step）✅
实现难度：⭐
技术方案：
    暂停状态下按 → 键调用 av_read_frame 解码下一帧并立即渲染
    注意处理 AVPacket 的连续性（可能需要清空解码器缓冲区）


Level 2：交互增强 ✅
3. 进度条拖拽
实现难度：⭐⭐
技术方案：
    在 SDL 中监听鼠标点击事件，根据 X 坐标计算目标时间：

4. 窗口缩放自适应 ✅
实现难度：⭐⭐
技术方案：
OpenGL：根据窗口大小动态调整视口（glViewport）和投影矩阵
或 SDL 渲染器：SDL_RenderSetLogicalSize 保持宽高比


Level 3：性能优化
5. 硬件加速解码（NVIDIA/Intel/AMD）
实现难度：⭐⭐⭐
技术方案：
    FFmpeg 指定硬件解码器（例：NVIDIA CUDA）:

6. 零拷贝渲染（PBO/DMA-BUF）
实现难度：⭐⭐⭐⭐
技术方案：
    使用 OpenGL Pixel Buffer Object (PBO) 异步上传纹理：

Level 4：高级功能
7. 视频滤镜（旋转/镜像/灰度化）
实现难度：⭐⭐⭐
技术方案：
    CPU 滤镜：FFmpeg 滤镜链（avfilter_graph_create_filter）
    GPU 滤镜：编写 OpenGL 着色器（如灰度化）：

8. 字幕叠加（SRT/ASS）
实现难度：⭐⭐⭐⭐
技术方案：
    解析字幕文件时序（例：00:01:23,456 --> 00:01:25,789）
    使用 FreeType + OpenGL 渲染文字到覆盖层或集成 libass 库处理复杂字幕样式


Level 5：专家级
9. 多实例渲染（画中画/多视图）
实现难度：⭐⭐⭐⭐⭐
技术方案：
    多个独立解码器实例（每个视频对应一个 AVFormatContext）
    OpenGL 多视口（glViewport 分区域渲染）

10. 动态分辨率切换（ABR）
实现难度：⭐⭐⭐⭐⭐
技术方案：
    实时监测渲染性能（帧率/FPS）
    动态切换视频流（如从 4K 切到 1080p 流）
```