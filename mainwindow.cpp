#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtMath>
#include <QDir>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QScreen>
#include <QString>

#include <dialoginfo.h>
#include <QDebug>



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("FScreenRecord");
    setWindowIcon(QIcon(":/FScreenRecorder.ico"));

    // Test FormMessageWidget for another style
    // and thread
    testWidget = new FormMessageWidget();

    setPlatformInfo();
    readSettings();

    durationSeconds = 0;
    elapsedSeconds = 0;
    merging = false;

    ui->timelineSlider->setRange(0,140);
    ui->timelineSlider->setColor( Qt::magenta );
    ui->timelineSlider->setSelectedColor( Qt::yellow );
    ui->timelineSlider->setTextColor(Qt::blue);
    ui->timelineSlider->update();
    ui->frameDeviceRectangle->hide();



    // Timer for record duration
    recordTimer = new QTimer(this);

    // Process for ffmpeg
    recordProcess = new QProcess(this);
    recordProcess->setProcessChannelMode(QProcess::MergedChannels);

    // connect process signals
    connect(recordProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::readyReadStandardOutput);
    connect(recordProcess, &QProcess::stateChanged , this, &MainWindow::recordProcessStateChanged);
    connect(recordProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
         [=](int exitCode, QProcess::ExitStatus exitStatus){ processFinished(exitCode, exitStatus); });


    // info process is the merge tool process
    infoProcess = new QProcess(this);
    infoProcess->setProcessChannelMode(QProcess::MergedChannels);
    connect(infoProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::readyReadInfoStandardOutput);

    // cut process is the cut process
    cutProcess = new QProcess(this);
    cutProcess->setProcessChannelMode(QProcess::MergedChannels);
    connect(cutProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
         [=](int exitCode, QProcess::ExitStatus exitStatus){ cutProcessFinished(exitCode, exitStatus); });

    // connect application signals
    connect(ui->closeButton, &QPushButton::clicked, this, &MainWindow::closeButtonClicked);
    connect(ui->recordButton, &QPushButton::clicked, this, &MainWindow::recordButtonClicked);
    connect(ui->mergeButton, &QPushButton::clicked, this, &MainWindow::mergeButtonClicked);
    connect(ui->infoButton, &QPushButton::clicked, this, &MainWindow::infoButtonClicked);    

    connect(ui->outputPathButton, &QPushButton::clicked, this, &MainWindow::outputPathButtonClicked);
    connect(ui->ffmpegPathButton, &QPushButton::clicked, this, &MainWindow::ffmpegPathButtonClicked);

    // connect record timer
    connect(recordTimer, &QTimer::timeout, this, &MainWindow::timeout );

    // for the tools panel
    connect(ui->tabWidget, &QTabWidget::currentChanged , this, &MainWindow::tabIndexChanged );
    connect(ui->videoListWidget, &QListWidget::itemClicked, this, &MainWindow::itemClicked );
    connect(ui->cutButton, &QPushButton::clicked, this, &MainWindow::cutButtonClicked);
    connect(ui->timelineSlider, &SliderTwoHandle::leftValueChanged, this, &MainWindow::handleSliderLeftValueChanged);
    connect(ui->timelineSlider, &SliderTwoHandle::rightValueChanged, this, &MainWindow::handleSliderRightValueChanged);

    // for the device panel
    connect(ui->videoDeviceBox, &QComboBox::currentTextChanged, this, &MainWindow::deviceSelectionChanged);
    connect(ui->heightSlider, &QSlider::valueChanged, this, &MainWindow::sliderValueChanged);
    connect(ui->widthSlider, &QSlider::valueChanged, this, &MainWindow::sliderValueChanged);
    connect(ui->xPosSlider , &QSlider::valueChanged, this, &MainWindow::sliderValueChanged);
    connect(ui->yPosSlider , &QSlider::valueChanged, this, &MainWindow::sliderValueChanged);

    installEventFilter(this);

#ifdef Q_OS_WIN32
    taskbarButton = new QWinTaskbarButton(this);
    taskbarButton->setOverlayIcon(QIcon(":/start.ico"));
    taskbarProgress = taskbarButton->progress();
    taskbarProgress->setVisible(true);
