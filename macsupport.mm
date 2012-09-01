#include "macsupport.h"
#include <QDebug>
#include <QColor>
#include <QWidget>
#include <QApplication>

#import <Cocoa/Cocoa.h>
#import <objc/runtime.h>

static MacSupport *ms_instance = 0;
static NSColor * gFrameColor = nil;
static NSColor * gTitleColor = nil;

NSString * nsStringFromQString(const QString & s)
{
    const char * utf8String = s.toUtf8().constData();
    return [[NSString alloc] initWithUTF8String: utf8String];
}

NSView * nsViewFromWidget(QWidget * w)
{
    return (NSView *)w->winId();
}

void dockClickHandler(id self, SEL _cmd)
{
    Q_UNUSED(self);
    Q_UNUSED(_cmd);
    ms_instance->emitDockClick();
}

MacSupport::MacSupport() {
    Class cls = [[[NSApplication sharedApplication] delegate] class];

    if (!class_addMethod(cls, @selector(applicationShouldHandleReopen:hasVisibleWindows:), (IMP) dockClickHandler, "v@:"))
        NSLog(@"MyPrivate::MyPrivate() : class_addMethod failed!");

    ms_instance = this;
    qDebug() << "MacSupport: constructor";
}

MacSupport::~MacSupport() {
    qDebug() << "MacSupport: destructor";
}

void MacSupport::emitDockClick() {
    qDebug() << "MacSupport: dockClicked";
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
    static QWidget * currentOverlay = NULL;
    if (currentOverlay)
        currentOverlay->deleteLater();
    currentOverlay = overlay;
    if (overlay)
    {
        NSView * overlayView = nsViewFromWidget(overlay);
        [[NSApp dockTile] setContentView: overlayView];
        overlay->update();
    }
    else
    {
        [[NSApp dockTile] setContentView: nil];
    }
    [[NSApp dockTile] display];
}

@interface DrawHelper : NSObject
{
}

- (float)roundedCornerRadius;
- (void)drawRectOriginal:(NSRect)rect;
- (NSWindow*)window;
- (id)_displayName;
- (NSRect)bounds;

- (void)drawRect:(NSRect)rect;
- (void) _drawTitleStringIn: (NSRect) rect withColor: (NSColor *) color;
@end

@implementation DrawHelper

- (void)drawRect:(NSRect)rect
{
    // Call original drawing method
    [self drawRectOriginal:rect];
    [self _setTextShadow:NO];

    NSRect titleRect;

    NSRect brect = [self bounds];

    float radius = [self roundedCornerRadius];
    NSBezierPath *path = [NSBezierPath alloc];
    NSPoint topMid = NSMakePoint(NSMidX(brect), NSMaxY(brect));
    NSPoint topLeft = NSMakePoint(NSMinX(brect), NSMaxY(brect));
    NSPoint topRight = NSMakePoint(NSMaxX(brect), NSMaxY(brect));
    NSPoint bottomRight = NSMakePoint(NSMaxX(brect), NSMinY(brect));

    [path moveToPoint: topMid];
    [path appendBezierPathWithArcFromPoint: topRight
        toPoint: bottomRight
        radius: radius];
    [path appendBezierPathWithArcFromPoint: bottomRight
        toPoint: brect.origin
        radius: radius];
    [path appendBezierPathWithArcFromPoint: brect.origin
        toPoint: topLeft
        radius: radius];
    [path appendBezierPathWithArcFromPoint: topLeft
        toPoint: topRight
        radius: radius];
    [path closePath];

    [path addClip];
    titleRect = NSMakeRect(0, 0, brect.size.width, brect.size.height);

    CGContextRef context = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
    CGContextSetBlendMode(context, kCGBlendModeMultiply);

    // background
    if (gFrameColor == nil)
    {
        NSLog(@"frame color is nil, setting default");
        gFrameColor = [[NSColor colorWithCalibratedRed: (50 / 255.0) green: (50 / 255.0) blue: (50 / 255.0) alpha: 1.0] retain];
    }

    [gFrameColor set];

    [[NSBezierPath bezierPathWithRect:rect] fill];

    CGContextSetBlendMode(context, kCGBlendModeCopy);
    // draw title text
    [self _drawTitleStringIn: titleRect withColor: nil];
}

- (void)_drawTitleStringIn: (NSRect) rect withColor: (NSColor *) color
{
    if (!gTitleColor)
        gTitleColor = [[NSColor colorWithCalibratedRed: .6 green: .6 blue: .6 alpha: 1.0] retain];
    [self _drawTitleStringOriginalIn: rect withColor: gTitleColor];
}

@end

@class NSThemeFrame;

void MacSupport::installCustomFrame()
{
    id _class = [NSThemeFrame class];

    // Exchange drawRect:
    Method m0 = class_getInstanceMethod([DrawHelper class], @selector(drawRect:));
    class_addMethod(_class, @selector(drawRectOriginal:), method_getImplementation(m0), method_getTypeEncoding(m0));

    Method m1 = class_getInstanceMethod(_class, @selector(drawRect:));
    Method m2 = class_getInstanceMethod(_class, @selector(drawRectOriginal:));

    method_exchangeImplementations(m1, m2);

    // Exchange _drawTitleStringIn:withColor:
    Method m3 = class_getInstanceMethod([DrawHelper class], @selector(_drawTitleStringIn:withColor:));
    class_addMethod(_class, @selector(_drawTitleStringOriginalIn:withColor:), method_getImplementation(m3), method_getTypeEncoding(m3));

    Method m4 = class_getInstanceMethod(_class, @selector(_drawTitleStringIn:withColor:));
    Method m5 = class_getInstanceMethod(_class, @selector(_drawTitleStringOriginalIn:withColor:));

    method_exchangeImplementations(m4, m5);
}

void MacSupport::setCustomBorderColor(const QColor & frameColor)
{
    if (gFrameColor)
        [gFrameColor release];
    gFrameColor = [[NSColor colorWithCalibratedRed: frameColor.redF() green: frameColor.greenF() blue: frameColor.blueF() alpha: frameColor.alphaF()] retain];
    foreach (QWidget * w, QApplication::topLevelWidgets())
        w->repaint();
}

void MacSupport::setCustomTitleColor(const QColor & titleColor)
{
    if (gTitleColor)
        [gTitleColor release];
    gTitleColor = [[NSColor colorWithCalibratedRed: titleColor.redF() green: titleColor.greenF() blue: titleColor.blueF() alpha: titleColor.alphaF()] retain];
    foreach (QWidget * w, QApplication::topLevelWidgets())
        w->repaint();
}

void MacSupport::requestAttention()
{
    [NSApp requestUserAttention: NSInformationalRequest];
}
