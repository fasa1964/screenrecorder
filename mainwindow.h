#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScreen>
#include <QAudioRecorder>
#include <QAudioDeviceInfo>
#include <QMediaRecorder>
#include <QTimer>
#include <QTime>
#include <QMap>
#include <QThread>
#include <QProcess>
#include <QSettings>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <dialogvideolist.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void closeButtonClicked();
    void recordButtonClicked();
    void mergeButtonClicked();
    void infoButtonClicked();
    void recordStarted();
    void readyReadStandardOutput();
    void readyReadStandardError();
    void recordFinished(int exitCode, QProcess::ExitStatus status);
    void processStateChanged(QProcess::ProcessState);

    void timout();

    void dirButtonClicked();
    void ffmpegDirButtonClicked();

    void checkUpdate();
    void replyFinished(QNetworkReply *reply);

private:
    Ui::MainWindow *ui;

    QProcess *recordProcess;

    QTimer *recordTimer;
    bool merging;

    DialogVideoList *videolist;
    void readSettings();
    void saveSettings();
};

#endif // MAINWINDOW_H
