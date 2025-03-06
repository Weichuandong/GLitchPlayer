//
// Created by WeiChuandong on 2025/3/6.
//
#include "video/filters/FilterManager.h"

namespace video {

FilterManager::FilterManager()
    : filterGraph(nullptr),
      bufferSinkCtx(nullptr),
      bufferSrcCtx(nullptr),
      width(0),
      height(0),
      pixFormat(0){

}

FilterManager::~FilterManager() {
    release();
}

bool FilterManager::init(int width_, int height_, int pixFormat_) {
    width = width_;
    height = height_;
    pixFormat = pixFormat_;

    return true;
}

void FilterManager::release() {
    if (filterGraph) {
        avfilter_graph_free(&filterGraph);
        filterGraph = nullptr;
    }

    bufferSrcCtx = nullptr;
    bufferSinkCtx = nullptr;
}

    void FilterManager::registerFilter(std::shared_ptr<Filter> filter) {
        if (filter) {
            filters[filter->getName()] = filter;
        }
    }

bool FilterManager::activateFilter(const std::string &filterName) {
    // 根据名称查找滤镜
    auto it = filters.find(filterName);
    if (it == filters.end()) {
        LOG_ERROR("Filter not found : {}", filterName);
        return false;
    }

    // 避免重复激活
    if (isFilterExists(filterName)) return true;

    activeFilters.push_back(filterName);
    return rebuildFilterChain();
}

bool FilterManager::deactivateFilter(const std::string &filterName) {
    // 查找并移除激活的滤镜
    for (auto it = activeFilters.begin(); it != activeFilters.end(); ++it) {
        if (*it == filterName) {
            activeFilters.erase(it);
            return rebuildFilterChain();
        }
    }
    return true;
}

    void FilterManager::deactivateAllFilter() {
        if (activeFilters.empty()) return;

        activeFilters.clear();

        release();
    }


bool FilterManager::rebuildFilterChain() {
    // 重建渲染链的核心逻辑
    release();

    if (activeFilters.empty()) return true;

    filterGraph = avfilter_graph_alloc();
    if (!filterGraph) {
        LOG_ERROR("Failed to allocate filter graph");
        return false;
    }

    // 构建缓冲源滤镜（接收解码后的原始帧）
    const AVFilter* bufferSrc = avfilter_get_by_name("buffer");
    if (!bufferSrc) {
        LOG_ERROR("Cannot find buffer source filter");
        return false;
    }

    // 构建缓冲槽滤镜（提供处理后的帧）
    const AVFilter* bufferSink = avfilter_get_by_name("buffersink");
    if (!bufferSink) {
        LOG_ERROR("Cannot find buffer sink filter");
        return false;
    }

    // 为buffer源滤镜创建参数
    AVRational timeBase = {1, 1000}; // 这里使用通用时基
    char args[512];
    snprintf(args, sizeof(args),
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
             width, height, pixFormat,
             timeBase.num, timeBase.den, 1, 1);

    // 创建buffer源滤镜上下文
    int ret = avfilter_graph_create_filter(&bufferSrcCtx, bufferSrc, "in",
                                           args, nullptr, filterGraph);
    if (ret < 0) {
        LOG_ERROR("Cannot create buffer source");
        return false;
    }

    // 创建buffer槽滤镜上下文
    ret = avfilter_graph_create_filter(&bufferSinkCtx, bufferSink, "out",
                                       nullptr, nullptr, filterGraph);
    if (ret < 0) {
        LOG_ERROR("Cannot create buffer sink");
        return false;
    }

    // 设置buffer槽滤镜的像素格式
    ret = av_opt_set_bin(bufferSinkCtx, "pix_fmts",
                         (uint8_t*)&pixFormat, sizeof(pixFormat),
                         AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        LOG_ERROR("Cannot set output pixel format");
        return false;
    }

    // 构建滤镜链描述字符串
    std::stringstream filterDesc;

    // 如果有多个滤镜，需要将它们连接起来
    for (size_t i = 0; i < activeFilters.size(); ++i) {
        const auto& filterName = activeFilters[i];
        auto it = filters.find(filterName);

        if (it != filters.end()) {
            if (i > 0) filterDesc << ",";
            filterDesc << it->second->getFilterString();
        }
    }

    std::string filtersDescStr = filterDesc.str();
    LOG_INFO("Building filter chain: {}", filtersDescStr.c_str());

    // 创建滤镜描述的输出和输入端
    AVFilterInOut* outputs = avfilter_inout_alloc();
    AVFilterInOut* inputs = avfilter_inout_alloc();

    if (!outputs || !inputs) {
        avfilter_inout_free(&outputs);
        avfilter_inout_free(&inputs);
        LOG_ERROR("Failed to allocate filter endpoints");
        return false;
    }

    // 配置滤镜图输入
    outputs->name = av_strdup("in");
    outputs->filter_ctx = bufferSrcCtx;
    outputs->pad_idx = 0;
    outputs->next = nullptr;

    // 配置滤镜图输出
    inputs->name = av_strdup("out");
    inputs->filter_ctx = bufferSinkCtx;
    inputs->pad_idx = 0;
    inputs->next = nullptr;

    // 解析滤镜字符串并将滤镜添加到图中
    if (filtersDescStr.empty()) {
        // 如果没有滤镜描述，创建一个简单的直通路径
        ret = avfilter_link(bufferSrcCtx, 0, bufferSinkCtx, 0);
    } else {
        // 解析滤镜链描述并创建滤镜链
        ret = avfilter_graph_parse_ptr(filterGraph, filtersDescStr.c_str(),
                                       &inputs, &outputs, nullptr);
    }

    avfilter_inout_free(&outputs);
    avfilter_inout_free(&inputs);

    if (ret < 0) {
        LOG_ERROR("Failed to parse filter description");
        char errBuff[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errBuff, sizeof(errBuff));
        LOG_ERROR("Error: %s", errBuff);
        return false;
    }

    // 配置滤镜图
    ret = avfilter_graph_config(filterGraph, nullptr);
    if (ret < 0) {
        LOG_ERROR("Failed to configure filter graph");
        return false;
    }

    LOG_INFO("Filter chain rebuilt successfully");
    return true;
}