#endif

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::closeButtonClicked()
{

    if(recordProcess->state() == QProcess::Running){
        QMessageBox::information(this, tr("Process"), tr("Process is still running.\n"
        "Please, stop recording before closing this application!"));
        return;
    }

    if(recordProcess->state() == QProcess::Running)
        recordProcess->kill();

    if(recordTimer != nullptr)
        recordTimer->deleteLater();

    // Test testWidget for style
    if(testWidget != nullptr){
        if(!testWidget->isHidden())
            testWidget->close();
    }

    saveSettings();
    close();
}

/// !brief Start recording
void MainWindow::recordButtonClicked()
{
    if(recordProcess->state() == QProcess::Running){
        recordProcess->write("q\n");
        return;
    }

    if(!checkPath())
        return;

    // Clear output text
    ui->processOutputText->clear();

    // Hide text inside the rectangle
    testWidget->setShowText(false);


    // Define the programm with path (ffprobe is inside in ffmpeg til 2022)
    QString programm = ui->ffmpegPathEdit->text();
    QStringList argumentList;

    // videoname = nr + videoname
//    QString nrs = QString("%1%2").arg(QString::number(ui->nrBox->value(),10),2,'0').arg(ui->outputEdit->text());
//    QString output =  nrs + "." + ui->videoFormatBox->currentText();
    QString output = getVideoName();

    QString videoDevice;
    QString audioDevice;
    QString videoGrab;
    QString videoSize;

    // check screen selected
    QString screenName = ui->videoDeviceBox->currentText();
    if(platform == "linux"){

        QScreen *selectedScreen = QGuiApplication::primaryScreen();
        QList<QScreen *> screenList = QGuiApplication::screens();
        foreach(QScreen *s, screenList){
            if(s->name() == screenName)
                selectedScreen = s;
        }


        if(selectedScreen->availableGeometry().x() == 0){  // Screen is on left side
            videoDevice = ":0.0";
            videoSize = QString::number( selectedScreen->size().width(), 10) + "x" + QString::number( selectedScreen->size().height(),10);
        }
        else {

            videoDevice = ":0.0+" + QString::number(selectedScreen->availableGeometry().x(),10) + ",0";
            videoSize = QString::number( selectedScreen->size().width(), 10) + "x" + QString::number( selectedScreen->size().height(),10);
        }
    }

    if(platform == "windows"){
        videoDevice = "desktop";                            // only desktop available yet
        videoGrab = "gdigrab";
        audioDevice = ui->audioDeviceBox->currentText();    // input audio device
    }

    QString crf = QString::number(ui->crfBox->value(),10);      // quality of video
    QString fps = QString::number(ui->fpsBox->value(),10);      // framerate of video
    QString seconds =  QString::number( QTime(0,0,0).secsTo(ui->timeEdit->time()),10); // record duration in sec
    durationSeconds = QTime(0,0,0).secsTo(ui->timeEdit->time());

    // Try to use rectangle for screen record
    //<< "-offset_x" <<"200" << "-offset_y" << "200" << "-video_size" << "vga" // is working as vga size
    //<< "-offset_x" <<"200" << "-offset_y" << "200" << "-video_size" << "1440:900" // is working as custom size
    //<< "-show_region" << "1"// show region not working

    if(platform == "windows"){

        if(ui->videoDeviceBox->currentText() == "rectangle"){

            argumentList << "-rtbufsize" << "1500M"
                         << "-f" << "dshow"
                         << "-i" << "audio=" + audioDevice
                         << "-f" << "-y" << "-rtbufsize" << "100M"
                         << "-f" << videoGrab
                         << "-t" << seconds
                         << "-framerate" << fps
                         << "-probesize" << "10M"
                         << "-draw_mouse" << "1"
                         << "-offset_x" << QString("%1").arg(ui->xPosSlider->value(),10)
                         << "-offset_y" <<QString("%1").arg(ui->yPosSlider->value(),10)
                         << "-video_size" <<  QString("%1").arg(ui->widthSlider->value(),10) + ":" +  QString("%1").arg(ui->heightSlider->value(),10)   // e. g. "960:680"
                         << "-i" << videoDevice // "desktop" default
                         << "-c:v" << "libx264"
                         << "-r" << fps << "-preset" << "ultrafast"
                         << "-tune" << "zerolatency"
                         << "-crf" << crf << "-pix_fmt" << "yuv420p"
                         << output;
        }
        else{
            argumentList << "-rtbufsize" << "1500M"
                         << "-f" << "dshow"
                         << "-i" << "audio=" + audioDevice
                         << "-f" << "-y" << "-rtbufsize" << "100M"
                         << "-f" << videoGrab
                         << "-t" << seconds
                         << "-framerate" << fps
                         << "-probesize" << "10M"
                         << "-draw_mouse" << "1"
                         << "-i" << videoDevice // "desktop" default
                         << "-c:v" << "libx264"
                         << "-r" << fps << "-preset" << "ultrafast"
                         << "-tune" << "zerolatency"
                         << "-crf" << crf << "-pix_fmt" << "yuv420p"
                         << output;
        }

    }

    if(platform == "linux"){


        videoGrab = "x11grab";

        argumentList  << "-video_size" << videoSize  // e.g "1440x900"
                      << "-framerate" << fps
                      << "-f" << videoGrab
                      << "-i" << videoDevice        // ":0.0+1024,0"
                      << "-f" << "alsa"             // better performance while recording frame drops = 3 by 2 Minutes : pulse = whorse
                      << "-ac" << "2"
                      << "-i" << "default"
                      << "-crf" << crf << "-pix_fmt" << "yuv420p"
                      << "-preset" << "ultrafast"
                      << "-c:v" << "libx264"
                      << output;
    }


    recordProcess->start(programm, argumentList);


}

