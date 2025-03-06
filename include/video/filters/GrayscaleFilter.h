//
// Created by WeiChuandong on 2025/3/6.
//

#ifndef VIDEOPLAYER_GRAYSCALEFILTER_H
#define VIDEOPLAYER_GRAYSCALEFILTER_H

#include "Filter.h"
#include <iomanip>
#include <sstream>
#include "logger.h"

namespace video {

    class GrayscaleFilter : public Filter {
    public:
        explicit GrayscaleFilter(float intensity = 1.0f);

        virtual ~GrayscaleFilter() = default;

        std::string getFilterString() const override;

        std::string getName() const override;

        void setIntensity(float intensity);

    private:
        float intensity;
    };
}
#endif //VIDEOPLAYER_GRAYSCALEFILTER_H
