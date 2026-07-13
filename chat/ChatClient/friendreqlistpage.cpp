#include "friendreqlistpage.h"
#include "ui_friendreqlistpage.h"
#include "netcontext.h"
#include <QJsonObject>
#include <QMessageBox>
FriendReqListPage *FriendReqListPage::obj = nullptr;
FriendReqListPage::FriendReqListPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FriendReqListPage)
{
    ui->setupUi(this);
    connect(
        NetContext::getObject(),
        &NetContext::userRequestList,
        this,
        &FriendReqListPage::userRequestListSlot);
    connect(
        NetContext::getObject(),
        &NetContext::userAcceptReqRe,
        this,
        &FriendReqListPage::userAcceptReqReSlot);
}

FriendReqListPage *FriendReqListPage::getObject()
{
    if(obj == nullptr)
        obj = new FriendReqListPage;
    return obj;
}

FriendReqListPage::~FriendReqListPage()
{
    delete ui;
}

void FriendReqListPage::userRequestListSlot(const QJsonArray &list)
{
    ui->lwReqList->clear();
    for(int i =0;i < list.size(); i++){
        QJsonObject data = list.at(i).toObject();
        int id = data.value("req_id").toInt();
        QString name = data.value("name").toString();
        QString username = data.value("username").toString();
        QString reqMsg = data.value("req_msg").toString();
        QString reqTime = data.value("req_time").toString();
        ui->lwReqList->addItem(
            QString("%1-%2-%3-%4-%5")
                .arg(id).arg(username).arg(name).arg(reqMsg).arg(reqTime)
            );
    }
}

void FriendReqListPage::userAcceptReqReSlot(bool re)
{
    QMessageBox::information(
        this, "提示", QString("同意申请：%1").arg(re ? "成功" : "失败"));
    if(re){
        NetContext::getObject()->userGetReqList();
        NetContext::getObject()->userGetFriendList();
    }

}

void FriendReqListPage::on_btnAccept_clicked()
{
    if(ui->lwReqList->currentRow() < 0)
        return;
    int reqId = ui->lwReqList->currentItem()->text()
                    .split('-').at(0).toInt();
    NetContext::getObject()->userAcceptRequest(reqId);
}

