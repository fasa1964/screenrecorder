#ifndef SLIDERTWOHANDLE_H
#define SLIDERTWOHANDLE_H

#include <QWidget>
#include <QSlider>
#include <QMouseEvent>
#include <QResizeEvent>

#include <QPaintEvent>

class SliderTwoHandle : public QSlider
{
    Q_OBJECT
public:
    explicit SliderTwoHandle(QWidget *parent = nullptr);

    virtual void paintEvent(QPaintEvent *) override;

    qreal minimum() const;
    void setMinimum(const qreal &minimum);

    qreal maximum() const;
    void setMaximum(const qreal &maximum);
    void setRange(int min, int max);

    int diffValue() const;
    void setDiffValue(int diffValue);

    bool showText() const;
    void setShowText(bool showText);

    QColor color() const;
    void setColor(const QColor &color);

    QColor textColor() const;
    void setTextColor(const QColor &textColor);

    QColor selectedColor() const;
    void setSelectedColor(const QColor &selectedColor);

protected:
    virtual void mouseMoveEvent(QMouseEvent *ev) override;
    virtual void mousePressEvent(QMouseEvent *ev) override;
    virtual void mouseReleaseEvent(QMouseEvent *ev) override;
    virtual void resizeEvent(QResizeEvent *event) override;

signals:
    void leftValueChanged(int value);
    void rightValueChanged(int value);

public slots:

private:
    QRectF leftHandle();
    QRectF rightHandle();

    void drawTrack(QPainter *p);

    qreal x1;
    qreal x2;
    qreal lValue;
    qreal rValue;
    bool leftHandleSelect;
    bool rightHandleSelect;

    qreal m_minimum;
    qreal m_maximum;
    int m_diffValue;

    bool m_showText;
    QColor m_textColor;
    QColor m_color;
    QColor m_selectedColor;

};

#endif // SLIDERTWOHANDLE_H
