#include "dialoginfo.h"
#include "ui_dialoginfo.h"

#include <QMessageBox>
#include <QPixmap>

DialogInfo::DialogInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogInfo)
{
    ui->setupUi(this);
    setWindowTitle(tr("About FScreenRecorder"));

    ui->labelVersion->setText("Version "+qApp->applicationVersion());
    ui->labelIcon->setPixmap(QPixmap(":/FScreenRecorder128.png"));

    connect(ui->closeButton, &QPushButton::clicked, this, &DialogInfo::close);
    connect(ui->updateButton, &QPushButton::clicked, this, &DialogInfo::updateButtonClicked);
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

void DialogInfo::updateButtonClicked()
{
    emit checkForUpdate();
    //close();
}
