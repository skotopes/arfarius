#include "histogramwidget.h"
#include "playlistitem.h"
#include <QtGui>

HistogramWidget::HistogramWidget(QWidget *parent)
    : QOpenGLWidget(parent), progress(-1), image(nullptr)
{
    setAttribute(Qt::WA_AlwaysStackOnTop);
}

void HistogramWidget::mouseReleaseEvent(QMouseEvent *e)
{
    float p = (float) e->pos().x() / width();
    emit clicked(p);
    e->accept();
}

void HistogramWidget::paintGL()
{
    QPainter painter(this);
    if (image) {
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.drawImage(QRect(0,0,width(),height()), *image);
    }

    glLogicOp(GL_XOR);
    glEnable(GL_COLOR_LOGIC_OP);
    float p = progress * width();
    painter.drawLine(QLineF(p, 0, p, height()));
    glDisable(GL_COLOR_LOGIC_OP);
}

void HistogramWidget::updateProgress(float p)
{
    progress = p;
    update();
}

void HistogramWidget::updateImage(QImage *i)
{
    qDebug() << this << "updateImage(QImage *)" << image << "->" << i;
    QImage *temp = image;
    image = i;
    if (temp) delete temp;
    update();
}

