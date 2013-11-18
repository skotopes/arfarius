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

public slots:
    void setDockBadge(const QString & badgeText);
    void setDockOverlay(QWidget * overlay);

    void requestAttention();

signals:
    void dockClicked();
};

#endif
