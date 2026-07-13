#include "loginpage.h"
#include "ui_loginpage.h"
#include "netcontext.h"
#include <QMessageBox>
#include <QRegularExpression>
#include "mainpage.h"
#include "mqttcontext.h"
#include "chathistorymanager.h"
LoginPage *LoginPage::obj = nullptr;
LoginPage::LoginPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginPage)
{
    ui->setupUi(this);
    connect(NetContext::getObject(),
            &NetContext::registerRe,
            this,
            &LoginPage::registerReSlot);
    connect(NetContext::getObject(),
            &NetContext::loginRe,
            this,
            &LoginPage::loginReSlot);
}

LoginPage *LoginPage::getObject()
{
    if(obj == nullptr)
        obj = new LoginPage;
    return obj;
}

LoginPage::~LoginPage()
{
    delete ui;
}

QString LoginPage::getUserName()
{
    return ui->leLgUsername->text();
}

void LoginPage::registerReSlot(QString username, bool re)
{
    QMessageBox::information(
        this, "提示", QString("注册：%1").arg(re ? "成功" : "失败"));
}

void LoginPage::loginReSlot(QString username, bool re)
{
    if(re){
        MainPage::getObject()->show();
        NetContext::getObject()->userGetInfo();
        NetContext::getObject()->userGetFriendList();
        this->hide();
        QString un = ui->leLgUsername->text();
        QString pd = ui->leLgPassword->text();
        bool ok = MQTTContext::getObject()->connectToBroker(
            QString("mqtt://%1:%2@127.0.0.1:1883").arg(un).arg(pd));
        ChatHistoryManager::getObject()->init(un);
    }
    else
        QMessageBox::information(
            this, "提示", QString("登陆：%1").arg(re ? "成功" : "失败"));
}

void LoginPage::on_btnReg_clicked()
{
    QString un = ui->leRegUsername->text();
    QString pd1 = ui->leRegPassword1->text();
    QString pd2 = ui->leRegPassword2->text();
    if(un.isEmpty() || pd1.isEmpty() || pd2.isEmpty()){
        QMessageBox::warning(
            this, "提示", "用户名密码不能为空");
        return;
    }
    if(pd1 != pd2){
        QMessageBox::warning(
            this, "提示", "两次密码不一致");
        return;
    }
    if(pd1.length() < 6){
        QMessageBox::warning(
            this, "提示", "密码长度最少6位");
        return;
    }
    bool hasLetter = pd1.contains(QRegularExpression("[a-zA-Z]"));
    if(!hasLetter){
        QMessageBox::warning(
            this, "提示", "密码必须包含至少一个字母");
        return;
    }
    NetContext::getObject()->userRegister(un, pd1);
}

void LoginPage::on_btnGotoReg_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageRegister);
}

void LoginPage::on_btnLogin_clicked()
{
    QString un = ui->leLgUsername->text();
    QString pd = ui->leLgPassword->text();
    if(un.isEmpty() || pd.isEmpty()){
        QMessageBox::warning(
            this, "提示", "用户名密码不能为空");
        return;
    }
    NetContext::getObject()->userLogin(un, pd);

}

void LoginPage::on_btnGotoLogin_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageLogin);
}

void LoginPage::on_leLgPassword_returnPressed()
{
    on_btnLogin_clicked();
}

