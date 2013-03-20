#include "timewidget.h"

#include <QtGui>

QString formatTime(size_t time)
{
    int h,m,s;

    s = time % 60;
    m = time / 60 % 60;
    h = time / 60 / 60;

    if (h > 0)
        return QString("%1:%2:%3").arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
    else
        return QString("%1:%2").arg(m).arg(s, 2, 10, QChar('0'));
}

TimeWidget::TimeWidget(QWidget *parent) :
    QWidget(parent)
{
}

void TimeWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QFont   textFont("Monaco", 10);
    painter.setFont(textFont);

    QColor  lightGreenColor(230, 230, 230);
    painter.setPen(lightGreenColor);

    painter.drawText(rect(), Qt::AlignVCenter|Qt::AlignHCenter, formatTime(progress.position) + "/" + formatTime(progress.duration));
}

void TimeWidget::mouseReleaseEvent(QMouseEvent *e)
{
//    float p = (float) e->pos().x() / width();
    e->accept();
}

void TimeWidget::updatePlayProgress(AVFile::Progress p)
{
    progress = p;
    update();
}
