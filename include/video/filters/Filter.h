//
// Created by Weichuandong on 2025/3/6.
//

#ifndef VIDEOPLAYER_FILTER_H
#define VIDEOPLAYER_FILTER_H

#include <string>
extern "C" {
#include <libavfilter/avfilter.h>
}

namespace video {

    class Filter {
    public:
        virtual ~Filter() = default;

        // 获取滤镜名称
        virtual std::string getName() const = 0;

        // 获取滤镜字符串，用于FFmpeg的滤镜配置
        virtual std::string getFilterString() const = 0;
    };

};

#endif //VIDEOPLAYER_FILTER_H
