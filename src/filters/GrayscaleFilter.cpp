//
// Created by WeiChuandong on 2025/3/6.
//

#include "video/filters/GrayscaleFilter.h"

namespace video {

    GrayscaleFilter::GrayscaleFilter(float intensity) : intensity(intensity) {
        intensity = std::max(0.0f, std::min(1.0f, intensity));
    }

    std::string GrayscaleFilter::getFilterString() const {
        if (intensity >= 0.999f) {
            return "format=gray";
        } else {
            float color = 1.0f - intensity;

            // 使用stringstream以确保浮点数格式正确
            std::stringstream ss;
            ss << std::fixed << std::setprecision(3);

            // colorchannelmixer滤镜允许混合RGB通道
            // 灰度公式: 0.299*R + 0.587*G + 0.114*B
            // 我们将根据intensity调整混合比例
            ss << "colorchannelmixer=";

            // 红色通道混合
            ss << "rr=" << (0.299f * intensity + color) << ":";
            ss << "rg=" << (0.587f * intensity) << ":";
            ss << "rb=" << (0.114f * intensity) << ":";

            // 绿色通道混合
            ss << "gr=" << (0.299f * intensity) << ":";
            ss << "gg=" << (0.587f * intensity + color) << ":";
            ss << "gb=" << (0.114f * intensity) << ":";

            // 蓝色通道混合
            ss << "br=" << (0.299f * intensity) << ":";
            ss << "bg=" << (0.587f * intensity) << ":";
            ss << "bb=" << (0.114f * intensity + color);

            return ss.str();
        }
    }

    void GrayscaleFilter::setIntensity(float intensity_) {
        intensity = intensity_;
    }

    std::string GrayscaleFilter::getName() const {
        LOG_DEBUG("name = {}", "gray" + std::to_string(intensity));

        return "gray" + std::to_string(intensity);
    }

}
