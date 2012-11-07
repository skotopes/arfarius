#ifndef HISTOGRAMWIDGET_H
#define HISTOGRAMWIDGET_H

#include <QWidget>

class HistogramWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HistogramWidget(QWidget *parent = 0);
    
protected:
    void paintEvent(QPaintEvent *event);
    void mouseReleaseEvent(QMouseEvent *);

private:
    float play_pointer;

public slots:
    void updatePlayPointer(float p);

signals:
    void newPlayPointer(float p);
};

#endif // HISTOGRAMWIDGET_H
