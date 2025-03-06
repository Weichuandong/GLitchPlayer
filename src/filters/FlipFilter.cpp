//
// Created by WeiChuandong on 2025/3/6.
//
#include "video/filters/FlipFilter.h"


namespace video {

    FlipFilter::FlipFilter(video::FlipFilter::FlipType type) : type(type){
        LOG_INFO("Created FlipFilter with type: {}",
                 (type == FlipType::VERTICAL ? "VERTICAL" : "HORIZONTAL"));
    }


    std::string FlipFilter::getName() const { return type == FlipType::VERTICAL ? "vflip" : "hflip"; }

    std::string FlipFilter::getFilterString() const { return type == FlipType::VERTICAL ? "vflip" : "hflip"; }

    FlipFilter::FlipType FlipFilter::getFlipType() const { return type; }

}