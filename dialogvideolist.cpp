#include "dialogvideolist.h"
#include "ui_dialogvideolist.h"

DialogVideoList::DialogVideoList(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogVideoList)
{
    ui->setupUi(this);
}

DialogVideoList::~DialogVideoList()
{
    delete ui;
}
