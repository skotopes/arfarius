#include "histogramwidget.h"

#include <QtGui>

QString formatTime(size_t time)
{
    int h,m,s;

    s = time % 60;
    m = time / 60 % 60;
    h = time / 60 / 60;

    if (h > 0)
        return QString("%1:%2:%3").arg(h, 2).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
    else
        return QString("%1:%2").arg(m, 2).arg(s, 2, 10, QChar('0'));
}


HistogramWidget::HistogramWidget(QWidget *parent)
    : QWidget(parent), progress()
{
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
    int p = progress.position / progress.duration * width();
    painter.drawLine(p, 0, p, height());

    painter.setPen(lightGreenColor);
    painter.drawText(0, height(), formatTime(progress.position));

    painter.setPen(lightRedColor);
    painter.drawText(0, 10, formatTime(progress.duration));
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
