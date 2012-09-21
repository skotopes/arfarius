#ifndef MACSUPPORT_H
#define MACSUPPORT_H

#include <QObject>

class QColor;

class MacSupport : public QObject
{
    Q_OBJECT
public:
    MacSupport();
    virtual ~MacSupport();
    void emitDockClick();
    void emitMediaKeys();

public slots:
    void setDockBadge(const QString & badgeText);
    void setDockOverlay(QWidget * overlay);

    void installCustomFrame();
    void setCustomBorderColor(const QColor & color);
    void setCustomTitleColor(const QColor & color);

    void requestAttention();

signals:
    void dockClicked();
    void playPause();
    void next(bool state);
    void prev(bool state);
};

#endif
