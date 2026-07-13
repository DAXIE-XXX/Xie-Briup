#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui { class LoginDialog; }
QT_END_NAMESPACE

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

    QString getServerIP() const;
    quint16 getServerPort() const;

private slots:
    void onConnectClicked();

private:
    Ui::LoginDialog *ui;
    QString serverIP;
    quint16 serverPort;
};

#endif // LOGINDIALOG_H