#include "macmediakeys.h"

#include <exception>
#include <QWidget>
#include <QDebug>

#import <Cocoa/Cocoa.h>
#import <objc/runtime.h>
#import <IOKit/hidsystem/ev_keymap.h>

#define NX_KEYSTATE_UP      0x0A
#define NX_KEYSTATE_DOWN    0x0B

static CFMachPortRef _eventPort;
static CFRunLoopSourceRef _runLoopSource;

CGEventRef tapEventCallback(CGEventTapProxy /*proxy*/, CGEventType type, CGEventRef event, void *refcon) {
    MacMediaKeys *me = reinterpret_cast<MacMediaKeys*>(refcon);

    if(type == kCGEventTapDisabledByTimeout)
        CGEventTapEnable(_eventPort, TRUE);

    if(type != NX_SYSDEFINED)
        return event;

    NSEvent *nsEvent = [NSEvent eventWithCGEvent:event];

    if([nsEvent subtype] != 8)
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
        case NX_KEYTYPE_FAST:
        case NX_KEYTYPE_REWIND:
            me->onKeyEvent(keyCode, keyState);
            return NULL;
        break;
    }

    return event;
}

static MacMediaKeys *ms_instance = 0;

MacMediaKeys::MacMediaKeys(QObject *parent):
    QObject(parent)
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

void MacMediaKeys::onKeyEvent(int keycode, int keystate)
{
    switch (keycode) {
        case NX_KEYTYPE_PLAY:
            if(keystate == NX_KEYSTATE_DOWN)
                emit play();
        break;
        case NX_KEYTYPE_FAST:
            if(keystate == NX_KEYSTATE_DOWN)
                emit next();
        break;
        case NX_KEYTYPE_REWIND:
            if(keystate == NX_KEYSTATE_DOWN)
                emit prev();
        break;
    }
}
