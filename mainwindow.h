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
#include <QFocusEvent>
#include <QResizeEvent>
#include <QWindowStateChangeEvent>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEvent>

#ifdef Q_OS_WIN32
    #include <QWinTaskbarProgress>
    #include <QWinTaskbarButton>
#endif

#ifdef Q_OS_LINUX
    #include <QDBusMessage>
    #include <QDBusConnection>
#endif

#include <dialogvideolist.h>
#include <formwidgetproccess.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;


private slots:
    void closeButtonClicked();
    void recordButtonClicked();
    void infoButtonClicked();
    void mergeButtonClicked();
    void cutButtonClicked();

    void ffmpegPathButtonClicked();
    void outputPathButtonClicked();

    // For the tools tab
    void tabIndexChanged(int index);
    void itemClicked(QListWidgetItem *item);
    void handleSliderLeftValueChanged(int value);
    void handleSliderRightValueChanged(int value);


    // SIGNALS from QTimer
    void timeout();

    // SIGNALS for ffmpeg
    void readyReadStandardOutput();
    void recordProcessStateChanged(QProcess::ProcessState status);
    void processFinished(int exitCode, QProcess::ExitStatus status);
    void cutProcessFinished(int exitCode, QProcess::ExitStatus status);

    // SIGNALS from infoProcess
    void readyReadInfoStandardOutput();

    // SIGNALS from DialogInfo
    void checkUpdate();

private:
    Ui::MainWindow *ui;

    // Process for ffmpeg
    QProcess *recordProcess;
    QProcess *infoProcess;
    QProcess *cutProcess;

    // Test widget proccess
    QProcess *widgetProccess;
    QThread *widgetThread;
    FormWidgetProccess *widget;

    // Take care of record time
    // ffmpeg stop recording video by -t duration
    // but audio keeps recording
    QTimer *recordTimer;
    QTime startTime;        // start time when recording
    int elapsedSeconds;     // elapsed seconds since recording
    int durationSeconds;    // record time in seconds
    bool merging;           // status for merging recorded videos


    // When recording an application is minimized
    // show button and progress for record time
    // Only for windows
#ifdef Q_OS_WIN32
    QWinTaskbarButton *taskbarButton;
    QWinTaskbarProgress *taskbarProgress;
#endif


    QString platform;
    void setPlatformInfo();

    // For the tools tab
    void updateVideoListWidget();

    QStringList availableVideos(QDir dir, const QStringList &filters);
    QString getText(const QString &sourceText, const QString &fromText, const QChar &tilChar);
    double getDuration(const QString &s);
    bool checkPath();
    void readSettings();
    void saveSettings();
};

#endif // MAINWINDOW_H
