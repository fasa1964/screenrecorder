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
#include <QWinTaskbarProgress>
#include <QWinTaskbarButton>

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

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event) override;
    virtual void showEvent(QShowEvent *event);


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
    void timlineSliderChanged(int value);

    // SIGNALS from QTimer
    void timeout();

    // SIGNALS for ffmpeg
    void readyReadStandardOutput();
    void recordProcessStateChanged(QProcess::ProcessState status);
    void processFinished(int exitCode, QProcess::ExitStatus status);
    void cutProcessFinished(int exitCode, QProcess::ExitStatus status);


    void readyReadInfoStandardOutput();

    void handleSliderLeftValueChanged(int value);
    void handleSliderRightValueChanged(int value);

private:
    Ui::MainWindow *ui;

    // Process for ffmpeg
    QProcess *recordProcess;
    QProcess *infoProcess;
    QProcess *cutProcess;

    // Take care of record time
    // ffmpeg stop recording video by -t duration
    // but audio keeps recording
    QTimer *recordTimer;
    QTime startTime;        // start time when recording
    int elapsedSeconds;     // elapsed seconds since recording
    int durationSeconds;    // record time in seconds
    bool merging;           // status for merging recorded videos


    // When recording an application
    // is minimized show button and progress for record time
    QWinTaskbarButton *taskbarButton;
    QWinTaskbarProgress *taskbarProgress;

    // For the tools tab
    void updateVideoListWidget();


    QStringList availableVideos(QDir dir, const QStringList &filters);
    QString getText(const QString &sourceText, const QString &fromText, const QChar &tilChar);
    bool checkPath();
    void readSettings();
    void saveSettings();
};

#endif // MAINWINDOW_H
