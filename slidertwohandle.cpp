#include "slidertwohandle.h"

#include <QPainter>
#include <QtMath>

#include <QDebug>

SliderTwoHandle::SliderTwoHandle(QWidget *parent) : QSlider(parent)
{
    setMouseTracking(true);
    setUpdatesEnabled(true);
    x1 = 0.0;
    x2 = rect().width() - rightHandle().width()-1;
    leftHandleSelect = false;
    rightHandleSelect = false;

    // default settings
    setRange(0,100);
    lValue = 0.0;
    rValue = maximum();
    setDiffValue( static_cast<int>(rValue) - static_cast<int>(lValue));

    setShowText(true);
    setColor(QColor(0, 115, 230));
    setSelectedColor(QColor(0, 77, 153));
    setTextColor(Qt::black);
}

void SliderTwoHandle::paintEvent(QPaintEvent *)
{

    QPainter painter(this);

    drawTrack(&painter);

    if(leftHandleSelect)
        painter.fillRect( leftHandle(), QBrush( selectedColor(), Qt::SolidPattern));
    else
        painter.fillRect( leftHandle(), QBrush( color(), Qt::SolidPattern));

    if(rightHandleSelect)
        painter.fillRect( rightHandle(), QBrush( selectedColor() , Qt::SolidPattern));
    else
        painter.fillRect( rightHandle(), QBrush( color(), Qt::SolidPattern));

    painter.drawRect(leftHandle());
    painter.drawRect(rightHandle());

    QPen pen = painter.pen();
    pen.setWidthF(5.0);
    pen.setColor(color());

    if(leftHandleSelect || rightHandleSelect)
        pen.setColor( selectedColor() );

    painter.setPen(pen);
    QLineF line(leftHandle().right(), rect().height()/2 , rightHandle().left(),rect().height()/2);
    painter.drawLine( line );

    if(showText()){
        painter.setPen( textColor() );
        painter.setFont(QFont("Arial", rect().height()/2 ));
        QPointF p1 = leftHandle().topLeft();
        QPointF p2 = rightHandle().bottomRight();
        painter.drawText(QRectF( p1 , p2 ), Qt::AlignCenter, QString::number( diffValue(),10));
    }

    //QWidget::paintEvent(ev);
}

void SliderTwoHandle::mouseMoveEvent(QMouseEvent *ev)
{
    if(leftHandleSelect){
        x1 = ev->pos().x();
        if(x1 < 0)
            x1 = 0;
        if(x1 > rightHandle().left() - leftHandle().width() )
            x1 = rightHandle().left()-leftHandle().width();

        update();

        lValue = (maximum() - minimum()) / (rect().width()-leftHandle().width()-1) * x1 + minimum();
        emit leftValueChanged( qFloor(lValue));
    }

    if(rightHandleSelect){
        x2 = ev->pos().x();
        if(x2 < leftHandle().right())
            x2 = leftHandle().right();
        if(x2 > rect().width() - rightHandle().width())
            x2 = rect().width() - rightHandle().width()-1;

        update();

        rValue = (maximum() - minimum()) / (rect().width()-rightHandle().width()-1) * x2 + minimum();
        emit rightValueChanged( qFloor(rValue));
    }

    setDiffValue( qFloor(rValue) - qFloor(lValue) );

    return QWidget::mouseMoveEvent(ev);
}

void SliderTwoHandle::mousePressEvent(QMouseEvent *ev)
{
    if(ev->pos().x() > leftHandle().left() && ev->pos().x() < leftHandle().right()){
        if(ev->button() == Qt::LeftButton)
            leftHandleSelect = true;
    }
    else
        leftHandleSelect = false;

    if(ev->pos().x() > rightHandle().left() && ev->pos().x() < rightHandle().right()){
        if(ev->button() == Qt::LeftButton)
            rightHandleSelect = true;
    }
    else
        rightHandleSelect = false;

    //return QWidget::mousePressEvent(ev);
}

void SliderTwoHandle::mouseReleaseEvent(QMouseEvent *)
{
    leftHandleSelect = false;
    rightHandleSelect = false;

    //return QWidget::mouseReleaseEvent(ev);
}

void SliderTwoHandle::resizeEvent(QResizeEvent *)
{
//    if(rValue == qFloor( maximum() ))
//        x2 = rect().width() - rightHandle().width() - 1;

//    if(x2 > rect().width() - rightHandle().width() || rValue != maximum() )
//        x2 = (rect().width()-rightHandle().width()-1) / 100 * (rValue - minimum());

    if(rightHandle().x() >= this->width()){
        x2 = this->width() - rightHandle().width()-1;
    }

   //return QWidget::resizeEvent(event);
}

QRectF SliderTwoHandle::leftHandle()
{
    QRectF leftRect(x1, 0, 13, rect().height());
    return leftRect;
}

QRectF SliderTwoHandle::rightHandle()
{
    QRectF rightRect(x2, 0, 13, rect().height());
    return rightRect;
}

void SliderTwoHandle::drawTrack(QPainter *p)
{
    QRectF trackRect(0,(rect().height()/2)-2.5, rect().width()-1, 5);
    QPen pen = p->pen();
    pen.setWidthF(0.5);
    pen.setColor(Qt::darkGray);
    p->setPen(pen);

    p->fillRect(trackRect, QBrush(QColor(217,217,217), Qt::SolidPattern));
    p->drawRect(trackRect);
}

QColor SliderTwoHandle::selectedColor() const
{
    return m_selectedColor;
}

void SliderTwoHandle::setSelectedColor(const QColor &selectedColor)
{
    m_selectedColor = selectedColor;
}

QColor SliderTwoHandle::textColor() const
{
    return m_textColor;
}

void SliderTwoHandle::setTextColor(const QColor &textColor)
{
    m_textColor = textColor;
}

QColor SliderTwoHandle::color() const
{
    return m_color;
}

void SliderTwoHandle::setColor(const QColor &color)
{
    m_color = color;
}

bool SliderTwoHandle::showText() const
{
    return m_showText;
}

void SliderTwoHandle::setShowText(bool showText)
{
    m_showText = showText;
}

int SliderTwoHandle::diffValue() const
{
    return m_diffValue;
}

void SliderTwoHandle::setDiffValue(int diffValue)
{
    m_diffValue = diffValue;
}

qreal SliderTwoHandle::maximum() const
{
    return m_maximum;
}

void SliderTwoHandle::setMaximum(const qreal &maximum)
{
    m_maximum = maximum;
    rValue = maximum;
}

void SliderTwoHandle::setRange(int min, int max)
{
    setMinimum(min);
    setMaximum(max);
    lValue = min;
    rValue = max;
}

qreal SliderTwoHandle::minimum() const
{
    return m_minimum;
}

void SliderTwoHandle::setMinimum(const qreal &minimum)
{
    m_minimum = minimum;
    lValue = minimum;
}
