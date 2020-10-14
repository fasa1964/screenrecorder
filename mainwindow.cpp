#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtMath>
#include <QDir>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <dialoginfo.h>
#include <QDebug>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("FScreenRecord");
    setWindowIcon(QIcon(QPixmap(":/FScreenRecorder.svg")));

    merging = false;
    readSettings();
    ui->recordButton->setToolTip("Start to record [Ctrl+r]" );

    // Timer for record duration
    recordTimer = new QTimer(this);

    // Process for ffmpeg
    recordProcess = new QProcess(this);


    // Default settings
    //-----------------
    // Videoformat
    QStringList videoFormatList;
    videoFormatList << "mkv" << "mp4";
    ui->videoFormatBox->addItems(videoFormatList);

    // Duration time default value 10 min
    ui->timeEdit->setTime(QTime(0,10,0));

    //audio devices
    int index = 0;
    foreach (QAudioDeviceInfo device, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
    {
          ui->audioDeviceBox->insertItem(index, device.deviceName());
          index++;
    }

    // video device
    ui->videoDeviceBox->addItems(QStringList() << "desktop" );


    connect(ui->closeButton, &QPushButton::clicked, this, &MainWindow::closeButtonClicked);
    connect(ui->dirButton, &QPushButton::clicked, this, &MainWindow::dirButtonClicked);
    connect(ui->ffmpegDirButton, &QPushButton::clicked, this, &MainWindow::ffmpegDirButtonClicked);
    connect(ui->recordButton, &QPushButton::clicked, this, &MainWindow::recordButtonClicked);
    connect(ui->mergeButton, &QPushButton::clicked, this, &MainWindow::mergeButtonClicked);
    connect(ui->infoButton, &QPushButton::clicked, this, &MainWindow::infoButtonClicked);

    connect(recordProcess, &QProcess::started, this, &MainWindow::recordStarted);
    connect(recordProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::readyReadStandardOutput);
    connect(recordProcess, &QProcess::readyReadStandardError, this, &MainWindow::readyReadStandardError);
    connect(recordProcess, &QProcess::stateChanged, this, &MainWindow::processStateChanged);
    connect(recordProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
         [=](int exitCode, QProcess::ExitStatus exitStatus){ recordFinished(exitCode, exitStatus); });

}

MainWindow::~MainWindow()
{
    if(recordTimer != nullptr)
        recordTimer->deleteLater();
    delete ui;
}

void MainWindow::closeButtonClicked()
{
    if(recordProcess->state() == QProcess::Running)
        recordProcess->terminate();
    saveSettings();
    close();
}

void MainWindow::recordButtonClicked()
{
    if(recordProcess->state() == QProcess::Running )
    {
        recordProcess->write("q");
        return;
    }

    QStringList argumentList;
    QString program = ui->ffmpegEdit->text();
    QString nrs = QString("%1%2").arg(QString::number(ui->nrBox->value(),10),2,'0').arg(ui->outputEdit->text());
    QString output =  nrs + "." + ui->videoFormatBox->currentText();
    QString audioDevice = ui->audioDeviceBox->currentText();
    QString videoDevice = ui->videoDeviceBox->currentText();
    QString crf = QString::number(ui->crfBox->value(),10);
    QString fps = QString::number(ui->fpsBox->value(),10);
    argumentList << "-rtbufsize" << "1500M"
                 << "-f" << "dshow"
                 << "-i" << "audio=" + audioDevice
                 << "-f" << "-y" << "-rtbufsize" << "100M"
                 << "-f" << "gdigrab"
                 << "-t" << ui->timeEdit->time().toString("hh:mm:ss") // "00:10:00" default
                 << "-framerate" << fps
                 << "-probesize" << "10M"
                 << "-draw_mouse" << "1"
                 << "-i" << videoDevice // "desktop" default
                 << "-c:v" << "libx264"
                 << "-r" << fps << "-preset" << "ultrafast"
                 << "-tune" << "zerolatency"
                 << "-crf" << crf << "-pix_fmt" << "yuv420p"
                 << output;

    recordProcess->setProcessChannelMode(QProcess::MergedChannels);
    recordProcess->start(program, argumentList);

}

