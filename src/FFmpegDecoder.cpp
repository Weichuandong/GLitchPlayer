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

        // 保存时间基
        stream_time_base = fmt_ctx->streams[video_stream_idx]->time_base;

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

        // 滤镜管理
        filterManager.init(codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt);
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
                    // 有效帧处理：计算并存储 PTS
                    int64_t pts = frame->pts;
                    if (pts == AV_NOPTS_VALUE) {
                        pts = pkt.dts;  // 回退到解码时间戳
                    }
                    // 转换为秒并存储
                    if (pts != AV_NOPTS_VALUE) {
                        last_valid_pts = pts * av_q2d(stream_time_base);
                        LOG_DEBUG("last_valid_pts = {}", last_valid_pts);
                    } else {
                        // 无有效时间戳时使用解码器内部计数
                        last_valid_pts = codec_ctx->frame_number * av_q2d(codec_ctx->time_base);
                        LOG_DEBUG("last_valid_pts = {}", last_valid_pts);
                    }
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

    double FFmpegDecoder::get_current_pts() const {
        return last_valid_pts;
    }

    bool FFmpegDecoder::seek(double seconds) {
        if (!fmt_ctx || video_stream_idx < 0) return false;

        // 计算目标时间戳（基于流的时间基）
        int64_t target_pts = static_cast<int64_t>(seconds / av_q2d(fmt_ctx->streams[video_stream_idx]->time_base));

        // 清空解码器缓冲区
        avcodec_flush_buffers(codec_ctx);

        // 执行 Seek（AVSEEK_FLAG_BACKWARD 确保跳到关键帧）
        int ret = av_seek_frame(fmt_ctx, video_stream_idx, target_pts, AVSEEK_FLAG_BACKWARD);

        if (ret < 0) {
            LOG_ERROR("Seek failed: {}", av_err2str(ret));
            return false;
        }

        last_valid_pts = target_pts;
        return true;
    }

    double FFmpegDecoder::duration() const {
        if (!fmt_ctx || video_stream_idx < 0) return 0.0;
        return fmt_ctx->duration * av_q2d(AV_TIME_BASE_Q);
    }

    bool FFmpegDecoder::get_next_frame(YUVData& yuv_data) {
        AVFrame* frame = av_frame_alloc();
        AVPacket pkt;

        while (av_read_frame(fmt_ctx, &pkt) >= 0) {
            if (pkt.stream_index == video_stream_idx) {
                avcodec_send_packet(codec_ctx, &pkt);
                if (avcodec_receive_frame(codec_ctx, frame) == 0) {
                    // 有效帧处理：计算并存储 PTS
                    int64_t pts = frame->pts;
                    if (pts == AV_NOPTS_VALUE) {
                        pts = pkt.dts;  // 回退到解码时间戳
                    }
                    // 转换为秒并存储
                    if (pts != AV_NOPTS_VALUE) {
                        last_valid_pts = pts * av_q2d(stream_time_base);
                    } else {
                        // 无有效时间戳时使用解码器内部计数
                        last_valid_pts = codec_ctx->frame_number * av_q2d(codec_ctx->time_base);
                    }

                    AVFrame* filterFrame = filterManager.applyFilters(frame);
                    yuv_data.frame = av_frame_clone(filterFrame);

                    if (filterFrame != frame) {
                        av_frame_free(&filterFrame);
                    }
                    av_frame_free(&frame);
                    av_packet_unref(&pkt);
                    return true;
                }
            }
            av_packet_unref(&pkt);
        }
        av_frame_free(&frame);
        return false;
    }
} // namespace video