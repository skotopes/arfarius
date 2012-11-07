#include "histogramwidget.h"

#include <QtGui>

HistogramWidget::HistogramWidget(QWidget *parent)
    : QWidget(parent), play_pointer(0)
{
//    QTimer *timer = new QTimer(this);
//    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
//    timer->start(1000);
}

void HistogramWidget::paintEvent(QPaintEvent *)
{
    QColor  lightGrayColor(128, 128, 128);
    QColor  lightGreenColor(200, 255, 200);
    QColor  lightRedColor(255, 200, 200);

    QColor  redColor(255, 55, 55);

    QFont   textFont("Arial", 10);

    QPainter painter(this);
    painter.setFont(textFont);

    painter.setPen(lightGrayColor);
    painter.drawLine(0, height()/2, width(), height()/2);

    painter.setPen(redColor);
    int p = play_pointer * width();
    painter.drawLine(p, 0, p, height());

    painter.setPen(lightGreenColor);
    painter.drawText(0, height(), "00:00");

    painter.setPen(lightRedColor);
    painter.drawText(0, 10, "00:00");
}

void HistogramWidget::mouseReleaseEvent(QMouseEvent *e)
{
    float p = width() / e->pos().x();
    emit newPlayPointer(p);
    e->accept();
}

void HistogramWidget::updatePlayPointer(float p)
{
    play_pointer = p;
    update();
}
