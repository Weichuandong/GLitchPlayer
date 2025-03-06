//
// Created by WeiChuandong on 2025/3/6.
//

#ifndef VIDEOPLAYER_FLIPFILTER_H
#define VIDEOPLAYER_FLIPFILTER_H

#include "Filter.h"
#include "logger.h"

namespace video {

    class FlipFilter : public Filter {
    public:
        enum FlipType {
            VERTICAL,
            HORIZONTAL
        };

        explicit FlipFilter(FlipType type = VERTICAL);
        ~FlipFilter() override = default;

        std::string getName() const override;
        std::string getFilterString() const override;

        FlipType getFlipType() const;
    private:
        FlipType type;
    };
}
#endif //VIDEOPLAYER_FLIPFILTER_H
