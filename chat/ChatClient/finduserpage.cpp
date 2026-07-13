#include "finduserpage.h"
#include "ui_finduserpage.h"
#include "netcontext.h"
#include <QJsonObject>
#include <QListWidgetItem>
#include <QInputDialog>
#include <QMessageBox>

FindUserPage *FindUserPage::obj = nullptr;
FindUserPage::FindUserPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FindUserPage)
{
    ui->setupUi(this);
    connect(NetContext::getObject(),
            &NetContext::userFindList,
            this,
            &FindUserPage::userFindListSlot);
    connect(NetContext::getObject(),
            &NetContext::userAddFriendRequestRe,
            this,
            &FindUserPage::userAddFriendRequestReSlot);
}

FindUserPage *FindUserPage::getObject()
{
    if(obj == nullptr)
        obj = new FindUserPage;
    return obj;
}

FindUserPage::~FindUserPage()
{
    delete ui;
}

void FindUserPage::userFindListSlot(const QJsonArray &list)
{
    ui->lwUserList->clear();
    for(int i =0;i < list.size(); i++){
        QString name = list.at(i).toObject()
                                .value("name").toString();
        QString username = list.at(i).toObject()
                               .value("username").toString();
        ui->lwUserList->addItem(
            QString("%1-%2-双击发送好友申请")
                .arg(username).arg(name)
            );
    }
}

void FindUserPage::userAddFriendRequestReSlot(bool re)
{
    QMessageBox::information(
        this, "提示", QString("发送申请：%1").arg(re ? "成功" : "失败"));
}

void FindUserPage::on_btnFind_clicked()
{
    QString name = ui->leName->text();
    if(name.isEmpty())
        return;
    NetContext::getObject()->userFind(name);
}

void FindUserPage::on_lwUserList_itemDoubleClicked(QListWidgetItem *item)
{
    bool ok = false;
    QString msg = QInputDialog::getText(
        this, "输入申请信息", "申请信息：", QLineEdit::Normal, "",
        &ok);
    if(!ok)
        return;
    QString username = ui->lwUserList->currentItem()->text()
                           .split("-").at(0);
    NetContext::getObject()->userAddFriendRequest(username, msg);
}







