#include "macsupport.h"

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
    MacSupport *me = reinterpret_cast<MacSupport*>(refcon);

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
            me->emitKeyEvent(keyCode, keyState);
            return NULL;
        break;
    }

    return event;
}

static MacSupport *ms_instance = 0;

NSString * nsStringFromQString(const QString & s)
{
    const char * utf8String = s.toUtf8().constData();
    return [[NSString alloc] initWithUTF8String: utf8String];
}

void dockClickHandler(id self, SEL _cmd)
{
    Q_UNUSED(self);
    Q_UNUSED(_cmd);
    ms_instance->emitDockClick();
}

MacSupport::MacSupport(QObject *parent):
    QObject(parent)
{
    if (ms_instance) {
        qFatal("MacSupport instance is already allocated");
    }

    Class cls = [[[NSApplication sharedApplication] delegate] class];
    if (!class_addMethod(cls, @selector(applicationShouldHandleReopen:hasVisibleWindows:), (IMP) dockClickHandler, "v@:"))
        qFatal("MacSupport(): unable to add dock click handler");

    _eventPort = CGEventTapCreate(
        kCGSessionEventTap,
        kCGHeadInsertEventTap,
        kCGEventTapOptionDefault,
        CGEventMaskBit(NX_SYSDEFINED),
        tapEventCallback,
        (void *)this
    );
    if (_eventPort == NULL) {
        qFatal("Fatal Error: Event Tap could not be created");
    }

    _runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorSystemDefault, _eventPort, 0);
    if (_runLoopSource == NULL) {
        qFatal("Fatal Error: Run Loop Source could not be created");
    }

    CFRunLoopRef runLoop = CFRunLoopGetCurrent();
    if (runLoop == NULL) {
        qFatal("Fatal Error: Couldn't get current threads Run Loop");
    }

    CFRunLoopAddSource(runLoop, _runLoopSource, kCFRunLoopCommonModes);

    ms_instance = this;
}

MacSupport::~MacSupport() {
    CFRelease(_eventPort);
    CFRelease(_runLoopSource);
    if (ms_instance == this) ms_instance = 0;
}

void MacSupport::emitDockClick() {
    emit dockClicked();
}

void MacSupport::emitKeyEvent(int keycode, int keystate)
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

void MacSupport::setDockBadge(const QString & badgeText)
{
    NSString * badgeString = nsStringFromQString(badgeText);
    [[NSApp dockTile] setBadgeLabel: badgeString];
    [badgeString release];
}

void MacSupport::setDockOverlay(QWidget * overlay)
{
    static QWidget * currentOverlay = 0;
    if (currentOverlay)
        currentOverlay->deleteLater();
    currentOverlay = overlay;
    if (overlay)
    {
        NSView * overlayView = (NSView *)overlay->winId();
        [[NSApp dockTile] setContentView: overlayView];
        overlay->update();
    }
    else
    {
        [[NSApp dockTile] setContentView: nil];
    }
    [[NSApp dockTile] display];
}

void MacSupport::requestAttention()
{
    [NSApp requestUserAttention: NSInformationalRequest];
}
