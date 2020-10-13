#include "dialoginfo.h"
#include "ui_dialoginfo.h"

DialogInfo::DialogInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogInfo)
{
    ui->setupUi(this);
    setWindowTitle(tr("About FScreenRecorder"));

    connect(ui->closeButton, &QPushButton::clicked, this, &DialogInfo::close);
}

DialogInfo::~DialogInfo()
{
    delete ui;
}
