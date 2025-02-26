#include "video/FFmpegDecoder.h"


namespace video {

    FFmpegDecoder::FFmpegDecoder(const std::string& filepath) {
      /* 初始化 FFmpeg 并打开文件 */
        // 打开文件并查找视频流
        if (avformat_open_input(&fmt_ctx, filepath.c_str(), nullptr, nullptr) != 0) {
            throw std::runtime_error("无法打开文件");
        }
        avformat_find_stream_info(fmt_ctx, nullptr);

        // 查找视频流
        video_stream_idx = -1;
        for (int i = 0; i < fmt_ctx->nb_streams; i++) {
            if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                video_stream_idx = i;
                break;
            }
        }
        if (video_stream_idx == -1) {
            throw std::runtime_error("未找到视频流");
        }

        // 初始化解码器
        AVCodecParameters* codec_params = fmt_ctx->streams[video_stream_idx]->codecpar;
        const AVCodec* codec = avcodec_find_decoder(codec_params->codec_id);
        if (!codec) {
            throw std::runtime_error("找不到解码器");
        }

        codec_ctx = avcodec_alloc_context3(codec);
        avcodec_parameters_to_context(codec_ctx, codec_params);
        if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
            throw std::runtime_error("无法打开解码器");
        }

        // 初始化图像转换器 (YUV → RGB)
        sws_ctx = sws_getContext(
            codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
            codec_ctx->width, codec_ctx->height, AV_PIX_FMT_RGB24,
            SWS_BILINEAR, nullptr, nullptr, nullptr
        );
    }

    FFmpegDecoder::~FFmpegDecoder() {
      /* 释放 FFmpeg 资源 */
        sws_freeContext(sws_ctx);
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&fmt_ctx);
    }

    bool FFmpegDecoder::get_next_frame(uint8_t* rgb_buffer) {
        /* 解码下一帧并转换为 RGB */
        AVPacket pkt;
        AVFrame* frame = av_frame_alloc();

        while (av_read_frame(fmt_ctx, &pkt) >= 0) {
            if (pkt.stream_index == video_stream_idx) {
                avcodec_send_packet(codec_ctx, &pkt);
                if (avcodec_receive_frame(codec_ctx, frame) == 0) {
                    // 转换为RGB
                    uint8_t* dst[] = {rgb_buffer};
                    int dst_linesize[] = {codec_ctx->width * 3};
                    sws_scale(sws_ctx, frame->data, frame->linesize,
                              0, codec_ctx->height, dst, dst_linesize);
                    av_frame_free(&frame);
                    av_packet_unref(&pkt);
                    return true;
                }
            }
            av_packet_unref(&pkt);
        }
        av_frame_free(&frame);
        return false; // 文件结束
    }

    int FFmpegDecoder::width() const {
        return codec_ctx ? codec_ctx->width : 0; // 返回视频宽度
    }

    int FFmpegDecoder::height() const {
        return codec_ctx ? codec_ctx->height : 0; // 返回视频高度
    }

} // namespace video