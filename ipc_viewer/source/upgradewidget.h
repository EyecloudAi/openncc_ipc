#ifndef UPGRADEWIDGET_H
#define UPGRADEWIDGET_H

#include <QObject>
#include <QObject>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QDialog>
#include <QLabel>

class UpgradeWidget : public QDialog
{
    Q_OBJECT
public:
    explicit UpgradeWidget(QDialog *parent = nullptr);
    void setFlag(bool flag);
signals:


private:
    QLabel* label_;
    bool flag_;
};

#endif // UPGRADEWIDGET_H