    AVFrame* FilterManager::applyFilters(AVFrame* frame) {
        // 如果没有滤镜图或没有输入帧，则直接返回原帧
        if (!filterGraph || !frame) {
            return frame;
        }

        // 将帧发送到源缓冲区
        int ret = av_buffersrc_add_frame_flags(bufferSrcCtx, frame,
                                               AV_BUFFERSRC_FLAG_KEEP_REF);
        if (ret < 0) {
            char errBuff[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, errBuff, sizeof(errBuff));
            LOG_ERROR("Error feeding the filter: %s", errBuff);
            return frame; // 返回原始帧
        }

        // 分配新的输出帧
        AVFrame* filteredFrame = av_frame_alloc();
        if (!filteredFrame) {
            LOG_ERROR("Failed to allocate filtered frame");
            return frame;
        }

        // 从滤镜链获取处理后的帧
        ret = av_buffersink_get_frame(bufferSinkCtx, filteredFrame);
        if (ret < 0) {
            // 如果没有可用的帧，释放分配的帧并返回原始帧
            av_frame_free(&filteredFrame);
            if (ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
                char errBuff[AV_ERROR_MAX_STRING_SIZE];
                av_strerror(ret, errBuff, sizeof(errBuff));
                LOG_ERROR("Error getting filtered frame: %s", errBuff);
            }
            return frame;
        }

        // 设置正确的PTS和时间基准
        filteredFrame->pts = frame->pts;
        filteredFrame->pkt_dts = frame->pkt_dts;

        // 返回处理后的帧，调用者负责最终释放原始帧
        return filteredFrame;
    }

    bool FilterManager::isFilterExists(const std::string& filterName) {
        if (activeFilters.empty()) return false;

        for (auto it : activeFilters) {
            if (it == filterName) return true;
        }
        return false;
    }




}
