#ifndef HISTOGRAMWIDGET_H
#define HISTOGRAMWIDGET_H

#include <QGLWidget>

class HistogramWidget : public QGLWidget
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
    void updateProgress(float);
    void updateImage(QImage *);

signals:
    void clicked(float);
};

#endif // HISTOGRAMWIDGET_H
