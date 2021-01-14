// Copyright (C) 2020-2022 Eyecloud Corporation
// SPDX-License-Identifier: Apache-2.0mv
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cliSdk.h"
#include <QDebug>
#include "videodecodethread.h"
#include "playwidget.h"
#include <QSqlDatabase>
#include <QMessageBox>
#include <QSqlQuery>
#include <QFileDialog>
#include "choosepkg.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/highgui/highgui_c.h>
#include "upgradewidget.h"
#include "aiutil.h"
#include "ipclog.h"

const QString kVersion("IV1.0.0");

const QString kAiModelRootPath("Configuration/blob");
const QString kAiModelDbPath("/Configuration/NCC.db");

const QString kNoneModelName("None");
const QString kNoneDeviceVersion("None");

QImage cvMat2QImage(const cv::Mat& mat)  {
    const uchar *pSrc = (const uchar*)mat.data;
    QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
    return image.rgbSwapped();
}


static QSqlDatabase loadDb(const QString& dbName){
    QString exePath = QCoreApplication::applicationDirPath();

    QString dbPath = exePath + dbName;

#ifdef Q_OS_WIN
    // 默认使用linux风格，当系统为win时，转成win风格的路径
    dbPath = QDir::toNativeSeparators(dbPath);
#endif

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);
    return db;
  }

