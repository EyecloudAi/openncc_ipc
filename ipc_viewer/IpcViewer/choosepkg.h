#ifndef CHOOSEPKG_H
#define CHOOSEPKG_H

#include <QObject>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QDialog>
class choosepkg : public QDialog
{
    Q_OBJECT
public:
    choosepkg(QDialog *parent = nullptr);
    QString getPkgName(){
        return pkgName_;
    }
public slots:
    void updatePkg();
    void okClicked();
private:

    QPushButton*        button_;
    QPushButton*        okButton_;
    QLineEdit*          line_;
    QString             pkgName_;
};

#endif // CHOOSEPKG_H
