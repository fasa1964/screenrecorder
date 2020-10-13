#ifndef FORMVIDEOLIST_H
#define FORMVIDEOLIST_H

#include <QWidget>

namespace Ui {
class FormVideoList;
}

class FormVideoList : public QWidget
{
    Q_OBJECT

public:
    explicit FormVideoList(QWidget *parent = 0);
    ~FormVideoList();

private:
    Ui::FormVideoList *ui;
};

#endif // FORMVIDEOLIST_H
