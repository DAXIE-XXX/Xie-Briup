#include "userchangepasswordpage.h"
#include "ui_userchangepasswordpage.h"
#include "netcontext.h"
#include <QMessageBox>
#include "mainpage.h"
#include "loginpage.h"

UserChangePasswordPage *UserChangePasswordPage::obj = nullptr;
UserChangePasswordPage::UserChangePasswordPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UserChangePasswordPage)
{
    ui->setupUi(this);
    connect(NetContext::getObject(),
            &NetContext::userChangePdRe,
            this,
            &UserChangePasswordPage::userChangePdReSlot);
}

UserChangePasswordPage *UserChangePasswordPage::getObject()
{
    if(obj == nullptr)
        obj = new UserChangePasswordPage;
    return obj;
}

UserChangePasswordPage::~UserChangePasswordPage()
{
    delete ui;
}

void UserChangePasswordPage::userChangePdReSlot(bool re)
{
    QMessageBox::information(
        this, "提示", QString("修改密码：%1").arg(re ? "成功" : "失败"));
    if(re){
        this->hide();
        MainPage::getObject()->hide();
        LoginPage::getObject()->show();
    }
}

void UserChangePasswordPage::on_btnChanged_clicked()
{
    QString oldPD = ui->leOldPD->text();
    QString newPD1 = ui->leNewPd1->text();
    QString newPD2 = ui->leNewPd2->text();
    if(oldPD.isEmpty() || newPD1.isEmpty() || newPD2.isEmpty()){
        QMessageBox::warning(
            this, "提示", "用户名密码不能为空");
        return;
    }
    if(newPD1 != newPD2){
        QMessageBox::warning(
            this, "提示", "两次密码不一致");
        return;
    }
    NetContext::getObject()->userChangePassword(oldPD, newPD1);
}

