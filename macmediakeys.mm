#include "macmediakeys.h"

#include <exception>
#include <QWidget>
#include <QDebug>
#include <QTimerEvent>

#import <Cocoa/Cocoa.h>
#import <objc/runtime.h>
#import <IOKit/hidsystem/ev_keymap.h>

#define NX_KEYSTATE_DOWN    0x0A
#define NX_KEYSTATE_UP      0x0B

static CFMachPortRef _eventPort;
static CFRunLoopSourceRef _runLoopSource;

CGEventRef tapEventCallback(CGEventTapProxy /*proxy*/, CGEventType type, CGEventRef event, void *refcon) {
    MacMediaKeys *me = reinterpret_cast<MacMediaKeys*>(refcon);

    if(type == kCGEventTapDisabledByTimeout)
        CGEventTapEnable(_eventPort, TRUE);

    if(type != NX_SYSDEFINED)
        return event;

    NSEvent *nsEvent = [NSEvent eventWithCGEvent:event];

    if ([nsEvent subtype] != 8)
        return event;

    int data = [nsEvent data1];
    int keyCode = (data & 0xFFFF0000) >> 16;
    int keyFlags = (data & 0xFFFF);
    int keyState = (keyFlags & 0xFF00) >> 8;
    BOOL keyIsRepeat = (keyFlags & 0x1) > 0;

    if (keyIsRepeat)
        return event;

    switch (keyCode) {
    case NX_KEYTYPE_PLAY:
        me->onPlayPauseKey(keyState);
        break;
    case NX_KEYTYPE_FAST:
        me->onForwardKey(keyState);
        break;
    case NX_KEYTYPE_REWIND:
        me->onBackwardKey(keyState);
        break;
    default:
        return event;
    }

    return NULL;
}

static MacMediaKeys *ms_instance = 0;

MacMediaKeys::MacMediaKeys(QObject *parent):
    QObject(parent),
    long_press_timeout(500), long_press_cycle(500),
    backward_state(NotPressed), backward_timer_id(0),
    forward_state(NotPressed), forward_timer_id(0)
{
    if (ms_instance) {
        qFatal("MacMediaKeys instance is already allocated");
    }

    _eventPort = CGEventTapCreate(
                kCGSessionEventTap,
                kCGHeadInsertEventTap,
                kCGEventTapOptionDefault,
                CGEventMaskBit(NX_SYSDEFINED),
                tapEventCallback,
                (void *)this
                );
    if (_eventPort == NULL) {
        qFatal("MacMediaKeys: Event Tap could not be created");
    }

    _runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorSystemDefault, _eventPort, 0);
    if (_runLoopSource == NULL) {
        qFatal("MacMediaKeys: Run Loop Source could not be created");
    }

    CFRunLoopRef runLoop = CFRunLoopGetCurrent();
    if (runLoop == NULL) {
        qFatal("MacMediaKeys: Couldn't get current threads Run Loop");
    }

    CFRunLoopAddSource(runLoop, _runLoopSource, kCFRunLoopCommonModes);

    ms_instance = this;
}

MacMediaKeys::~MacMediaKeys() {
    CFRelease(_eventPort);
    CFRelease(_runLoopSource);
    if (ms_instance == this) ms_instance = 0;
}

void MacMediaKeys::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == backward_timer_id) {
        if (backward_state == NotPressed) {
            qCritical("timerEvent(): programming error, backward button state is NotPressed");
        } else if (backward_state == Pressed) {
            backward_state = PressedEmmiting;
            killTimer(backward_timer_id);
            backward_timer_id = startTimer(long_press_cycle);
            Q_ASSERT(backward_timer_id > 0);
            emit seekBackward();
        } else if (backward_state == PressedEmmiting) {
            emit seekBackward();
        }
    } else if (event->timerId() == forward_timer_id) {
        if (forward_state == NotPressed) {
            qCritical("timerEvent(): programming error, forward button state is NotPressed");
        } else if (forward_state == Pressed) {
            forward_state = PressedEmmiting;
            killTimer(forward_timer_id);
            forward_timer_id = startTimer(long_press_cycle);
            Q_ASSERT(forward_timer_id > 0);
            emit seekForward();
        } else if (forward_state == PressedEmmiting) {
            emit seekForward();
        }
    } else {
        qCritical("timerEvent(): programming error, unknown timer id fired %i", event->timerId());
    }
}

void MacMediaKeys::onBackwardKey(int keystate)
{
    if (backward_state == NotPressed) {
        if (keystate == NX_KEYSTATE_DOWN) {
            backward_timer_id = startTimer(long_press_timeout);
            Q_ASSERT(backward_timer_id > 0);
            backward_state = Pressed;
        } else if (keystate == NX_KEYSTATE_UP) {
            qCritical("onBackwardKey(): programming error, state is NotPressed, but UP state recieved");
        }
    } else if (backward_state == Pressed) {
        if (keystate == NX_KEYSTATE_DOWN) {
            qCritical("onBackwardKey(): programming error, state is Pressed, but DOWN state recieved");
        } else if (keystate == NX_KEYSTATE_UP) {
            killTimer(backward_timer_id);
            backward_timer_id = 0;
            backward_state = NotPressed;
            emit backward();
        }
    } else if (backward_state == PressedEmmiting) {
        if (keystate == NX_KEYSTATE_DOWN) {
            qCritical("onBackwardKey(): programming error, state is PressedEmmiting, but DOWN state recieved");
        } else if (keystate == NX_KEYSTATE_UP) {
            killTimer(backward_timer_id);
            backward_timer_id = 0;
            backward_state = NotPressed;
        }
    }
}

void MacMediaKeys::onPlayPauseKey(int keystate)
{
    if (keystate == NX_KEYSTATE_UP) {
        emit playPause();
    }
}

void MacMediaKeys::onForwardKey(int keystate)
{
    if (forward_state == NotPressed) {
        if (keystate == NX_KEYSTATE_DOWN) {
            forward_timer_id = startTimer(long_press_timeout);
            Q_ASSERT(forward_timer_id > 0);
            forward_state = Pressed;
        } else if (keystate == NX_KEYSTATE_UP) {
            qCritical("onForwardKey(): programming error, state is NotPressed, but UP state recieved");
        }
    } else if (forward_state == Pressed) {
        if (keystate == NX_KEYSTATE_DOWN) {
            qCritical("onForwardKey(): programming error, state is Pressed, but DOWN state recieved");
        } else if (keystate == NX_KEYSTATE_UP) {
            killTimer(forward_timer_id);
            forward_timer_id = 0;
            forward_state = NotPressed;
            emit forward();
        }
    } else if (forward_state == PressedEmmiting) {
        if (keystate == NX_KEYSTATE_DOWN) {
            qCritical("onForwardKey(): programming error, state is PressedEmmiting, but DOWN state recieved");
        } else if (keystate == NX_KEYSTATE_UP) {
            killTimer(forward_timer_id);
            forward_timer_id = 0;
            forward_state = NotPressed;
        }
    }
}
