#ifndef DIALOGINFO_H
#define DIALOGINFO_H

#include <QDialog>

namespace Ui {
class DialogInfo;
}

class DialogInfo : public QDialog
{
    Q_OBJECT

public:
    explicit DialogInfo(QWidget *parent = 0);
    ~DialogInfo();

signals:
    void checkForUpdate();

private slots:
    void qtButtonClicked();
    void updateButtonClicked();

private:
    Ui::DialogInfo *ui;

};

#endif // DIALOGINFO_H
