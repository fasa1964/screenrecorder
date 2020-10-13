#include "formvideolist.h"
#include "ui_formvideolist.h"

FormVideoList::FormVideoList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormVideoList)
{
    ui->setupUi(this);
}

FormVideoList::~FormVideoList()
{
    delete ui;
}
