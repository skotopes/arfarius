#ifndef HISTOGRAMWIDGET_H
#define HISTOGRAMWIDGET_H

#include <QWidget>
#include "avfile.h"

class HistogramWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HistogramWidget(QWidget *parent = 0);
    
protected:
    void paintEvent(QPaintEvent *event);
    void mouseReleaseEvent(QMouseEvent *);

private:
    AVFile::Progress progress;

public slots:
    void updatePlayProgress(AVFile::Progress);

signals:
    void newPlayPointer(float p);
};

#endif // HISTOGRAMWIDGET_H
