#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtMath>
#include <QDir>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("FScreenRecord");
    nr = 1;
    merging = false;
    readSettings();

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

    // Duration time 10 min
    ui->timeEdit->setTime(QTime(0,10,0));

    //audio devices
    int index = 0;
    foreach (QAudioDeviceInfo device, QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
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
    QString program = ui->ffmpegEdit->text();    //"d:/ffmpeg/bin/ffmpeg.exe";
    QString nrs = QString("%1%2").arg(QString::number(nr,10),2,'0').arg(ui->outputEdit->text());
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
    QDir dir = QDir::current();
    dir.setFilter(QDir::Files);

    QStringList filters;
    filters << "*." + ui->videoFormatBox->currentText();
    dir.setNameFilters(filters);

    QFile file("video.txt");
    if(!file.open(QFileDevice::WriteOnly))
        return;

    QTextStream out(&file);
    QStringList list =  dir.entryList();
    foreach (QString s, list)
    {
       out << "file" << " " << "'"<< s << "'" << "\n";
    }

    file.close();

    merging = true;
    QStringList argumentList;
    QString program = ui->ffmpegEdit->text();
    QString output =  QString("output") + "." + ui->videoFormatBox->currentText();
    argumentList << "-f" << "concat"
                 << "-i" << "video.txt"
                 << "-c" << "copy"
                 << output;

    recordProcess->setProcessChannelMode(QProcess::MergedChannels);
    recordProcess->start(program, argumentList);


}

void MainWindow::recordStarted()
{
    ui->statusBar->showMessage("recording...");
    ui->recordButton->setText("&stop");
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

    QString data = ui->processOutputText->toPlainText();
    qDebug() << data;

}

void MainWindow::readyReadStandardError()
{
   ui->statusBar->showMessage( recordProcess->errorString() );
}

void MainWindow::recordFinished(int /*exitCode*/, QProcess::ExitStatus status)
{
    if(status == 0){
        ui->statusBar->showMessage( "Process normaly exit" );
        ui->recordButton->setText("&record");
        nr++;
        if(nr > 2)
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
}

void MainWindow::saveSettings()
{
    QSettings settings("FasaSoft", "FScreenRecorder");
    settings.setValue("pathffmpeg", ui->ffmpegEdit->text());
    settings.setValue("pathrecord", ui->pathRecordEdit->text());
}


