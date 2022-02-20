#ifndef FORMMESSAGEWIDGET_H
#define FORMMESSAGEWIDGET_H

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QRect>

#include <QTimer>

namespace Ui {
class FormMessageWidget;
}

class FormMessageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FormMessageWidget(QWidget *parent = nullptr);
    ~FormMessageWidget();

    virtual void paintEvent(QPaintEvent *);

    void setLabelText(const QString &text);

    const QRect &rect() const;
    void setRect(const QRect &newRect);
    void setPos(int x , int y);


    bool showText() const;
    void setShowText(bool newShowText);

private:
    Ui::FormMessageWidget *ui;
    QTimer *timer;
    QRect m_rect;
    bool m_showText;
};

#endif // FORMMESSAGEWIDGET_H
