#include "macmediakeys.h"

#import <MediaPlayer/MediaPlayer.h>

static MacMediaKeys *ms_instance = nullptr;

MacMediaKeys::MacMediaKeys(QObject *parent)
    : QObject(parent)
{
    if (ms_instance) {
        qFatal("MacMediaKeys instance is already allocated");
    }
    ms_instance = this;

    // force mac os to use our handler
    setPlayingState("Arfarius", "Arfarius", true);

    MPRemoteCommandCenter *commandCenter = [MPRemoteCommandCenter sharedCommandCenter];

    commandCenter.playCommand.enabled = true;
    [commandCenter.playCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent * _Nonnull event) {
        emit ms_instance->play();
        return MPRemoteCommandHandlerStatusSuccess;
    }];

    commandCenter.pauseCommand.enabled = true;
    [commandCenter.pauseCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent * _Nonnull event) {
        emit ms_instance->pause();
        return MPRemoteCommandHandlerStatusSuccess;
    }];

    commandCenter.togglePlayPauseCommand.enabled = true;
    [commandCenter.togglePlayPauseCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent * _Nonnull event) {
        emit ms_instance->playPause();
        return MPRemoteCommandHandlerStatusSuccess;
    }];

    commandCenter.stopCommand.enabled = true;
    [commandCenter.stopCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent * _Nonnull event) {
        emit ms_instance->stop();
        return MPRemoteCommandHandlerStatusSuccess;
    }];

    commandCenter.nextTrackCommand.enabled = true;
    [commandCenter.nextTrackCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent * _Nonnull event) {
        emit ms_instance->forward();
        return MPRemoteCommandHandlerStatusSuccess;
    }];

    commandCenter.previousTrackCommand.enabled = true;
    [commandCenter.previousTrackCommand addTargetWithHandler:^MPRemoteCommandHandlerStatus(MPRemoteCommandEvent * _Nonnull event) {
        emit ms_instance->backward();
        return MPRemoteCommandHandlerStatusSuccess;
    }];
}

MacMediaKeys::~MacMediaKeys() {
    if (ms_instance == this) ms_instance = nullptr;
}

void MacMediaKeys::setPlayingState(QString artist, QString title, bool playing) {
    MPNowPlayingInfoCenter *nowPlaying = [MPNowPlayingInfoCenter defaultCenter];
    nowPlaying.playbackState = playing ? MPNowPlayingPlaybackStatePlaying : MPNowPlayingPlaybackStatePaused;

    NSMutableDictionary *info = [NSMutableDictionary dictionary];
    info[MPMediaItemPropertyTitle] = artist.toNSString();
    info[MPMediaItemPropertyArtist] = title.toNSString();
    nowPlaying.nowPlayingInfo = info;
}
