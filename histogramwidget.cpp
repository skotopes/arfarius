#include "histogramwidget.h"

#include <QtGui>

HistogramWidget::HistogramWidget(QWidget *parent)
    : QWidget(parent), progress(-1), image(0)
{
}

void HistogramWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    if (image) {
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.drawImage(QRect(0,0,width(),height()), *image);
    }

    painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    painter.setPen(QColor(255,255,255));
    int p = progress * width();
    painter.drawLine(p, 0, p, height());
}

void HistogramWidget::mouseReleaseEvent(QMouseEvent *e)
{
    float p = (float) e->pos().x() / width();
    emit newPlayPointer(p);
    e->accept();
}

void HistogramWidget::updatePlayProgress(float p)
{
    progress = p;
    update();
}

void HistogramWidget::updateImage(QImage *i)
{
    QImage *t = image;
    image = i;
    delete t;
    update();
}
