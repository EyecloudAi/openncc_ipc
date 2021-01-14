#include "choosepkg.h"
#include <QLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QDebug>

QString selectFwFile(){
    QString fwName;

    QFileDialog *fileDialog = new QFileDialog(nullptr);

    fileDialog->setWindowTitle(QStringLiteral("选中文件"));

    fileDialog->setDirectory(".");

    fileDialog->setNameFilter(("File(*.ec)"));

    fileDialog->setFileMode(QFileDialog::ExistingFiles);
    fileDialog->setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (fileDialog->exec()) {
        fileNames = fileDialog->selectedFiles();
        if(fileNames.size() >= 1){
            qInfo()<<"Select mvcmd name="<<fileNames[0];
            fwName = fileNames[0];
        }
    }

    if(fwName.isEmpty()){
        qCritical()<<"Error: fw name is Empty,Please select fw!";
        return QString();
    }

    return fwName;
}


choosepkg::choosepkg(QDialog *parent)
{


    button_ = new QPushButton(this);
    line_ = new QLineEdit(this);
    okButton_ = new QPushButton(this);

    QHBoxLayout* layout_ = new QHBoxLayout(this);

    layout_->addWidget(button_);
    layout_->addWidget(line_);

    button_->setMinimumHeight(40);
    button_->setMinimumWidth(100);

    line_->setMinimumHeight(40);
    line_->setMinimumWidth(300);

    okButton_->setMinimumHeight(40);
    okButton_->setMinimumWidth(60);

    button_->setText("choose pkg");
    okButton_->setText("ok");

    layout_->addWidget(okButton_);

    this->setToolTip("please choose pkg to upgrade");

    connect(button_, SIGNAL(clicked()), this ,SLOT(updatePkg()));
    connect(okButton_, SIGNAL(clicked()), this ,SLOT(okClicked()));

    this->resize(500,300);
}

void choosepkg::okClicked(){
    this->close();
    //exit(0);
}

void choosepkg::updatePkg(){
    QString pkgName = selectFwFile();
    pkgName_ = pkgName;
    line_->setText(pkgName_);
    //qInfo()<<

}
