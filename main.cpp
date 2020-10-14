#include "mainwindow.h"
#include <QApplication>
#include <QIcon>
#include <QPixmap>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("FScreenRecorder");
    app.setApplicationVersion("1.0");
    app.setWindowIcon(QIcon(QPixmap(":/FScreenRecorder.svg")));

    MainWindow w;
    w.show();

    return app.exec();
}
