#include "macsupport.h"

#include <exception>
#include <QWidget>
#include <QDebug>

#import <Cocoa/Cocoa.h>
#import <objc/runtime.h>

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
        qDebug() << "MyPrivate::MyPrivate() : class_addMethod failed!";

    ms_instance = this;
}

MacSupport::~MacSupport() {
    if (ms_instance == this) ms_instance = 0;
}

void MacSupport::emitDockClick() {
    emit dockClicked();
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
