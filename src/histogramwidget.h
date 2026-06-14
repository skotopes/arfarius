#ifndef HISTOGRAMWIDGET_H
#define HISTOGRAMWIDGET_H

#include <QOpenGLWidget>

class HistogramWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit HistogramWidget(QWidget *parent = 0);
    
protected:
    void mouseReleaseEvent(QMouseEvent *);
    void paintGL();

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
