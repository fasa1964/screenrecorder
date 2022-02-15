#include "formwidgetproccess.h"
#include "ui_formwidgetproccess.h"

FormWidgetProccess::FormWidgetProccess(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormWidgetProccess)
{
    ui->setupUi(this);
}

FormWidgetProccess::~FormWidgetProccess()
{
    delete ui;
}