void MainWindow::infoButtonClicked()
{
    DialogInfo *infoDlg = new DialogInfo(this);
    connect(infoDlg, &DialogInfo::checkForUpdate, this, &MainWindow::checkUpdate);
    infoDlg->exec();
}

void MainWindow::mergeButtonClicked()
{

    // Check if still recording
    if(recordProcess->state() == QProcess::Running){
        QMessageBox::information(this, tr("Merge videos"), tr("You have to stop recording, before you can merge videos!"));
        return;
    }

    // Get available videos
    QStringList filters;
    filters << "*." + ui->videoFormatBox->currentText();

    QStringList list =  availableVideos(QDir::current(), filters);

    DialogVideoList *videoDlg = new DialogVideoList(list, this);
    if(videoDlg->exec() == QDialog::Rejected)
        return;

    QStringList videoList = videoDlg->request();
    QString videoName = videoDlg->getVideoName();

    foreach(QString sn, videoList){
        if(sn.contains(' ')){
            QMessageBox::information(this, tr("Merge Videos"), tr("Please remove all spaces from filename,\n"
                                                                  "before merging videos!"));
            return;
        }
    }


    QFile file("video.txt");
    if(file.exists())
       if(!file.remove())
           return;

    if(!file.open(QFileDevice::WriteOnly))
        return;

    QTextStream out(&file);
    foreach (QString s, videoList)
    {
       out << "file" << " " << "'"<< s << "'" << "\n";
    }

    file.close();

    merging = true;
    QStringList argumentList;
    QString program = ui->ffmpegPathEdit->text();
    QString output = videoName + "." + ui->videoFormatBox->currentText();
    argumentList << "-f" << "concat"
                 << "-i" << "video.txt"
                 << "-c" << "copy"
                 << output;

    recordProcess->setProcessChannelMode(QProcess::MergedChannels);
    recordProcess->start(program, argumentList);

}

void MainWindow::cutButtonClicked()
{
    QStringList argumentList;
    QString program = ui->ffmpegPathEdit->text();
    QString output = getText(ui->selectedVideoEdit->text(), "", '.') + "CutVersion" + "." + ui->videoFormatBox->currentText();
    argumentList << "-i" << ui->selectedVideoEdit->text()
                 << "-ss" << QString::number(ui->startSecBox->value(),10)
                 << "-t" << QString::number(ui->endSecBox->value(),10)
                 << "-c" << "copy"
                 << output;

    cutProcess->setProcessChannelMode(QProcess::MergedChannels);
    cutProcess->start(program, argumentList);

    ui->cutButton->setEnabled(false);

}

/// !brief Get ffmpeg installed path including exe/bin
void MainWindow::ffmpegPathButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                    "/home", "*.*");
    ui->ffmpegPathEdit->setText( fileName );
}

/// !brief Get output path
void MainWindow::outputPathButtonClicked()
{
    QString outputpath = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                          "/home", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);


    ui->outputPathEdit->setText( outputpath );

    // Set application working directorie
    QDir::setCurrent( outputpath );
}


