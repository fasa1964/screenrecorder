#ifndef DIALOGVIDEOLIST_H
#define DIALOGVIDEOLIST_H

#include <QDialog>

namespace Ui {
class DialogVideoList;
}

class DialogVideoList : public QDialog
{
    Q_OBJECT

public:
    explicit DialogVideoList(QWidget *parent = 0);
    ~DialogVideoList();

private:
    Ui::DialogVideoList *ui;
};

#endif // DIALOGVIDEOLIST_H
