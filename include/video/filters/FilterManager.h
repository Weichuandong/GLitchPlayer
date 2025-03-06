//
// Created by WeiChuandong on 2025/3/6.
//

#ifndef VIDEOPLAYER_FILTERMANAGER_H
#define VIDEOPLAYER_FILTERMANAGER_H

extern "C" {
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
};

#include <map>
#include <string>
#include <memory>
#include <vector>
#include <sstream>
#include "logger.h"

#include "video/filters/Filter.h"

namespace video {

    class Filter;

    class FilterManager {
    public:
        FilterManager();
        ~FilterManager();

        // 初始化滤镜管理器
        bool init(int width, int height, int pixFormat);

        // 释放资源
        void release();

        // 注册滤镜
        void registerFilter(std::shared_ptr<Filter> filter);

        // 激活/停用滤镜
        bool activateFilter(const std::string& filterName);
        bool deactivateFilter(const std::string& filterName);

        void deactivateAllFilter();

        // 应用滤镜链处理帧
        AVFrame* applyFilters(AVFrame* frame);

        // 判断该滤镜是否在使用
        bool isFilterExists(const std::string& filterName);

        // 获取滤镜信息
        std::vector<std::string> getAvailableFilters() const;
        std::vector<std::string> getActiveFilters() const;

    private:
        // 重建滤镜链
        bool rebuildFilterChain();

        AVFilterGraph* filterGraph;
        AVFilterContext* bufferSrcCtx;
        AVFilterContext* bufferSinkCtx;
        std::map<std::string, std::shared_ptr<Filter>> filters;
        std::vector<std::string> activeFilters;
        int width, height, pixFormat;
    };
};

#endif //VIDEOPLAYER_FILTERMANAGER_H
