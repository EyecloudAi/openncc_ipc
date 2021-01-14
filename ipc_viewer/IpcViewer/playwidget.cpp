// Copyright (C) 2020-2022 Eyecloud Corporation
// SPDX-License-Identifier: Apache-2.0
#include "playwidget.h"
#include <QPixmap>
#include <QDebug>



PlayWidget::PlayWidget(QWidget *parent) : QWidget(parent)
{
    imageLabel_ = new QLabel(this);
    imageLabel_->resize(1920,1080);
}

PlayWidget::~PlayWidget(){
    delete imageLabel_;
}

void PlayWidget::updateMat(QImage img){
    imageLabel_->setPixmap(QPixmap::fromImage(img));
}

void PlayWidget::closeEvent(QCloseEvent *event){
    qInfo()<<"Close PlayWidget on 'PlayWidget'...";
    emit widgetCloseInternal();

}
