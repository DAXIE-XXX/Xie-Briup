#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class LoginPage;
}
QT_END_NAMESPACE

class LoginPage : public QWidget
{
    Q_OBJECT

private:
    explicit LoginPage(QWidget *parent = nullptr);

public:
    static LoginPage *getObject();
    ~LoginPage() override;
    QString getUserName();

private slots:
    void registerReSlot(QString username, bool re);
    void loginReSlot(QString username, bool re);

    void on_btnReg_clicked();

    void on_btnGotoReg_clicked();

    void on_btnLogin_clicked();

    void on_btnGotoLogin_clicked();

    void on_leLgPassword_returnPressed();

private:
    static LoginPage *obj;
    Ui::LoginPage *ui;
};
#endif // LOGINPAGE_H
