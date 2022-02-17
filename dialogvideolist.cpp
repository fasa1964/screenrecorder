#include "dialogvideolist.h"
#include "ui_dialogvideolist.h"

DialogVideoList::DialogVideoList(const QStringList &availableVideos, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogVideoList)
{
    ui->setupUi(this);
    setWindowTitle(tr("Videolist"));
    setupListWidget(availableVideos);

    connect(ui->cancelButton, &QPushButton::clicked, this, &DialogVideoList::reject);
    connect(ui->applyButton, &QPushButton::clicked, this, &DialogVideoList::accept);
    connect(ui->listWidget, &QListWidget::itemClicked, this, &DialogVideoList::itemClicked);
    connect(ui->videoNameEdit, &QLineEdit::textChanged, this, &DialogVideoList::videoNameEditTextChanged);
}

DialogVideoList::~DialogVideoList()
{
    delete ui;
}

QStringList DialogVideoList::request()
{
    return selectedItems();
}

QString DialogVideoList::getVideoName()
{
    return ui->videoNameEdit->text();
}

void DialogVideoList::itemClicked(QListWidgetItem */*item*/)
{
    if(selectedItems().size() < 2 || ui->videoNameEdit->text().isEmpty())
        ui->applyButton->setEnabled(false);
    else
        ui->applyButton->setEnabled(true);

}

void DialogVideoList::videoNameEditTextChanged(const QString &text)
{
    if(selectedItems().count() > 1 && !text.isEmpty())
        ui->applyButton->setEnabled(true);
    else
        ui->applyButton->setEnabled(false);
}

void DialogVideoList::setupListWidget(const QStringList &vList)
{
    ui->listWidget->clear();
    foreach (QString s, vList) {
        QListWidgetItem *item = new QListWidgetItem(s);
        item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
        item->setCheckState(Qt::Unchecked);
        ui->listWidget->addItem(item);
    }
}

QStringList DialogVideoList::selectedItems()
{
    QStringList list;
    for(int i = 0; i < ui->listWidget->count(); i++)
    {
        QListWidgetItem *item = ui->listWidget->item(i);
        if(item->checkState() == Qt::Checked)
           list << item->text();
    }

    return list;
}
