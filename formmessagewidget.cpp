#include "formmessagewidget.h"
#include "ui_formmessagewidget.h"

#include <QStyleOption>
#include <QStyle>
#include <QPalette>
#include <QColor>
#include <QString>
#include <QFont>
#include <QLayout>
#include <QStyle>
#include <QStyleFactory>

FormMessageWidget::FormMessageWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormMessageWidget)
{
    ui->setupUi(this);

    setShowText(true);



    setAttribute(Qt::WA_TranslucentBackground, true);
    //setAttribute(Qt::WA_AlwaysStackOnTop, true);
    //setAttribute(Qt::WA_ShowWithoutActivating, true);


    // Test
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_SetStyle, true);



    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    //setWindowFlags(Qt::FramelessWindowHint);


    this->setStyle( QStyleFactory::create("Fusion"));
}

FormMessageWidget::~FormMessageWidget()
{
    delete ui;
}

void FormMessageWidget::paintEvent(QPaintEvent */*event*/)
{


    QPainter painter(this);

    // Setting paint color
    QPalette pal = this->palette();

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    pal.setColor(QPalette::Normal, QPalette::Foreground, QColor(Qt::red) );
    pal.setColor(QPalette::Inactive, QPalette::Foreground, QColor(Qt::red) );
#else
    pal.setColor(QPalette::Normal, QPalette::WindowText, QColor(Qt::red) );
    pal.setColor(QPalette::Inactive, QPalette::WindowText, QColor(Qt::red) );
#endif

    this->setPalette(pal);


    painter.drawRect(rect());

    // When not recording show text
    if(showText()){
        QFont font = painter.font();
        font.setPixelSize(30);
        //font.setPixelSize( geometry().width()%45 ? 30 : 20  );
        painter.setFont(font);


        QString textX = QString("Pos x: %1").arg( geometry().x() ,10);
        QString textY = QString("Pos y: %1").arg(geometry().y() ,10);
        QString textWidth = QString("Width: %1").arg(rect().width()+1,10);
        QString textHeight = QString("Height: %1").arg(rect().height()+1,10);


        painter.drawText( 100,100, textX);
        painter.drawText( 100,140, textY );
        painter.drawText( 100,180, textWidth );
        painter.drawText( 100,220, textHeight );
    }

}

void FormMessageWidget::setLabelText(const QString &text)
{
    ui->label->setText(text);
    //QTimer::singleShot(5000, this, SLOT(hide()));
}

const QRect &FormMessageWidget::rect() const
{
    return m_rect;
}

void FormMessageWidget::setRect(const QRect &newRect)
{
    m_rect = newRect;
    m_rect = QRect(0,0, geometry().width()-1 , geometry().height()-1);
//    setGeometry(newRect);
//    update();
}

void FormMessageWidget::setPos(int x, int y)
{
    setGeometry(x,y, geometry().width(), geometry().height());
    //setGeometry(x,y, rect().width(), rect().height());
    update();
}

bool FormMessageWidget::showText() const
{
    return m_showText;
}

void FormMessageWidget::setShowText(bool newShowText)
{
    m_showText = newShowText;
    update();
}


