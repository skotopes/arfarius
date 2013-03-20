#ifndef TIMEWIDGET_H
#define TIMEWIDGET_H

#include <QWidget>
#include "avfile.h"

class TimeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TimeWidget(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *event);
    void mouseReleaseEvent(QMouseEvent *);

private:
    AVFile::Progress progress;

public slots:
    void updatePlayProgress(AVFile::Progress);

};

#endif // TIMEWIDGET_H
