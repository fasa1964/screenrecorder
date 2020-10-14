#include "dialoginfo.h"
#include "ui_dialoginfo.h"

#include <QMessageBox>

DialogInfo::DialogInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogInfo)
{
    ui->setupUi(this);
    setWindowTitle(tr("About FScreenRecorder"));
    ui->labelVersion->setText("Version "+qApp->applicationVersion());

    connect(ui->closeButton, &QPushButton::clicked, this, &DialogInfo::close);
    connect(ui->qtButton, &QPushButton::clicked, this, &DialogInfo::qtButtonClicked);
}

DialogInfo::~DialogInfo()
{
    delete ui;
}

void DialogInfo::qtButtonClicked()
{
    QMessageBox::aboutQt(this);
}
