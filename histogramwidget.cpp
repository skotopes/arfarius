#include "histogramwidget.h"

#include <QtGui>

HistogramWidget::HistogramWidget(QWidget *parent)
    : QWidget(parent), progress()
{
}

void HistogramWidget::paintEvent(QPaintEvent *)
{
    QColor  lightGrayColor(128, 128, 128);
    QColor  lightRedColor(255, 200, 200);

    QColor  redColor(255, 55, 55);

    QPainter painter(this);

    painter.setPen(lightGrayColor);
    painter.drawLine(0, height()/2, width(), height()/2);

    painter.setPen(redColor);
    int p = progress.position / progress.duration * width();
    painter.drawLine(p, 0, p, height());
}

void HistogramWidget::mouseReleaseEvent(QMouseEvent *e)
{
    float p = (float) e->pos().x() / width();
    emit newPlayPointer(p);
    e->accept();
}

void HistogramWidget::updatePlayProgress(AVFile::Progress p)
{
    progress = p;
    update();
}