void MainWindow::loadModelFromDb(){
    QSqlDatabase db = loadDb(kAiModelDbPath);
    if (!db.open()){
        QMessageBox::warning(this,
                             "Loading database error",
                             "Failed to load default sqlite database!\n\n"
                                                           "You need reinstall your NCC View or add a modle first.");
    }else {
        QSqlQuery query;
        QString modelName;
        query.exec("SELECT * FROM ncc_model where parentId = 1 or parentId = 3");
        while (query.next()){
            modelName = query.value("model_name").toString();
            dbModelNames_.insert(modelName);
            ui->aiModelComboBox_->addItem(modelName);
        }
        ui->aiModelComboBox_->addItem(kNoneModelName);
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->versionLabel_->setText(kVersion);

    qRegisterMetaType<cv::Mat>("cv::Mat");

    connect(&(IpcLog::IpcLogger()),
            &IpcLog::sendLogToText,
            this,
            &MainWindow::sendLogToText);

    std::map<QString,int> bpsInfo = PlayParamasManager::instance().getBpsInfo();
    std::map<QString,int> codeTypeInfo = PlayParamasManager::instance().getCodeTypeInfo();
    for(auto iter: bpsInfo){
        ui->bpsComboBox_->addItem(iter.first);
    }
    for(auto iter: codeTypeInfo){
        ui->codeTypeComboBox->addItem(iter.first);
    }

    loadModelFromDb();

    handle_                     = 0;

    playWidget_                 = nullptr;
    videoDecodeThread_          = nullptr;

//    UpgradeWidget* upW = new UpgradeWidget();
//    upW->setWindowModality(Qt::ApplicationModal);
//    if(0 == 0){
//        upW->setFlag(true);
//    }else{
//        upW->setFlag(false);
//    }
//    upW->exec();
//    delete upW;

}

MainWindow::~MainWindow()
{
    delete ui;
}
// ok
void MainWindow::on_scanDeviceButton__clicked()
{
    std::vector<std::string> rtspAddress;

    qInfo()<<"Close video play for scan device....";
    closeVideoPlay();
    qInfo()<<"Successed close video play for scan device";

    qInfo()<<"Close device for scan device....";
    closeDevice();
    qInfo()<<"Successed close device for scan device";

    ui->devicesComboBox_->clear();

    qInfo()<<"Scan device...";
    int count = ScanIPCAddr(5,rtspAddress);
    qInfo()<<"Successed scan device";

    QString logs("Scan ipc finished, return ipc number=");
    logs += QString::number(count);
    for(auto address: rtspAddress){
        logs += ", " + QString::fromStdString(address);
    }

    for(auto address: rtspAddress){

        QString qStr = QString::fromStdString(address);

        QStringList splitList = qStr.split(':');

        if(splitList.size() < 3){
            qCritical()<<"scan device,but get invalid rtsp url,url="<< qStr;
            continue;
        }

        QString ip = splitList[1].mid(2);
        QStringList portLists = splitList[2].split('/');

        if(portLists.size() < 2){
            qCritical()<<"scan device,but get invalid rtsp url,url="<< qStr;
            continue;
        }

        QString port = portLists[0];

        QString addrStr = ip + ":" + port;

        rtspAddress_[ip] = qStr;
    }

    // update ipc combox
    for(auto ipItems: rtspAddress_){
        ui->devicesComboBox_->addItem(ipItems.first);
    }
    qInfo()<<logs;

    if(!rtspAddress_.empty()){
        chooseOneDevice();
    }
}

// ok
void MainWindow::chooseOneDevice(){
    // 1. 查询设备信息  1.1 查询固件版本
    qInfo()<<"get device version...";
    QString remoteVersion = getDeviceVersion();
    if(remoteVersion.isEmpty()){
        closeDevice();
        remoteVersion = kNoneDeviceVersion;
        qCritical()<<"Failed to get device version";
    }
    qInfo()<<"Successed get device version,version="<<remoteVersion;
    ui->deviceVersionlLineEdit_->setText(remoteVersion);

    // 1.2 查询model信息， 无效的模型显示None
    qInfo()<<"query remote model...";
    QString remoteModelName = queryRemoteModel();
    if(remoteModelName.isEmpty()){
        remoteModelName = kNoneModelName;
    }else if(dbModelNames_.find(remoteModelName) == dbModelNames_.end()){
        qCritical()<< "Remote model invalid, you should delete it.";
        remoteModelName = kNoneModelName;
    }
    qInfo()<<"Successed query remote model,model name="<<remoteModelName_;
    remoteModelName_ = remoteModelName;
    ui->aiModelComboBox_->setCurrentText(remoteModelName_);

    // 1.3 查询播放参数, 查询不到参数显示None
    qInfo()<<"query remote device play paramas...";
    video_enc_param enc;
    if(queryRemoteDevicePlayParamas(enc)){
       QString bpsStr       = PlayParamasManager::instance().strBps(enc.enc_bps);
       QString codeTypeStr  = PlayParamasManager::instance().strCodecType(enc.enc_type);
       qInfo()<<"Successed query remote play paramas, bps="<<bpsStr
             <<", code type="<< codeTypeStr;
       ui->bpsComboBox_->setCurrentText(bpsStr);
       ui->codeTypeComboBox->setCurrentText(codeTypeStr);
    }else{
        closeDevice();
        qCritical()<<"Failed to get device play paramas";
    }
}
// ok
bool MainWindow::queryRemoteDevicePlayParamas(video_enc_param& enc){

    if(!openDevice()){
        return false;
    }
    struct video_enc_param tempEnc;
    int ret = GetEncParam(handle_,&tempEnc);
    qInfo()<<"Finished get encode paramas,bps="<<tempEnc.enc_bps
          <<", code type="<<tempEnc.enc_type;
    if(0 != ret){
        closeDevice();
        qCritical()<<"Failed query remote device encode paramas.'GetEncParam' ret="<<ret;
        return false;
    }
    enc = tempEnc;
    return true;
}
// ok
bool MainWindow::setRemoteDevicePlayParamas(video_enc_param& enc){

    if(!openDevice()){
        return false;
    }
    struct video_enc_param tempEnc = enc;
    int ret = SetEncParam(handle_,&tempEnc);
    if(0 != ret){
        closeDevice();
        qCritical()<<"Failed update remote device encode paramas.'SetEncParam' ret="<<ret;
        return false;
    }
    return true;
}
// ok
QString MainWindow::getDeviceVersion(){
    if(!openDevice()){
        return QString("");
    }
    char versionInputBuf[1024];
    memset(versionInputBuf,0,1024);
    int ret = GetDeviceVer(handle_,versionInputBuf);
    if(0 != ret){
        closeDevice();
        return QString("");
    }else{
        return QString::fromLocal8Bit(versionInputBuf);
    }
}
// ok
QString MainWindow::queryRemoteModel(){
    if(!openDevice()){
        return QString("");
    }
    char remoteModelName[256];
    memset(remoteModelName,0,256);
    int ret = GetAIMode(handle_,remoteModelName);
    if(0 != ret){
        closeDevice();
        qCritical()<<"Failed get remote ai model, 'GetAIMode' ret="
                  << ret;
        return QString("");
    }
    QString remoteModel =  QString::fromLocal8Bit(remoteModelName);
    qInfo()<<"Successed get ai model, remote ai model name is "
          << remoteModel;
    return remoteModel;
}
// ok
bool MainWindow::updateModel(QString modelName){

    if(!openDevice()){
        return false;
    }

    QString blobName    = kAiModelRootPath + "/" + modelName + ".blob";
    QString xmlName     = kAiModelRootPath + "/" + modelName + ".xml";

    qInfo()<<"Start updtae ai model, model name="<<blobName;

    int ret = UpdateAiModel(handle_,blobName.toLocal8Bit().toStdString().c_str(),
                  xmlName.toLocal8Bit().toStdString().c_str(),
                  modelName.toLocal8Bit().toStdString().c_str());

    if(ret != 0){
        closeDevice();
        qCritical()<<"Failed updtae ai model,model name="
                  <<modelName
                 <<", 'UpdateAiModel' ret = "
                <<ret;
        return false;
    }

    qInfo()<<"Successed updtae ai model,model name="
          <<modelName;

    return true;
}

void MainWindow::on_playButton__clicked(){

    QString choosedDeviceIp = ui->devicesComboBox_->currentText();
    if(choosedDeviceIp.isEmpty() || rtspAddress_.count(choosedDeviceIp) <= 0){
        qCritical()<<"Failed to open rtsp stream,invalid devivce ip="
                  <<choosedDeviceIp;
        return;
    }

    QString rtspUrl = rtspAddress_[choosedDeviceIp];
    qInfo()<<"Play by rtsp: rtsp url="<< rtspUrl;

    closeDevice();

    qInfo()<<"query remote model...";
    QString remoteModelName = queryRemoteModel();
    remoteModelName_ = remoteModelName;
    qInfo()<<"Successed query remote model,model name="<<remoteModelName_;

    qInfo()<<"query device play paramas...";
    video_enc_param enc;
    bool ret = queryRemoteDevicePlayParamas(enc);
    if(!ret){
        qCritical()<<"Fauled to query remote device play paramas.";
        closeDevice();
    }else{
        QString bpsStr = PlayParamasManager::instance().strBps(enc.enc_bps);
        QString codeTypeStr = PlayParamasManager::instance().strCodecType(enc.enc_type);
        qInfo()<<"remote bps="<<bpsStr
              <<", remote code type="<<codeTypeStr;
    }

    openVideoPlay(rtspUrl,1);
    elaTimer_.restart();
    qInfo()<<"Play rtsp...!";
}

// ok
// 1. close play
// 2. delete or update model
// 3. restart mainapp
void MainWindow::on_updateAiModelButton__clicked(){

    QString logs;

    qInfo()<<"Clicked update ai model...";

    QString modelName = ui->aiModelComboBox_->currentText();
    if(modelName.isEmpty()){
        QMessageBox::warning(this,
                             "No model Select",
                             "No model Select");
        return;
    }

    qInfo()<<"Clicked update ai model,query Remote Model...";
    QString remoteModelName = queryRemoteModel();
    if(modelName.compare(remoteModelName) == 0){
         qInfo()<<"Need not update!";
        return;
    }

    qInfo()<<"Close video play for update ai model clicked...";
    closeVideoPlay();

    if(modelName.compare(kNoneModelName) == 0){
        int ret = DelAIMode(handle_);
        if(ret != 0){
            qCritical()<<"Failed to delete model";
            return;
        }
        logs  = QString("Successed delete remote model,remote model=")
                + remoteModelName;
        sendLogToText(logs);
    }else{
        updateModel(modelName);
    }

    qInfo()<<"Successed update model";

    qInfo()<<"update model for update ai model clicked...";
    rebootApp();

    qInfo()<<"Successed update ai model for update ai model clicked";
}
// ok
void MainWindow::closeDevice(){
    if(handle_ > 0){
        CloseDevice(handle_);
        handle_ = 0;
    }
}
// ok
bool MainWindow::openDevice(){

    if(handle_ > 0){
        return true;
    }

    QString choosedDeviceIp = ui->devicesComboBox_->currentText();
    if(rtspAddress_.count(choosedDeviceIp) < 0 || choosedDeviceIp.isEmpty()){
        QMessageBox::warning(this,
                             "Failed connect device",
                             "No device Select");
        return false;
    }

    handle_ = ConnectToDevice(choosedDeviceIp.toLocal8Bit().toStdString().c_str());

    if(handle_ <= 0){
        qCritical()<<"Failed connect device,device ip="<<choosedDeviceIp;
        return false;
    }

    qInfo()<<"Successed connect device,device ip="<<choosedDeviceIp;
    return true;
}
// ok
void MainWindow::on_rebootDeviceButton__clicked(){

    qInfo()<<"Restart Device...";

    if(!openDevice()){
        qCritical()<<"Failed to restart device,can not connect device";
        return;
    }

    int ret = RebootByRemote(handle_,1);
    if(ret != 0){
        closeDevice();
        qCritical()<<"Failed to restart device, 'RebootByRemote' cmd return "
                  << ret;
        return;
    }

    qInfo()<<"Successed to restart Device.";
}

void MainWindow::openVideoPlay(QString rtspUrl,int delayTime){

    closeVideoPlay();

    playWidget_                         = new PlayWidget;
    videoDecodeThread_                  = new VideoDecodeThread;

    connect(videoDecodeThread_,
            &VideoDecodeThread::updateMat,
            this,
            &MainWindow::updateMat);

    connect(videoDecodeThread_,
            &VideoDecodeThread::closeDevice,
            this,
            &MainWindow::rtspClose);

    connect(videoDecodeThread_,
            &VideoDecodeThread::restartRtspPlay,
            this,
            &MainWindow::restartRtspPlay);

    connect(videoDecodeThread_,
            &VideoDecodeThread::rtspServerError,
            this,
            &MainWindow::rtspServerError);

    connect(playWidget_,
            &PlayWidget::widgetCloseInternal,
            this,
            &MainWindow::widgetCloseInternal);

    playWidget_->show();
    videoDecodeThread_->playerRtspStream(rtspUrl,delayTime);
}

void MainWindow::closeVideoPlay(){
    if(playWidget_){
        playWidget_->hide();
        delete playWidget_;
        playWidget_ = nullptr;
    }
    if(videoDecodeThread_){
        videoDecodeThread_->stopRtspStream();
        delete videoDecodeThread_;
        videoDecodeThread_ = nullptr;
    }
}
// ok
void MainWindow::rebootApp(){

    qInfo()<<"Wait remote app restart...";

    if(!openDevice()){
        qCritical()<<"Failed to restart App,can not connect device.";
        return;
    }

    int ret = RebootByRemote(handle_,2);
    if(ret != 0){
        closeDevice();
        qCritical()<<"Failed to restart App, 'RebootByRemote' cmd return "
                  << ret;
        return;
    }

    closeDevice();

    qInfo()<<"Successed to restart App.";
}
// ok
void MainWindow::on_selectPkgButton__clicked(){

   //pkgPath_ = selectFwFile();
    choosepkg* pkg1 = new choosepkg();
    pkg1->setWindowModality(Qt::ApplicationModal);
    pkg1->exec();
    pkgPath_ = pkg1->getPkgName();
    delete pkg1;
    
   qInfo()<<"pkgPath_="<<pkgPath_;
   QString baseName;
   if(!pkgPath_.isEmpty()){
       QFileInfo fileInfo(pkgPath_);
       baseName = fileInfo.fileName();
   }
   
    qInfo()<<"Selectd one update pkg, pkg path="
            <<pkgPath_
            <<", pkg base name="
            <<baseName;
}
// ok
void MainWindow::on_upgradeButton__clicked(){

    qInfo()<<"Start upgrade pkg...,pkg path="<<pkgPath_;

    if(!openDevice()){
        qCritical()<<"Failed upgrade pkg, can not connect remote device.";
        return;
    }

    if(pkgPath_.isEmpty()){
        qCritical()<<"Failed to upgrade pkg,please select one .ec pkg";
        return;
    }

    QFile ecFile(pkgPath_);
    if(!ecFile.exists()){
        qCritical()<<"Failed to upgrade pkg, can not found upgrade pkg, upgrade pkg name ="
                  << pkgPath_;
        return;
    }

    int ret = StartUpgrade(handle_,pkgPath_.toLocal8Bit().toStdString().c_str());

    if(ret != 0){
        closeDevice();
        qCritical()<<"Failed to upgrade pkg, 'StartUpgrade' interface of 'cli sdk' return "
                    << ret
                    <<  ", pkg name is "
                    << pkgPath_;
        //return;
    }

    UpgradeWidget* upW = new UpgradeWidget();
    upW->setWindowModality(Qt::ApplicationModal);
    if(0 == ret){
        upW->setFlag(true);
    }else{
        upW->setFlag(false);
    }
    upW->exec();
    delete upW;

    if(ret == 0){
        on_rebootDeviceButton__clicked();
    }
}

void MainWindow::widgetCloseInternal(){
    qInfo()<<"closeVideoPlay for close playwidget...";
    closeVideoPlay();

    qInfo()<<"Successed close playwidget";
}

void MainWindow::closeEvent(QCloseEvent *event){

    qInfo()<<"closeVideoPlay for close mainwindows...";
    closeVideoPlay();

    qInfo()<<"CloseDevice for close mainwindows...";
    closeDevice();

    qInfo()<<"Successed close mainqindows";
    
    handle_ = 0;
    exit(0);
}
// ok
void MainWindow::updateMat(cv::Mat mat){
    static int numberNoFps = 0;
    static int lastFps = 0;
    static int recvCount = 0;
    
    int ret = 0, size = 0;

    if(nullptr == videoDecodeThread_ || nullptr == playWidget_){
        // Notes: if rtsp stream has stopped,
        // but has one mat has been events queue,
        // fliter it.
        return;
    }
    
    if(openDevice()){

        if(recvCount % 100 == 0){
            qInfo()<<"ReadMetaData...";
        }

        BYTE meta[1024 * 100];
        size = sizeof(meta);
        ret = ReadMetaData(handle_,meta,size);
        if(ret == 0){

            if(0 ==remoteModelName_.compare("classification-fp16")){
                mat = cls_show_img_func(mat,
                                     (char*)meta+sizeof(frameSpecOut)+OUTPUT_INDEX_SIZE,
                                     0.3);
            }else if(0 ==remoteModelName_.compare("face-detection-retail-0004-fp16")){
                mat = fd_show_img_func(mat,
                                     (char*)meta+sizeof(frameSpecOut)+OUTPUT_INDEX_SIZE,
                                     0.3);
            }else if(!remoteModelName_.isEmpty()){
                mat = pintScoreOnMat(mat,
                                     (char*)meta+sizeof(frameSpecOut)+OUTPUT_INDEX_SIZE,
                                     0.3);
            }
            frameSpecOut* out = (frameSpecOut*)meta;
            if(out->res[8] > 0){
                int fps = 1000 / out->res[8];
                if(fps > 0){
                    lastFps = fps;
                } 
                numberNoFps = 0;
            }else{
                numberNoFps++;
            }

        }else {
            //qInfo()<<"Failed for 'ReadMetaData',ret="<<ret;
            if(-2 == ret){
                qInfo()<<"Failed for 'ReadMetaData', close device";
                closeDevice();
            }else{
                numberNoFps++;
            }
        }

        if(numberNoFps < 60 && lastFps > 0){
            char buf[256];
            memset(buf,0,256);
            sprintf(buf, "fps: %d",lastFps);
            cv::Point                   origin;
            origin.x = 32 ;
            origin.y = 80;
            cv::putText(mat, buf, origin, cv::FONT_HERSHEY_COMPLEX, 1,  cv::Scalar(255, 255, 128), 1, 8, 0);
        }
        //qInfo()<<"numberNoFps="<<numberNoFps<<", lastFps="<<lastFps;

    }
    
    if(recvCount++ % 2000 == 0){
        qInfo()<<"update mat, recv count="
              <<recvCount;
    }
    
    QImage img = cvMat2QImage(mat);
    playWidget_->updateMat(img);
}
// ok
void MainWindow::sendLogToText(QString log){
    ui->LogEdit_->append(log);
}

void MainWindow::restartRtspPlay(QString rtspUrl){
    openVideoPlay(rtspUrl,1);
}

void MainWindow::on_devicesComboBox__currentTextChanged(const QString &arg1)
{
    qInfo()<<"devices comboBox Changed, current text="<<arg1;
    if(arg1.isEmpty()){
        return;
    }
    closeVideoPlay();
    closeDevice();
    chooseOneDevice();
}

void MainWindow::rtspServerError(QString rtspUrl){

    closeDevice();

    // number of restart rtsp server per min = 1;
    qInfo()<<"Restart remote rtsp server..., rtsp url="<< rtspUrl;

    if(nullptr == videoDecodeThread_ || nullptr == playWidget_){
        QString logs = "Play Widget closed, do not restart rtsp server.";
        IpcLog::IpcLogger().info(logs);
        return;
    }

    if(openDevice()){
        int ret = RebootByRemote(handle_,2);
        if(ret != 0){
            closeDevice();
            qCritical()<<"Failed restart rtsp server, 'RebootByRemote' cmd return "
                      << ret;
        }
    }else{
        qCritical()<<"Failed restart rtsp server, can not connect remote device.";
    }
    openVideoPlay(rtspUrl,15);
}

void MainWindow::disableChangePlayParams(){
    ui->bpsComboBox_->setDisabled(true);
    ui->codeTypeComboBox->setDisabled(true);
}

void MainWindow::enableChangePlayParams(){
     ui->bpsComboBox_->setDisabled(false);
     ui->codeTypeComboBox->setDisabled(false);
}

void MainWindow::rtspClose(){
    qInfo()<<"close device for power failed...";
    closeDevice();
    qInfo()<<"Successed close device for power failed";
}

void MainWindow::on_updatePlayButton__clicked()
{
    bool needStartMainApp = false;
    video_enc_param enc,currentEnc;
    qInfo()<<"query remote device play paramas...";
    bool ret = queryRemoteDevicePlayParamas(enc);

    if(!ret){
         qCritical()<<"Failed to query remote device play paramas.";
         closeDevice();
    }else{
        qInfo()<<"Successed query remote device play paramas,enc bps="<<enc.enc_bps
              <<", code type="<<enc.enc_type;

        QString currentBpsStr = ui->bpsComboBox_->currentText();
        QString currentCodeTypeStr = ui->codeTypeComboBox->currentText();
        
        currentEnc.enc_bps = PlayParamasManager::instance().intBps(currentBpsStr);
        currentEnc.enc_type = PlayParamasManager::instance().intCodeType(currentCodeTypeStr);

        if(enc.enc_bps != currentEnc.enc_bps || enc.enc_type != currentEnc.enc_type){
            qInfo()<<"set remote device play paramas...";
            int ret = SetEncParam(handle_,&currentEnc);
            if(0 != ret){
                qCritical()<<"Failed update remote device encode paramas.'SetEncParam' ret="<<ret;
            }
            qInfo()<<"Successed set remote device play paramas";
            needStartMainApp = true;
        }
    }
    if(needStartMainApp){
        rebootApp();
    }
}

void MainWindow::on_pushButton_clicked()
{
    closeDevice();

    QString choosedDeviceIp = ui->devicesComboBox_->currentText();
    if(choosedDeviceIp.isEmpty() || rtspAddress_.count(choosedDeviceIp) <= 0){
        qCritical()<<"Failed to open rtsp stream,invalid devivce ip="
                  <<choosedDeviceIp;
        return;
    }

    QString rtspUrl = rtspAddress_[choosedDeviceIp];
    qInfo()<<"Play by rtsp: rtsp url="<< rtspUrl;

    if(openDevice()){
        int ret = RebootByRemote(handle_,2);
        if(ret != 0){
            closeDevice();
            qCritical()<<"Failed restart rtsp server, 'RebootByRemote' cmd return "
                      << ret;
        }
    }else{
        qCritical()<<"Failed restart rtsp server, can not connect remote device.";
    }
    //openVideoPlay(rtspUrl,15);

    closeDevice();
}
