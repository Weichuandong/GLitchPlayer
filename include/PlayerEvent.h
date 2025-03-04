//
// Created by WeiChuandong on 2025/2/26.
//

#ifndef PLAYEREVENT_H
#define PLAYEREVENT_H

enum class PlayerEvent {
    None,                       // 无事件
    PlayPause,                  // 暂停/播放
    SeekBackWard_5,             // 回退5S
    SeekForward_5,              // 前进5S
    Restart,                    // 从头播放
    Quit                        // 退出
};

#endif //PLAYEREVENT_H
