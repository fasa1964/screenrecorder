#ifndef DIALOGVIDEOLIST_H
#define DIALOGVIDEOLIST_H

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class DialogVideoList;
}

class DialogVideoList : public QDialog
{
    Q_OBJECT

public:
    explicit DialogVideoList(const QStringList &availableVideos, QWidget *parent = 0);
    ~DialogVideoList();

    QStringList request();
    QString getVideoName();

private slots:

    void itemClicked(QListWidgetItem */*item*/);

private:
    Ui::DialogVideoList *ui;

    void setupListWidget(const QStringList &vList);
    QStringList selectedItems();
};

#endif // DIALOGVIDEOLIST_H
