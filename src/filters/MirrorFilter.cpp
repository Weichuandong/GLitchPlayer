//
// Created by WeiChuandong on 2025/3/6.
//
#include "video/filters/MirrorFilter.h"

namespace video {

    MirrorFilter::MirrorFilter(video::MirrorFilter::MirrorType type) : type(type){

    }

    std::string MirrorFilter::getFilterString() const {
        switch (type) {
            case MirrorType::HORIZONTAL:
                return "split[main][tmp]; [tmp]crop=iw/2:ih:0:0,hflip[flip];[main][flip]overlay=W/2:0";
            case MirrorType::VERTICAL:
                return "split[main][tmp];[tmp]crop=iw:ih/2:0:0,vflip[flip];[main][flip]overlay=0:H/2";

            case MirrorType::QUAD:
                return "split=4[a][b][c][d];[a]hflip[ah];[b]vflip[bv];[c]hflip,vflip[chv];"
                       "[ah][bv]vstack=inputs=2[top];[d][chv]vstack=inputs=2[bottom];[top][bottom]hstack=inputs=2";

            default:
                LOG_WARN("Unknown mirror type, defaulting to horizontal split");
                return "split[main][tmp];[tmp]crop=iw/2:ih:0:0,hflip[flip];[main][flip]overlay=W/2:0";
        }
    }

    std::string MirrorFilter::getName() const {
        switch (type) {
            case MirrorType::HORIZONTAL:
                return "hmirror";
            case MirrorType::VERTICAL:
                return "vmirror";
            case MirrorType::QUAD:
                return "quadmirror";
            default:
                return "hmirror";
        }
    }

}