void MainWindow::mergeButtonClicked()
{
    // Get available videos
    QDir dir = QDir::current();
    dir.setFilter(QDir::Files);

    QStringList filters;
    filters << "*." + ui->videoFormatBox->currentText();
    dir.setNameFilters(filters);
    dir.setSorting(QDir::Time);
    QStringList list =  dir.entryList();


    videolist = new DialogVideoList(list, this);
    if(videolist->exec() == QDialog::Rejected)
        return;

    QStringList nameList = videolist->request();
    QString videoName = videolist->getVideoName();

    QFile file("video.txt");
    if(file.exists())
       if(!file.remove())
           return;

    if(!file.open(QFileDevice::WriteOnly))
        return;

    QTextStream out(&file);
    foreach (QString s, nameList)
    {
       out << "file" << " " << "'"<< s << "'" << "\n";
    }

    file.close();

    merging = true;
    QStringList argumentList;
    QString program = ui->ffmpegEdit->text();
    QString output = videoName + "." + ui->videoFormatBox->currentText();
    argumentList << "-f" << "concat"
                 << "-i" << "video.txt"
                 << "-c" << "copy"
                 << output;

    recordProcess->setProcessChannelMode(QProcess::MergedChannels);
    recordProcess->start(program, argumentList);

}

void MainWindow::infoButtonClicked()
{
    DialogInfo *infoDlg = new DialogInfo(this);
    infoDlg->exec();
}

void MainWindow::recordStarted()
{
    ui->statusBar->showMessage("recording...");
    ui->recordButton->setText("&stop");
    ui->recordButton->setShortcut(QKeySequence(tr("Ctrl+s")));
    ui->recordButton->setToolTip("Stop recording [Ctrl+s]" );

    ui->mergeButton->setEnabled(false);

    if(!merging)
    {
        int msec = QTime(0,0,0).msecsTo( ui->timeEdit->time() );
        recordTimer->setInterval( msec );
        recordTimer->start();
        connect(recordTimer, &QTimer::timeout, this, &MainWindow::timout );
    }
}

void MainWindow::readyReadStandardOutput()
{
    ui->processOutputText->append( recordProcess->readAllStandardOutput() );
}

void MainWindow::readyReadStandardError()
{
   ui->statusBar->showMessage( recordProcess->errorString() );
}

void MainWindow::recordFinished(int /*exitCode*/, QProcess::ExitStatus status)
{
    if(status == 0)
    {
        ui->statusBar->showMessage( "Process normaly exit" );
        ui->recordButton->setText("&record");
        ui->recordButton->setShortcut(QKeySequence(tr("Ctrl+r")));
        ui->recordButton->setToolTip("Start to record [Ctrl+r]" );
        ui->mergeButton->setEnabled(true);
    }else
        ui->statusBar->showMessage( "Process exit by crashed" );

    merging = false;
}

void MainWindow::processStateChanged(QProcess::ProcessState /*state*/)
{
    if(recordProcess->state() == QProcess::NotRunning)
        ui->statusBar->showMessage( "process is not running." );

    if(recordProcess->state() == QProcess::Starting)
        ui->statusBar->showMessage( "process is starting." );

    if(recordProcess->state() == QProcess::Running)
        ui->statusBar->showMessage( "process is running." );
}

void MainWindow::timout()
{
    if(recordProcess->state() == QProcess::Running )
    {
        recordTimer->stop();
        recordProcess->write("q");
        disconnect(recordTimer, &QTimer::timeout, this, &MainWindow::timout );
    }
}

void MainWindow::dirButtonClicked()
{
    QString currentPath = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                          "/home", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);


    ui->pathRecordEdit->setText( currentPath );
    QDir::setCurrent( currentPath );
}

void MainWindow::ffmpegDirButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                    "/home", "*.*");
    ui->ffmpegEdit->setText( fileName );
}

void MainWindow::readSettings()
{
    QSettings settings("FasaSoft", "FScreenRecorder");

    QString pathFFMPEG = settings.value("pathffmpeg").toString();
    ui->ffmpegEdit->setText( pathFFMPEG );

    QString pathRecord = settings.value("pathrecord", QDir::currentPath()).toString();
    ui->pathRecordEdit->setText( pathRecord );
    QDir::setCurrent(pathRecord);
}

void MainWindow::saveSettings()
{
    QSettings settings("FasaSoft", "FScreenRecorder");
    settings.setValue("pathffmpeg", ui->ffmpegEdit->text());
    settings.setValue("pathrecord", ui->pathRecordEdit->text());
}


