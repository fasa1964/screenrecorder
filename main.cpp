#include "mainwindow.h"
#include <QApplication>
#include <QIcon>
#include <QPixmap>
#include <QStyle>
#include <QStyleFactory>

#ifdef Q_OS_LINUX
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";
    switch (type) {
    case QtDebugMsg:
        //fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        fprintf(stderr, "Debug: %s \n", localMsg.constData());
        break;
    case QtInfoMsg:
        fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    }
}
#endif

int main(int argc, char *argv[])
{

#ifdef Q_OS_LINUX
    qInstallMessageHandler(myMessageOutput);
#endif

    QApplication app(argc, argv);

    app.setApplicationName("FScreenRecorder");
    app.setApplicationVersion("1.1");
    app.setWindowIcon(QIcon(":/FScreenRecorder.ico"));
    app.setDesktopFileName("FScreenRecorder");
    //app.setStyle("windows"); // Looks old fashion window
    app.setStyle( QStyleFactory::create("Fusion") );

    MainWindow w;
    w.show();

    return app.exec();
}
