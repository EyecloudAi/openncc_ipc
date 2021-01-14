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
#ifndef PLAYWIDGET_H
#define PLAYWIDGET_H

#include <QObject>
#include <memory>
#include <queue>
#include <mutex>
#include <QPixmap>
#include <QLabel>
#include <QMetaType>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/highgui/highgui_c.h>

#include "cliSdk.h"

class PlayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PlayWidget(QWidget *parent = nullptr);
    ~PlayWidget();
    void updateMat(QImage img);

signals:
    void widgetCloseInternal();

protected:
    void closeEvent(QCloseEvent *event);

private:
    QLabel*                                                         imageLabel_;
};

#endif // PLAYWIDGET_H
