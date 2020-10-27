#include "mainwindow.h"
#include <QApplication>
#include <QIcon>
#include <QPixmap>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("FScreenRecorder");
    app.setApplicationVersion("1.1");
    app.setWindowIcon(QIcon(":/FScreenRecorder.ico"));

    MainWindow w;
    w.show();

    return app.exec();
}
