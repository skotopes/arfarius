#ifndef HISTOGRAMWIDGET_H
#define HISTOGRAMWIDGET_H

#include <QWidget>

class QImage;

class HistogramWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HistogramWidget(QWidget *parent = 0);
    
protected:
    void paintEvent(QPaintEvent *event);
    void mouseReleaseEvent(QMouseEvent *);

private:
    float progress;
    QImage *image;

public slots:
    void updatePlayProgress(float);
    void updateImage(QImage *);

signals:
    void newPlayPointer(float);
};

#endif // HISTOGRAMWIDGET_H