void MainWindow::deviceSelectionChanged(const QString &text)
{
    if(text == "rectangle"){
        ui->frameDeviceRectangle->show();
        testWidget->show();
    }
    else {
        ui->frameDeviceRectangle->hide();
        testWidget->hide();
    }
}

void MainWindow::sliderValueChanged(int /*value*/)
{
    ui->valueX->setText( QString("%1").arg(ui->xPosSlider->value(),10 ) );
    ui->valueY->setText( QString("%1").arg(ui->yPosSlider->value(),10 ) );
    ui->valueWidth->setText( QString("%1").arg(ui->widthSlider->value(),10 ) );
    ui->valueHeight->setText( QString("%1").arg(ui->heightSlider->value(),10 ) );


    testWidget->setPos(ui->xPosSlider->value(), ui->yPosSlider->value());
    testWidget->setGeometry(ui->xPosSlider->value() ,  ui->yPosSlider->value() ,  ui->widthSlider->value(), ui->heightSlider->value());
    testWidget->setRect( QRect(0 , 0 ,  ui->widthSlider->value(), ui->heightSlider->value()) );

}

/// !brief Update videos when tab tools was selected
void MainWindow::tabIndexChanged(int index)
{
    if(index == 3) // Tab tools was selected
        updateVideoListWidget();
}

/// !brief When click on videoListWidget item
/// in the tools panel
void MainWindow::itemClicked(QListWidgetItem *item)
{
    if(item->checkState() == Qt::Unchecked)
        return;

    for(int i = 0; i < ui->videoListWidget->count(); i++){
        if(item->checkState() == Qt::Checked){
            if(ui->videoListWidget->item(i)->text() != item->text())
                ui->videoListWidget->item(i)->setCheckState(Qt::Unchecked);

        }
    }

    QString videofile = QDir::currentPath()+"/"+item->text();
    ui->selectedVideoEdit->setText( videofile );

    QString programm = ui->ffmpegPathEdit->text();

// Out of date no ffprobe anymore
//#ifdef Q_OS_WIN32
//    programm.replace("ffmpeg.exe", "ffprobe.exe");
//#endif

#ifdef Q_OS_LINUX
    programm.replace("ffmpeg", "ffprobe");
#endif

    // mkv -> width, height, r_frame_rate, start_time, TAG:DURATION
    // mp4 -> width, height, r_frame_rate, start_time, duration

    QStringList argumentList;
    argumentList //<< "-v" << "error"
                 //<< "-show_format"
                 <<"-i"
                 << videofile
                 << "-f" << "null-";

    infoProcess->start(programm, argumentList);

}


/// !brief Slot form recordTimer
/// reached every seconds
void MainWindow::timeout()
{
    elapsedSeconds++;
    if( elapsedSeconds > durationSeconds && !merging){
        if(recordProcess->state() == QProcess::Running)
            recordProcess->write("q\n");
    }

    // Test widget is working
//    QString counter = QString::number(elapsedSeconds);
//    testWidget->setLabelText(counter);



#ifdef Q_OS_WIN32
    taskbarProgress->setValue(elapsedSeconds);
#endif

}

/// !brief Get the standard output information
/// specialy during recording
void MainWindow::readyReadStandardOutput()
{


//    if(!recordProcess->errorString().isEmpty()){
//        QMessageBox::information(this, tr("Process Error"), tr("Recording Error:\n") +
//                                QString::number(recordProcess->error(),10)+ ":" + recordProcess->errorString());
//        recordProcess->write("q\n");
//        return;

//    }

    QByteArray outputArray = recordProcess->readAllStandardOutput();
    QList<QByteArray> outputList = outputArray.split('\n');
    foreach (QString s, outputList) {

        // Error has detected
        if(s.contains("Error") || s.contains("error")){
            ui->processOutputText->append( s );

            if(s.contains("width") || s.contains("height")){
                ui->processOutputText->append("\nTry to reduce the height or width from rectangle!");
                s.append("\nTry to reduce the height or width from rectangle!");
            }
            setWindowState(Qt::WindowActive);
            QMessageBox::warning(this, tr("Error"), tr("FFMPEG failed because:\n")+s);
            return;
        }

        // When recording
        if(s.contains("time=") || s.contains("fps=")){
            ui->processOutputText->append( s );
            QString textTime = getText(s, "time=", ' ');
            QString elapsed = "[" + QString::number(elapsedSeconds,10) + "]";
            ui->statusBar->showMessage("recording: "+textTime+":"+elapsed);
        }

        if(s.contains("Overwrite?") || s.contains("[y/N]")){

            if(recordTimer->isActive()){
                recordTimer->stop();
#ifdef Q_OS_WIN32
                taskbarProgress->setPaused(true);
#endif
            }

            // When Overwrite occure the task bar process appear in
            // yellow color by default, make sure the application
            // popup.
            setWindowState(Qt::WindowActive);

            int result = QMessageBox::question(this, tr("Question"), s,
                         QMessageBox::Yes, QMessageBox::No);

            if(result == QMessageBox::Yes){
                recordProcess->write("y\n");
                recordTimer->start(1000);
#ifdef Q_OS_WIN32
                taskbarProgress->setPaused(false);
#endif
            }

            // User abort recording
            if(result == QMessageBox::No){
                recordProcess->write("N\n");
#ifdef Q_OS_WIN32
                taskbarProgress->stop();
#endif
            }
        }
    }
}

