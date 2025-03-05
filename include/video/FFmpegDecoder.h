#ifndef FFMPEGDECODER_H
#define FFMPEGDECODER_H

#include <string>
#include <stdexcept>
#include "logger.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

namespace video {

    class FFmpegDecoder {
    public:

        struct YUVData {
            AVFrame* frame;

            ~YUVData() {
                if (frame) av_frame_free(&frame);
            }
        };

        explicit FFmpegDecoder(const std::string& filepath);
        ~FFmpegDecoder();

        bool get_next_frame(YUVData& yuv_data);   // 获取下一帧 YUV 数据
        bool get_next_frame(uint8_t* rgb_buffer); // 获取下一帧 RGB 数据
        int width() const;  // 视频宽度
        int height() const; // 视频高度
        double get_current_pts() const;  //获取当前时间戳
        bool seek(double seconds);       //跳转到指定时间
        double duration() const;        //获取视频总时长

    private:
        AVFormatContext* fmt_ctx = nullptr;
        AVCodecContext* codec_ctx = nullptr;
        struct SwsContext* sws_ctx = nullptr;
        int video_stream_idx = -1;

        double last_valid_pts = 0.0;     // 当前帧 PTS（秒为单位）
        AVRational stream_time_base;     // 视频流时间基
    };

} // namespace video


#endif //FFMPEGDECODER_H
