#include "macmediakeys.h"

#import <MediaPlayer/MediaPlayer.h>

#include <atomic>
#include <QDebug>

static std::atomic<MacMediaKeys*> ms_instance{nullptr};

MacMediaKeys::MacMediaKeys(QObject *parent)
    : QObject(parent)
{
    MacMediaKeys *expected = nullptr;
    if (!ms_instance.compare_exchange_strong(expected, this)) {
        qWarning() << "MacMediaKeys: instance already exists";
        return;
    }

    // force mac os to use our handler
    setPlayingState("Arfarius", "Arfarius", true);

    MPRemoteCommandCenter *commandCenter = [MPRemoteCommandCenter sharedCommandCenter];

    commandCenter.playCommand.enabled = true;
    [commandCenter.playCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent * _Nonnull event) {
        MacMediaKeys *inst = ms_instance.load(std::memory_order_acquire);
        if (inst) {
            emit inst->play();
        }
        return MPRemoteCommandHandlerStatusSuccess;
    }];

    commandCenter.pauseCommand.enabled = true;
    [commandCenter.pauseCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent * _Nonnull event) {
        MacMediaKeys *inst = ms_instance.load(std::memory_order_acquire);
        if (inst) {
            emit inst->pause();
        }
        return MPRemoteCommandHandlerStatusSuccess;
    }];

    commandCenter.togglePlayPauseCommand.enabled = true;
    [commandCenter.togglePlayPauseCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent * _Nonnull event) {
        MacMediaKeys *inst = ms_instance.load(std::memory_order_acquire);
        if (inst) {
            emit inst->playPause();
        }
        return MPRemoteCommandHandlerStatusSuccess;
    }];

    commandCenter.stopCommand.enabled = true;
    [commandCenter.stopCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent * _Nonnull event) {
        MacMediaKeys *inst = ms_instance.load(std::memory_order_acquire);
        if (inst) {
            emit inst->stop();
        }
        return MPRemoteCommandHandlerStatusSuccess;
    }];

    commandCenter.nextTrackCommand.enabled = true;
    [commandCenter.nextTrackCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent * _Nonnull event) {
        MacMediaKeys *inst = ms_instance.load(std::memory_order_acquire);
        if (inst) {
            emit inst->forward();
        }
        return MPRemoteCommandHandlerStatusSuccess;
    }];

    commandCenter.previousTrackCommand.enabled = true;
    [commandCenter.previousTrackCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent * _Nonnull event) {
        MacMediaKeys *inst = ms_instance.load(std::memory_order_acquire);
        if (inst) {
            emit inst->backward();
        }
        return MPRemoteCommandHandlerStatusSuccess;
    }];
}

MacMediaKeys::~MacMediaKeys()
{
    ms_instance.store(nullptr, std::memory_order_release);
}

void MacMediaKeys::setPlayingState(const QString &artist, const QString &title, bool playing) {
    MPNowPlayingInfoCenter *nowPlaying = [MPNowPlayingInfoCenter defaultCenter];
    nowPlaying.playbackState = playing ? MPNowPlayingPlaybackStatePlaying : MPNowPlayingPlaybackStatePaused;

    NSMutableDictionary *info = [NSMutableDictionary dictionary];
    info[MPMediaItemPropertyArtist] = artist.toNSString();
    info[MPMediaItemPropertyTitle] = title.toNSString();
    nowPlaying.nowPlayingInfo = info;
}
