#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScreen>
#include <QtGlobal>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QAudioRecorder>
#include <QAudioDeviceInfo>
#include <QMediaRecorder>
#endif

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

//#include <QxtGlobalShortcut>


/// !brief Test widget style option
#include <formmessagewidget.h>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#ifdef Q_OS_WIN32
    #include <QWinTaskbarProgress>
    #include <QWinTaskbarButton>
#endif
#endif


//#ifdef Q_OS_WIN64
//#include <QWinTaskbarProgress>
//#include <QWinTaskbarButton>
//#endif

#ifdef Q_OS_LINUX
    #include <QDBusMessage>
    #include <QDBusConnection>
#endif

#include <dialogvideolist.h>


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

    // Test AudioDevice adding maualy
    void addAudioDeviceButtonClicked();

    void ffmpegPathButtonClicked();
    void outputPathButtonClicked();

    // Signal from device tab
    void deviceSelectionChanged(const QString &text);
    void sliderValueChanged(int);

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

    // Test AudioDevice adding maualy
    // Set the widgets hidden or showing
    // depend on AudioDeviceInfo
    void setManualAudioEditVisible(bool visible);


    // Take care of record time
    // ffmpeg stop recording video by -t duration
    // but audio keeps recording
    QTimer *recordTimer;
    QTime startTime;        // start time when recording
    int elapsedSeconds;     // elapsed seconds since recording
    int durationSeconds;    // record time in seconds
    bool merging;           // status for merging recorded videos


    // Test FormMessageWidget for another style
    // and thread
    FormMessageWidget *testWidget;


    // When recording an application is minimized
    // show button and progress for record time
    // Only for windows
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#ifdef Q_OS_WIN32
    QWinTaskbarButton *taskbarButton;
    QWinTaskbarProgress *taskbarProgress;
#endif
#endif


    QString platform;
    void setPlatformInfo();

    // For the tools tab
    void updateVideoListWidget();

    QString getVideoName();
    QStringList availableVideos(QDir dir, const QStringList &filters);
    QString getText(const QString &sourceText, const QString &fromText, const QChar &tilChar);
    double getDuration(const QString &s);
    bool checkPath();
    void readSettings();
    void saveSettings();
};

#endif // MAINWINDOW_H
