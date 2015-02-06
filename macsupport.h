#ifndef MACSUPPORT_H
#define MACSUPPORT_H

#include <QObject>

class QColor;

class MacSupport : public QObject
{
    Q_OBJECT
public:
    MacSupport(QObject *parent = 0);
    virtual ~MacSupport();

    void emitDockClick();
    void emitKeyEvent(int keycode, int keystate);

public slots:
    void setDockBadge(const QString & badgeText);
    void setDockOverlay(QWidget * overlay);

    void requestAttention();

signals:
    void prev();
    void play();
    void next();
};

#endif