/// !brief Get the state of record process
void MainWindow::recordProcessStateChanged(QProcess::ProcessState status)
{
    if(status == QProcess::Running){
        ui->statusBar->showMessage("Process is running!");
        ui->recordButton->setText("stop");
        ui->recordButton->setToolTip("Click for stop recording.\nShortcut [Ctrl+S]");
        ui->recordButton->setShortcut(QKeySequence("Ctrl+S"));
        ui->recordButton->setIcon(QIcon(":/record.ico"));

#ifdef Q_OS_WIN32
        taskbarButton->setOverlayIcon(QIcon(":/record.ico"));
#endif

    }

    if(status == QProcess::Starting && merging == false){

        elapsedSeconds = 0;
        recordTimer->start(1000);

#ifdef Q_OS_WIN32
        taskbarProgress->setRange(elapsedSeconds, durationSeconds);
#endif

        // Minimize window
        if(ui->minimizedBox->isChecked() && windowState() != Qt::WindowMinimized && !merging)
            setWindowState(Qt::WindowMinimized);
    }

    if(status == QProcess::NotRunning){
        ui->statusBar->showMessage("Process not running!");
        ui->recordButton->setText("record");
        ui->recordButton->setToolTip("Click for starting recording.\nShortcut [Ctrl+R]");
        ui->recordButton->setShortcut(QKeySequence("Ctrl+R"));
        ui->recordButton->setIcon(QIcon(":/start.ico"));

#ifdef Q_OS_WIN32
        taskbarButton->setOverlayIcon(QIcon(":/start.ico"));
#endif
    }
}

void MainWindow::processFinished(int exitCode, QProcess::ExitStatus status)
{
    if(exitCode == 0 && status == QProcess::NormalExit){
        if(merging)
            ui->statusBar->showMessage("Merging videos successful finished!", 5000);
        else
            ui->statusBar->showMessage("Process successful finished!", 5000);

        if(recordTimer->isActive())
            recordTimer->stop();

#ifdef Q_OS_WIN32
        taskbarButton->setOverlayIcon(QIcon(":/start.ico"));
        taskbarProgress->reset();
#endif


        if(windowState() == Qt::WindowMinimized)
            setWindowState(Qt::WindowActive);

    }

    if(exitCode == 1 && status == QProcess::CrashExit){
        QMessageBox::warning(this, tr("Error"), tr("Process error occured:\n")+recordProcess->errorString());
        qDebug() << "Crashed wile recording:" << recordProcess->errorString();
        return;
    }

    merging = false;
}

void MainWindow::cutProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    if(exitCode == 0 && status == QProcess::NormalExit){
        ui->statusBar->showMessage("Cutting process successful finished!");
    }

    if(exitCode == 1 && status == QProcess::CrashExit){
        QMessageBox::warning(this, tr("Error"), tr("Process error occured:\n")+cutProcess->errorString());
        return;
    }
}

