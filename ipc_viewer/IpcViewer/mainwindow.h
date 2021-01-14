/*Copyright 2020 Eyecloud, Inc

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include "videodecodethread.h"
#include "playwidget.h"
#include "cliSdk.h"
#include "deviceagent.h"
#include <memory>
#include <chrono>
#include "aiutil.h"
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <set>
namespace Ui {
class MainWindow;
}

struct PlayParamasManager{
    
    static PlayParamasManager& instance(){
        static PlayParamasManager inst;
        return inst;
    }

    std::map<QString,int> getBpsInfo(){
        return bpsMap_;
    }

    std::map<QString,int> getCodeTypeInfo(){
        return codeTypeMap_;
    }

    PlayParamasManager(){

        // create map: "2M Bit/s"-2000000
        std::vector<QString> bpsInfo{QString("1M Bit/s"),
                                        QString("2M Bit/s"),
                                        QString("3M Bit/s"),
                                        QString("4M Bit/s"),
                                        QString("5M Bit/s"),
                                        QString("6M Bit/s"),
                                        QString("7M Bit/s"),
                                        QString("8M Bit/s"),
                                        QString("9M Bit/s"),
                                        QString("10M Bit/s")
                                    };

        for(int i = 0;i < bpsInfo.size();i++){
            bpsMap_[bpsInfo[i]]                     = (i + 1) * 1000;
            bpsMapReverse_[(i + 1) * 1000]          = bpsInfo[i];
        }


        // create map: "H264"-264, "H265"-265
        std::vector<QString> codecInfo{QString("H264"),
                                      QString("H265")};
        for(int i = 0;i < 2;i++){
            codeTypeMap_[codecInfo[i]]              = i + 264;
            codeTypeMapReverse_[i + 264]            = codecInfo[i];  
           //ui->codeTypeComboBox->addItem(codecInfo[i]);
        }
    }
    
    QString strCodecType(int codeType){
        if(codeTypeMapReverse_.count(codeType) <= 0){
            return QString("");
        }else{
            return codeTypeMapReverse_[codeType];
        }
    }
    
    QString strBps(int bps){
        if(bpsMapReverse_.count(bps) <= 0){
            return QString("");
        }else{
            return bpsMapReverse_[bps];
        }
    }
    
    int intCodeType(QString codeType){
        if(codeTypeMap_.count(codeType) <= 0){
            return -1;
        }else{
            return codeTypeMap_[codeType];
        }  
    }
    
    int intBps(QString codeType){
        if(bpsMap_.count(codeType) <= 0){
            return -1;
        }else{
            return bpsMap_[codeType];
        }  
    }

    std::map<QString,int>               bpsMap_;
    std::map<QString,int>               codeTypeMap_;
    std::map<int,QString>               bpsMapReverse_;
    std::map<int,QString>               codeTypeMapReverse_;
};


struct StatusSynch{
    enum DeviveStatus{
        None,
        OpenDevice
    };
    StatusSynch(){
        if(status_ != None){
            return;
        }
    }
    ~StatusSynch(){

    }
    static DeviveStatus status_;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void loadModelFromDb();

    bool openDevice();

    void closeDevice();

    bool updateModel(QString modelName);

    QString  getDeviceVersion();

    void rebootApp();

    void closeVideoPlay();

    void openVideoPlay(QString rtspUrl, int delayTime);

    QString queryRemoteModel();

    void disableChangePlayParams();
    void enableChangePlayParams();

    bool queryRemoteDevicePlayParamas(video_enc_param& enc);
    bool setRemoteDevicePlayParamas(video_enc_param& enc);


    void chooseOneDevice();

private slots:
    void on_scanDeviceButton__clicked();

    void on_playButton__clicked();

    void on_updateAiModelButton__clicked();

    void on_rebootDeviceButton__clicked();

    void on_selectPkgButton__clicked();

    void on_upgradeButton__clicked();

    void on_devicesComboBox__currentTextChanged(const QString &arg1);

    void on_updatePlayButton__clicked();

    void on_pushButton_clicked();

public slots:
    void updateMat(cv::Mat mat);

    void widgetCloseInternal();// on play widget closed.

    void sendLogToText(QString log);

    void restartRtspPlay(QString rtspUrl);

    void rtspServerError(QString rtspUrl);

    void rtspClose(); // close devivce for power failure



protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::MainWindow *ui;

    std::map<QString, QString>          rtspAddress_; //  only modify by scan button.

    CLI_HANDLE                          handle_;

    PlayWidget                          *playWidget_;
    VideoDecodeThread                   *videoDecodeThread_;

    QString                             pkgPath_;

    std::shared_ptr<DeviceAgent>        deviceAgentPtr_;

    NccMideaElapsedTimer                elaTimer_;

    std::set<QString>                   dbModelNames_;
    QString                             remoteModelName_;

    video_enc_param                     playEnc_;


};

#endif // MAINWINDOW_H
