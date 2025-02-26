#ifndef FFMPEGDECODER_H
#define FFMPEGDECODER_H

#include <string>
#include <stdexcept>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

namespace video {

    class FFmpegDecoder {
    public:
        FFmpegDecoder(const std::string& filepath);
        ~FFmpegDecoder();

        bool get_next_frame(uint8_t* rgb_buffer); // 获取下一帧 RGB 数据
        int width() const;  // 视频宽度
        int height() const; // 视频高度

    private:
        AVFormatContext* fmt_ctx = nullptr;
        AVCodecContext* codec_ctx = nullptr;
        struct SwsContext* sws_ctx = nullptr;
        int video_stream_idx = -1;
    };

} // namespace video


#endif //FFMPEGDECODER_H
