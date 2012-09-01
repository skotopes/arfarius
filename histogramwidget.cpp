#include "histogramwidget.h"

#include <QtGui>

HistogramWidget::HistogramWidget(QWidget *parent)
    : QWidget(parent)
{
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(1000);
}

void HistogramWidget::paintEvent(QPaintEvent *)
{
    QColor axisColor(128, 128, 128);
    QPainter painter(this);
    painter.setPen(axisColor);
    painter.drawLine(0, height()/2, width(), height()/2);

    painter.setRenderHint(QPainter::Antialiasing);
}

void HistogramWidget::setProgress(float p)
{

}