/// !brief When selecting video in the tool panel
/// check for start and duration but video and audio
/// is not specify yet
void MainWindow::readyReadInfoStandardOutput()
{ 
    QByteArray array = infoProcess->readAllStandardOutput();
    QString text(array);
    QStringList list = text.split("\n");
    foreach (QString s, list) {

        if(s.contains("start")){
           double min = getText( s, "start", '\r').toDouble();
           ui->startSecBox->setValue( static_cast<int>(min) );
           ui->timelineSlider->setMinimum( min );
        }

        if(s.contains("DURATION")){
            double max =  getDuration(s);
            ui->endSecBox->setValue( static_cast<int>(max) );
            ui->timelineSlider->setMaximum( max );
        }
    }

    ui->timelineSlider->update();
    ui->cutButton->setEnabled(true);
}

void MainWindow::checkUpdate()
{
    QMessageBox::information(this, tr("Update"), tr("Not implemented yet!"));
}

void MainWindow::setPlatformInfo()
{
    platform = "unknown";

#ifdef Q_OS_WIN32
    platform = "windows";
#endif
#ifdef Q_OS_LINUX
    platform = "linux";
#endif
}

void MainWindow::handleSliderLeftValueChanged(int value)
{
    ui->startSecBox->setValue(value);
}

void MainWindow::handleSliderRightValueChanged(int value)
{
    ui->endSecBox->setValue(value);
}

void MainWindow::updateVideoListWidget()
{
    QStringList vList = availableVideos(QDir::current(), QStringList() << "*.mkv" << "*.mp4");

    ui->videoListWidget->clear();
    foreach (QString s, vList) {
        QListWidgetItem *item = new QListWidgetItem(s);
        item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
        item->setCheckState(Qt::Unchecked);
        ui->videoListWidget->addItem(item);
    }
}

///!brief Get the current videoname including nr of video
QString MainWindow::getVideoName()
{
    QString nrs = QString("%1%2").arg(QString::number(ui->nrBox->value(),10),2,'0').arg(ui->outputEdit->text());
    QString output =  nrs + "." + ui->videoFormatBox->currentText();
    return output;
}

QStringList MainWindow::availableVideos(QDir dir, const QStringList &filters)
{
    dir.setFilter(QDir::Files);
    dir.setNameFilters(filters);
    dir.setSorting(QDir::Time | QDir::Reversed );

    return  dir.entryList();

}

QString MainWindow::getText(const QString &sourceText, const QString &fromText, const QChar &tilChar)
{
    QString text;


    if(!fromText.isEmpty()){
        if(!sourceText.contains(fromText))
            return text;
    }

    if(!sourceText.contains(tilChar))
        return text;

    int index;

    if(!fromText.isEmpty())
        index = sourceText.indexOf(fromText);
    else
        index = 0;

    index += fromText.size();

    for(int i = index; i < sourceText.size(); i++){
        if(sourceText.at(i) == tilChar)
            break;
        text.append( sourceText.at(i) );
    }

    return text;
}

/// !brief Returns the duration in seconds
double MainWindow::getDuration(const QString &s)
{
    double value = 0.0;
    QStringList slist = s.split(":");
    value += slist.last().toDouble();
    return value;
}

/// !brief Check output path and path of ffmpeg
bool MainWindow::checkPath()
{
    if(ui->outputPathEdit->text().isEmpty()){
        QMessageBox::information(this, tr("Output path"), tr("Please, set the output path, in settings tab!"));
        return false;
    }

    QFile file(ui->ffmpegPathEdit->text());
    if(!file.exists()){
        QMessageBox::information(this, tr("FFMPEG path"), tr("Please, set the ffmpeg path, in settings tab!"));
        return false;
    }


    return true;
}

/// !brief Get events when application minimized or popup back from taskbar
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::Type(QEvent::WindowStateChange)){

        // Window popup from taskbar
        if(windowState() == Qt::WindowNoState){

            // When set to stop recording when click on taskbar
            if(ui->stopRecordingBox->isChecked() && recordTimer->isActive())
                recordProcess->write("q\n"); // quit recording
        }
    }
    else{
        // pass the event on to the parent class
        return QMainWindow::eventFilter(obj, event);
    }

    return false;
}

/// !brief Get valid window handle
void MainWindow::showEvent(QShowEvent *event)
{
#ifdef Q_OS_WIN32
    taskbarButton->setWindow( windowHandle() );
#endif

    event->accept();
}



