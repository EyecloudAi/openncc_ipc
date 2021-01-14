#include "upgradewidget.h"
#include <QHBoxLayout>
UpgradeWidget::UpgradeWidget(QDialog *parent) : QDialog(parent)
{
    label_ = new QLabel(this);
    flag_ = false;

    QHBoxLayout* layout_ = new QHBoxLayout(this);


    label_->setMinimumHeight(150);
    label_->setMinimumWidth(400);

    layout_->addWidget(label_);

}

void UpgradeWidget::setFlag(bool flag){
    flag_ = flag;
    if(flag_){
        label_->setText("Successed upgrade, please restart scan device.");
    }else{
        label_->setText("Failed upgrade.");
    }
}
