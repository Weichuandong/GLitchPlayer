//
// Created by WeiChuandong on 2025/3/6.
//

#ifndef VIDEOPLAYER_MIRRORFILTER_H
#define VIDEOPLAYER_MIRRORFILTER_H

#include "video/filters/Filter.h"
#include "logger.h"

namespace video {

    class MirrorFilter : public Filter {
    public:
        enum MirrorType {
            HORIZONTAL,         // 左右分屏镜像
            VERTICAL,           // 上下分屏
            QUAD                // 四分屏
        };

        explicit MirrorFilter(MirrorType type = MirrorType::HORIZONTAL);

        ~MirrorFilter() override = default;

        std::string getName() const override;
        std::string getFilterString() const override;

    private:
        MirrorType type;
    };

}
#endif //VIDEOPLAYER_MIRRORFILTER_H