void MainWindow::readSettings()
{
    QSettings settings("FasaSoft", "FScreenRecorder");

    ui->ffmpegPathEdit->setText( settings.value("pathffmpeg").toString() );
    ui->minimizedBox->setChecked(settings.value("minimizedWindow",false).toBool());
    ui->stopRecordingBox->setChecked(settings.value("stoprecording",false).toBool());
    ui->timeEdit->setTime( settings.value("recordtime", QTime(0,10,0) ).toTime());

    QString pathRecord = settings.value("pathrecord", QDir::currentPath()).toString();
    ui->outputPathEdit->setText( pathRecord );
    QDir::setCurrent(pathRecord);


    // Video attributes
    ui->videoFormatBox->setCurrentText(  settings.value("videoformat", "mkv").toString()  );
    ui->fpsBox->setValue( settings.value("videoframerate", 25).toInt() );
    ui->crfBox->setValue( settings.value("videoquality", 20).toInt() );


    // Default settings for audio and video devices
    // Video

    QList<QScreen *> screenList = QGuiApplication::screens();
    QStringList screeNames;
    foreach(QScreen *s, screenList)
        screeNames << s->name();

    ui->videoDeviceBox->addItems(QStringList() << "desktop" <<  screeNames << "rectangle" );

    // Test read first screen geometry
    int w = screenList.first()->availableGeometry().width();
    int h = screenList.first()->availableGeometry().height();

    ui->widthSlider->setRange(0, w);
    ui->heightSlider->setRange(0,h);
    ui->xPosSlider->setRange(0,w);
    ui->yPosSlider->setRange(0,h);

    // Audio
    int index = 0;
    foreach (QAudioDeviceInfo device, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
    {
          ui->audioDeviceBox->insertItem(index, device.deviceName());
          index++;
    }

    // Add the default audio input device input
    ui->audioDeviceBox->insertItem(ui->audioDeviceBox->count(), QAudioDeviceInfo::defaultInputDevice().deviceName());


    // Default settings for video format
    ui->videoFormatBox->addItems(QStringList() << "mkv" << "mp4");

    ui->tabWidget->setCurrentIndex(0);
    this->setGeometry( settings.value("geometrie", QRectF(0,0,400,500)).toRect() );


    // Set the last videoname and nr
    bool ok;
    int nr = settings.value("nrvideo", 0).toInt(&ok);
    ui->nrBox->setValue(nr);

    QString vname = settings.value("videoname", "").toString();
    ui->outputEdit->setText(vname);



    if(testWidget != nullptr){

        int x = settings.value("xpos",0).toInt(&ok);  Q_ASSERT(ok);
        int y = settings.value("ypos",0).toInt(&ok);  Q_ASSERT(ok);
        int w = settings.value("width",800).toInt(&ok);  Q_ASSERT(ok);
        int h = settings.value("height",680).toInt(&ok);  Q_ASSERT(ok);


        testWidget->setGeometry(0,0,w,h);
        testWidget->setRect( QRect( 0, 0, w, h ));
        testWidget->setPos( x, y );

        ui->xPosSlider->setValue(x);
        ui->yPosSlider->setValue(y);
        ui->widthSlider->setValue(w);
        ui->heightSlider->setValue(h);

        ui->valueX->setText( QString("%1").arg(x,10));
        ui->valueY->setText( QString("%1").arg(y,10));
        ui->valueWidth->setText( QString("%1").arg(w,10));
        ui->valueHeight->setText( QString("%1").arg(h,10));
    }

}

void MainWindow::saveSettings()
{
    QSettings settings("FasaSoft", "FScreenRecorder");
    settings.setValue("pathffmpeg", ui->ffmpegPathEdit->text());
    settings.setValue("pathrecord", ui->outputPathEdit->text());
    settings.setValue("minimizedWindow", ui->minimizedBox->isChecked());
    settings.setValue("stoprecording", ui->stopRecordingBox->isChecked());
    settings.setValue("recordtime", ui->timeEdit->time());
    settings.setValue("videoformat", ui->videoFormatBox->currentText());
    settings.setValue("videoframerate", ui->fpsBox->value());
    settings.setValue("videoquality", ui->crfBox->value());
    settings.setValue("geometrie", this->geometry());
    settings.setValue("nrvideo", ui->nrBox->value());
    settings.setValue("videoname", ui->outputEdit->text() );

    if(testWidget != nullptr){
        settings.setValue("xpos", testWidget->geometry().x() );
        settings.setValue("ypos", testWidget->geometry().y() );
        settings.setValue("width", testWidget->geometry().width() );
        settings.setValue("height", testWidget->geometry().height() );
    }

}


