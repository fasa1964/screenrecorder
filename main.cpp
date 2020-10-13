#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("FScreenRecorder");
    app.setApplicationVersion("1.0");

    MainWindow w;
    w.show();

    return app.exec();
}
