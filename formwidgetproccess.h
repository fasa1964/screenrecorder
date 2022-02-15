#ifndef FORMWIDGETPROCCESS_H
#define FORMWIDGETPROCCESS_H

#include <QWidget>

namespace Ui {
class FormWidgetProccess;
}

class FormWidgetProccess : public QWidget
{
    Q_OBJECT

public:
    explicit FormWidgetProccess(QWidget *parent = nullptr);
    ~FormWidgetProccess();

private:
    Ui::FormWidgetProccess *ui;
};

#endif // FORMWIDGETPROCCESS_H
