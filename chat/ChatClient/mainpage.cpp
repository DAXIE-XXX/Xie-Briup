#include "mainpage.h"
#include "ui_mainpage.h"
#include "netcontext.h"
#include <algorithm>
#include "edituserinfopage.h"
#include "finduserpage.h"
#include "friendreqlistpage.h"
#include <QJsonObject>
#include "userchangepasswordpage.h"
#include "mqttcontext.h"
#include "loginpage.h"
#include <QDateTime>
#include <QJsonDocument>
#include "chathistorymanager.h"
#include <QListWidgetItem>
#include <QInputDialog>
#include <QMessageBox>
#include <QProcess>
#include <QApplication>
#include <QMenu>
#include <QAction>
#include <QStringList>
#include <QPixmap>
#include <QImage>
#include <QBuffer>

MainPage *MainPage::obj = nullptr;
MainPage::MainPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainPage)
    , m_groupMenu(nullptr)
    , m_friendMenu(nullptr)
    , m_friendGroupMenu(nullptr)
    , m_currentGroupId(-1)
    , m_menuFriendGroupId(-1)
{
    ui->setupUi(this);
    ui->tbReadData->setStyleSheet("QTextBrowser { background-color: #F0F0F0; border: none; }");
    ui->lwGroupList->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->lwFriendList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(NetContext::getObject(),
            &NetContext::userInfo,
            this,
            &MainPage::userInfoSlot);
    connect(NetContext::getObject(),
            &NetContext::userFriendList,
            this,
            &MainPage::userFriendListSlot);
    connect(NetContext::getObject(),
            &NetContext::userSendChatRe,
            this,
            &MainPage::userSendChatSlot);
    connect(MQTTContext::getObject(),
            &MQTTContext::readSubscriptionMessageSig,
            this,
            &MainPage::readSubscriptionMessageSlot);
    connect(MQTTContext::getObject(),
            &MQTTContext::connected,
            this,
            &MainPage::mqttConnectedSlot);
    connect(NetContext::getObject(),
            &NetContext::groupList,
            this,
            &MainPage::groupListSlot);
    connect(NetContext::getObject(),
            &NetContext::groupCreateRe,
            this,
            &MainPage::groupCreateSlot);
    connect(NetContext::getObject(),
            &NetContext::groupSendChatRe,
            this,
            &MainPage::groupSendChatSlot);
    connect(NetContext::getObject(),
            &NetContext::logoutRe,
            this,
            &MainPage::logoutReSlot);
    connect(NetContext::getObject(),
            &NetContext::groupAddMemberRe,
            this,
            &MainPage::groupAddMemberReSlot);
    connect(NetContext::getObject(),
            &NetContext::groupRemoveMemberRe,
            this,
            &MainPage::groupRemoveMemberReSlot);
    connect(NetContext::getObject(),
            &NetContext::groupMembersList,
            this,
            &MainPage::groupMembersListSlot);
    connect(NetContext::getObject(),
            &NetContext::groupSetAdminsRe,
            this,
            &MainPage::groupSetAdminsReSlot);
    connect(NetContext::getObject(),
            &NetContext::groupDissolveRe,
            this,
            &MainPage::groupDissolveReSlot);
    connect(NetContext::getObject(),
            &NetContext::groupExitRe,
            this,
            &MainPage::groupExitReSlot);
    connect(NetContext::getObject(),
            &NetContext::groupAdminsList,
            this,
            &MainPage::groupAdminsListSlot);
    connect(NetContext::getObject(),
            &NetContext::groupRenameRe,
            this,
            &MainPage::groupRenameReSlot);
    connect(NetContext::getObject(),
            &NetContext::groupTransferRe,
            this,
            &MainPage::groupTransferReSlot);
    connect(NetContext::getObject(),
            &NetContext::friendBlockRe,
            this,
            &MainPage::friendBlockReSlot);
    connect(NetContext::getObject(),
            &NetContext::friendUnblockRe,
            this,
            &MainPage::friendUnblockReSlot);
    connect(NetContext::getObject(),
            &NetContext::friendGroupCreateRe,
            this,
            &MainPage::friendGroupCreateReSlot);
    connect(NetContext::getObject(),
            &NetContext::friendGroupDeleteRe,
            this,
            &MainPage::friendGroupDeleteReSlot);
    connect(NetContext::getObject(),
            &NetContext::friendGroupRenameRe,
            this,
            &MainPage::friendGroupRenameReSlot);
    connect(NetContext::getObject(),
            &NetContext::friendGroupList,
            this,
            &MainPage::friendGroupListSlot);
    connect(NetContext::getObject(),
            &NetContext::friendSetGroupRe,
            this,
            &MainPage::friendSetGroupReSlot);
    connect(NetContext::getObject(),
            &NetContext::friendSetRemarkRe,
            this,
            &MainPage::friendSetRemarkReSlot);
    connect(NetContext::getObject(),
            &NetContext::groupSetRemarkRe,
            this,
            &MainPage::groupSetRemarkReSlot);
    connect(ui->lwFriendList,
            &QListWidget::customContextMenuRequested,
            this,
            &MainPage::on_lwFriendList_customContextMenuRequested);
}

MainPage *MainPage::getObject()
{
    if(obj == nullptr)
        obj = new MainPage;
    return obj;
}

MainPage::~MainPage()
{
    delete ui;
    delete m_groupMenu;
    delete m_friendMenu;
    delete m_friendGroupMenu;
}

void MainPage::userInfoSlot(QString name, QString phone, QString email, QString info, QString birthday, QString avatar)
{
    ui->lbInfo->setText(
        QString("昵称：%1\n手机：%2\n邮箱：%3\n生日：%4\n签名：%5")
        .arg(name).arg(phone).arg(email).arg(birthday).arg(info)
        );
    if(!avatar.isEmpty()){
        QByteArray avatarData = QByteArray::fromBase64(avatar.toUtf8());
        QImage image;
        image.loadFromData(avatarData);
        if(!image.isNull()){
            QPixmap pixmap = QPixmap::fromImage(image.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->lbAvatar->setPixmap(pixmap);
            ui->lbAvatar->setScaledContents(true);
        }
        m_avatarCache.insert(LoginPage::getObject()->getUserName(), avatar);
    }
}

void MainPage::userFriendListSlot(const QJsonArray &list)
{
    m_friendList = list;
    ui->lwFriendList->clear();

    QMap<int, QJsonArray> groupFriends;
    QMap<int, QString> groupNames;

    groupNames.insert(0, "默认分组");
    for(int i = 0; i < m_friendGroupList.size(); i++){
        QJsonObject obj = m_friendGroupList.at(i).toObject();
        groupNames.insert(obj.value("group_id").toInt(), obj.value("group_name").toString());
    }

    for(int i = 0; i < list.size(); i++){
        QJsonObject data = list.at(i).toObject();
        int groupId = data.value("group_id").toInt();
        if(groupId == 0 || !groupNames.contains(groupId)){
            groupId = 0;
        }
        groupFriends[groupId].append(data);
    }

    QList<int> groupIds = groupNames.keys();
    std::sort(groupIds.begin(), groupIds.end());

    for(int i = 0; i < groupIds.size(); i++){
        int gid = groupIds.at(i);
        QString groupName = groupNames.value(gid);
        int friendCount = groupFriends.value(gid).size();

        QListWidgetItem *groupItem = new QListWidgetItem();
        groupItem->setSizeHint(QSize(260, 25));
        groupItem->setData(Qt::UserRole, -1);
        groupItem->setFlags(groupItem->flags() & ~Qt::ItemIsSelectable);

        QWidget *groupWidget = new QWidget();
        QHBoxLayout *groupLayout = new QHBoxLayout(groupWidget);
        groupLayout->setContentsMargins(5, 2, 5, 2);

        QLabel *lbGroupName = new QLabel();
        lbGroupName->setText(QString("%1 (%2)").arg(groupName).arg(friendCount));
        lbGroupName->setStyleSheet("font-weight: bold; color: #333333; background-color: #E8E8E8; padding: 2px 5px; border-radius: 3px;");

        groupLayout->addWidget(lbGroupName);
        groupLayout->addStretch();

        ui->lwFriendList->addItem(groupItem);
        ui->lwFriendList->setItemWidget(groupItem, groupWidget);

        QJsonArray friends = groupFriends.value(gid);
        for(int j = 0; j < friends.size(); j++){
            QJsonObject data = friends.at(j).toObject();
            QString name = data.value("name").toString();
            QString username = data.value("username").toString();
            QString phone = data.value("phone").toString();
            QString info = data.value("info").toString();
            QString avatar = data.value("avatar").toString();
            QString remark = data.value("remark").toString();
            int isBlocked = data.value("is_blocked").toInt();
            m_avatarCache.insert(username, avatar);

            QString displayName = remark.isEmpty() ? name : remark;
            QString blockedTag = isBlocked ? " [已拉黑]" : "";

            QListWidgetItem *item = new QListWidgetItem();
            item->setSizeHint(QSize(260, 70));
            item->setData(Qt::UserRole, username);

            QWidget *widget = new QWidget();
            QHBoxLayout *layout = new QHBoxLayout(widget);
            layout->setSpacing(8);
            layout->setContentsMargins(15, 5, 5, 5);

            QLabel *lbAvatar = new QLabel();
            lbAvatar->setFixedSize(50, 50);
            lbAvatar->setAlignment(Qt::AlignCenter);
            if(!avatar.isEmpty()){
                QByteArray avatarData = QByteArray::fromBase64(avatar.toUtf8());
                QImage image;
                image.loadFromData(avatarData);
                if(!image.isNull()){
                    QPixmap pixmap = QPixmap::fromImage(image.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    lbAvatar->setPixmap(pixmap);
                    lbAvatar->setScaledContents(true);
                    lbAvatar->setStyleSheet("border: 1px solid #CCCCCC; border-radius: 3px;");
                } else {
                    lbAvatar->setStyleSheet("border: 1px solid #CCCCCC; border-radius: 3px; background-color: #E0E0E0; color: #666666; font-size: 16px;");
                    lbAvatar->setText(displayName.left(1));
                }
            } else {
                lbAvatar->setStyleSheet("border: 1px solid #CCCCCC; border-radius: 3px; background-color: #E0E0E0; color: #666666; font-size: 16px;");
                lbAvatar->setText(displayName.left(1));
            }

            QLabel *lbInfo = new QLabel();
            lbInfo->setText(
                QString("%1%2\n账号：%3\n签名：%4")
                    .arg(displayName).arg(blockedTag)
                    .arg(username).arg(info)
                );
            lbInfo->setWordWrap(true);
            if(isBlocked){
                lbInfo->setStyleSheet("color: #999999;");
            }

            layout->addWidget(lbAvatar);
            layout->addWidget(lbInfo);
            layout->addStretch();

            ui->lwFriendList->addItem(item);
            ui->lwFriendList->setItemWidget(item, widget);
        }
    }
}

void MainPage::readSubscriptionMessageSlot(const QString &topic, const QByteArray &value)
{
    QJsonObject obj = QJsonDocument::
                      fromJson(value).object();
    int type = obj.value("type").toInt();
    if(type == 3001){
        QString fromUsername =
            obj.value("from_username").toString();
        QString msg = obj.value("msg").toString();
        QString time = obj.value("time").toString();
        qint64 index = ChatHistoryManager::getObject()
                           ->insertPrivateMessage(
                               fromUsername,
                               LoginPage::getObject()->getUserName(),
                               MessageSendStatus::Delivered, time, msg);
        if(ui->toolBox->currentIndex() == 0 &&
            ui->lwFriendList->currentRow() >= 0){
            if(ui->lwFriendList->currentItem()->data(Qt::UserRole).toString() == fromUsername)
            appendReceivedBubble(time, fromUsername, msg, m_avatarCache.value(fromUsername, ""));
        }
    }
    else if(type == 3002){
        QString fromUsername =
            obj.value("from_username").toString();
        int groupId = obj.value("group_id").toInt();
        QString msg = obj.value("msg").toString();
        QString time = obj.value("time").toString();
        ChatHistoryManager::getObject()
            ->insertGroupMessage(
                QString::number(groupId),
                fromUsername,
                MessageSendStatus::Delivered, time, msg);
        if(ui->toolBox->currentIndex() == 1 &&
            ui->lwGroupList->currentRow() >= 0){
            int row = ui->lwGroupList->currentRow();
            if(row < m_groupList.size()){
                int currentGroupId =
                    m_groupList.at(row).toObject()
                        .value("group_id").toInt();
                if(currentGroupId == groupId){
                    appendReceivedBubble(time, fromUsername, msg, m_avatarCache.value(fromUsername, ""));
                }
            }
        }
    }
    else if(type == 3004){
        loadGroupList();
    }
}

void MainPage::mqttConnectedSlot()
{
    MQTTContext::getObject()->subscribeTopic(
        QString("chat/%1").arg(LoginPage::getObject()->getUserName()), 1);
    loadGroupList();
    NetContext::getObject()->friendGroupGetList();
}

void MainPage::userSendChatSlot(bool re, int index)
{
    ChatHistoryManager::getObject()->updatePrivateMessageStatus(index, re ? MessageSendStatus::Delivered : MessageSendStatus::Failed);
}

void MainPage::groupListSlot(const QJsonArray &list)
{
    m_groupList = list;
    ui->lwGroupList->clear();
    for(int i = 0; i < list.size(); i++){
        QJsonObject data = list.at(i).toObject();
        QString groupName = data.value("group_name").toString();
        QString groupRemark = data.value("group_remark").toString();
        int groupId = data.value("group_id").toInt();
        QString displayName = groupRemark.isEmpty() ? groupName : groupRemark;
        ui->lwGroupList->addItem(
            QString("%1-%2").arg(groupId).arg(displayName)
            );
    }
}

void MainPage::groupCreateSlot(bool re, int groupId)
{
    if(re){
        QMessageBox::information(this, "提示", "群聊创建成功");
        loadGroupList();
    }
    else{
        QMessageBox::warning(this, "提示", "群聊创建失败");
    }
}

void MainPage::groupSendChatSlot(bool re, int index)
{
    ChatHistoryManager::getObject()->updateGroupMessageStatus(
        index, re ? MessageSendStatus::Delivered : MessageSendStatus::Failed);
}

void MainPage::logoutReSlot(bool re)
{
    QProcess::startDetached(QApplication::applicationFilePath(), {});
    QApplication::quit();
}

void MainPage::groupAddMemberReSlot(bool re)
{
    if(re){
        QMessageBox::information(this, "提示", "邀请成功");
        loadGroupList();
    }
    else{
        QMessageBox::warning(this, "提示", "邀请失败");
    }
}

void MainPage::groupRemoveMemberReSlot(bool re)
{
    if(re){
        QMessageBox::information(this, "提示", "操作成功");
        loadGroupList();
    }
    else{
        QMessageBox::warning(this, "提示", "操作失败");
    }
}

void MainPage::groupMembersListSlot(const QJsonArray &list)
{
    m_groupMembers = list;
    for(int i = 0; i < list.size(); i++){
        QJsonObject data = list.at(i).toObject();
        QString username = data.value("username").toString();
        QString avatar = data.value("avatar").toString();
        m_avatarCache.insert(username, avatar);
    }
}

void MainPage::groupSetAdminsReSlot(bool re)
{
    if(re){
        QMessageBox::information(this, "提示", "设置成功");
    }
    else{
        QMessageBox::warning(this, "提示", "设置失败");
    }
}

void MainPage::groupDissolveReSlot(bool re)
{
    if(re){
        QMessageBox::information(this, "提示", "群聊已解散");
        loadGroupList();
    }
    else{
        QMessageBox::warning(this, "提示", "解散失败");
    }
}

void MainPage::groupExitReSlot(bool re)
{
    if(re){
        QMessageBox::information(this, "提示", "已退出群聊");
        loadGroupList();
    }
    else{
        QMessageBox::warning(this, "提示", "退出失败");
    }
}

void MainPage::groupAdminsListSlot(const QJsonArray &list)
{
    m_groupAdmins = list;
    for(int i = 0; i < list.size(); i++){
        QJsonObject data = list.at(i).toObject();
        QString username = data.value("username").toString();
        QString avatar = data.value("avatar").toString();
        m_avatarCache.insert(username, avatar);
    }
    if(m_currentGroupId > 0 && m_groupMenuGroupIndex >= 0){
        showGroupManageMenuWithPermission();
    }
}

void MainPage::groupRenameReSlot(bool re)
{
    if(re){
        QMessageBox::information(this, "提示", "群名修改成功");
        loadGroupList();
    }
    else{
        QMessageBox::warning(this, "提示", "群名修改失败");
    }
}

void MainPage::groupTransferReSlot(bool re)
{
    if(re){
        QMessageBox::information(this, "提示", "群聊转让成功");
        loadGroupList();
    }
    else{
        QMessageBox::warning(this, "提示", "群聊转让失败");
    }
}

void MainPage::loadGroupList()
{
    NetContext::getObject()->groupGetList();
}

void MainPage::loadGroupHistory()
{
    int row = ui->lwGroupList->currentRow();
    if(row < 0 || row >= m_groupList.size()){
        ui->tbReadData->clear();
        return;
    }
    int groupId = m_groupList.at(row).toObject()
                      .value("group_id").toInt();
    QJsonArray list = ChatHistoryManager::getObject()
                          ->queryGroupMessages(QString::number(groupId));
    ui->tbReadData->clear();
    for(int i = list.size()-1; i >= 0; i--){
        QJsonObject obj = list.at(i).toObject();
        QString sender = obj["sender_username"].toString();
        QString time = obj["send_time"].toString();
        QString msg = obj["message_content"].toString();
        QString myAvatar = m_avatarCache.value(LoginPage::getObject()->getUserName(), "");
        QString senderAvatar = m_avatarCache.value(sender, "");
        if(sender == LoginPage::getObject()->getUserName())
            appendSentBubble(time, msg, myAvatar);
        else
            appendReceivedBubble(time, sender, msg, senderAvatar);
    }
}

void MainPage::showGroupManageMenu(int groupIndex)
{
    if(groupIndex < 0 || groupIndex >= m_groupList.size())
        return;

    QJsonObject groupInfo = m_groupList.at(groupIndex).toObject();
    m_currentGroupId = groupInfo.value("group_id").toInt();
    QString adminUsername = groupInfo.value("admin_username").toString();
    bool isOwner = (adminUsername == LoginPage::getObject()->getUserName());

    m_groupMenuGroupIndex = groupIndex;
    m_groupMenuIsOwner = isOwner;
    NetContext::getObject()->groupGetAdmins(m_currentGroupId);
}

void MainPage::showGroupManageMenuWithPermission()
{
    QString currentUsername = LoginPage::getObject()->getUserName();
    bool isAdmin = m_groupMenuIsOwner;
    if(!isAdmin){
        for(int i = 0; i < m_groupAdmins.size(); i++){
            QString username = m_groupAdmins.at(i).toObject().value("username").toString();
            if(username == currentUsername){
                isAdmin = true;
                break;
            }
        }
    }

    if(m_groupMenu)
        delete m_groupMenu;
    m_groupMenu = new QMenu(this);

    QAction *remarkAction = m_groupMenu->addAction("设置群备注");
    QAction *editNameAction = m_groupMenu->addAction("修改群名称");
    QAction *inviteMemberAction = m_groupMenu->addAction("邀请好友进群");
    QAction *addAdminAction = m_groupMenu->addAction("添加管理员");
    QAction *removeAdminAction = m_groupMenu->addAction("移除管理员");
    QAction *kickMemberAction = m_groupMenu->addAction("踢出群成员");
    QAction *transferAction = m_groupMenu->addAction("转让群聊");
    QAction *exitAction = m_groupMenu->addAction("退出群聊");

    connect(remarkAction, &QAction::triggered, this, &MainPage::onSetGroupRemark);
    connect(editNameAction, &QAction::triggered, this, &MainPage::onEditGroupName);
    connect(inviteMemberAction, &QAction::triggered, this, &MainPage::onInviteMember);
    connect(addAdminAction, &QAction::triggered, this, &MainPage::onAddAdmin);
    connect(removeAdminAction, &QAction::triggered, this, &MainPage::onRemoveAdmin);
    connect(kickMemberAction, &QAction::triggered, this, &MainPage::onKickMember);
    connect(transferAction, &QAction::triggered, this, &MainPage::onTransferGroup);
    connect(exitAction, &QAction::triggered, this, &MainPage::onExitGroup);

    editNameAction->setEnabled(isAdmin);
    addAdminAction->setEnabled(m_groupMenuIsOwner);
    removeAdminAction->setEnabled(m_groupMenuIsOwner);
    kickMemberAction->setEnabled(isAdmin);
    transferAction->setEnabled(m_groupMenuIsOwner);

    QPoint globalPos = ui->lwGroupList->mapToGlobal(ui->lwGroupList->visualItemRect(ui->lwGroupList->item(m_groupMenuGroupIndex)).center());
    m_groupMenu->exec(globalPos);
}

void MainPage::on_btnEditInfo_clicked()
{
    EditUserInfoPage::getObject()->show();
}

void MainPage::on_btnFindUser_clicked()
{
    FindUserPage::getObject()->show();
}

void MainPage::on_btnFriendReq_clicked()
{
    FriendReqListPage::getObject();
    NetContext::getObject()->userGetReqList();
    FriendReqListPage::getObject()->show();
}

void MainPage::on_btnChangePassword_clicked()
{
    UserChangePasswordPage::getObject()->show();
}

void MainPage::on_btnSend_clicked()
{
    if(ui->toolBox->currentIndex() == 0 &&
        ui->lwFriendList->currentRow() < 0)
        return;
    if(ui->toolBox->currentIndex() == 1 &&
        ui->lwGroupList->currentRow() < 0)
        return;
    QString data = ui->teSendData->toPlainText();
    if(data.isEmpty())
        return;
    QString time = QDateTime::currentDateTime()
                .toString("yyyy-MM-dd hh:mm:ss.zzz");
    if(ui->toolBox->currentIndex() == 0){
        QString username = ui->lwFriendList
            ->currentItem()->data(Qt::UserRole).toString();
        qint64 index = ChatHistoryManager::getObject()
                ->insertPrivateMessage(
                               LoginPage::getObject()->getUserName(),
                               username, MessageSendStatus::Sent, time, data);
        if(index > 0)
            NetContext::getObject()
                ->userSendChatMsg(username, data, time, index);
        appendSentBubble(time, data, m_avatarCache.value(LoginPage::getObject()->getUserName(), ""));
    }
    else if(ui->toolBox->currentIndex() == 1){
        int row = ui->lwGroupList->currentRow();
        if(row >= 0 && row < m_groupList.size()){
            int groupId = m_groupList.at(row).toObject()
                              .value("group_id").toInt();
            qint64 index = ChatHistoryManager::getObject()
                    ->insertGroupMessage(
                                   QString::number(groupId),
                                   LoginPage::getObject()->getUserName(),
                                   MessageSendStatus::Sent, time, data);
            if(index > 0)
            NetContext::getObject()
                ->groupSendChatMsg(groupId, data, time, index);
        }
        appendSentBubble(time, data, m_avatarCache.value(LoginPage::getObject()->getUserName(), ""));
    }
}

void MainPage::on_lwFriendList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if(current == nullptr)
        return;
    QString username = current->data(Qt::UserRole).toString();
    QJsonArray list = ChatHistoryManager::getObject()->queryPrivateMessages(username);
    ui->tbReadData->clear();
    for(int i = list.size()-1; i >= 0; i--){
        QJsonObject obj = list.at(i).toObject();
        QString from = obj["sender_username"].toString();
        QString to = obj["receiver_username"].toString();
        int status = obj["send_status"].toInt();
        QString time = obj["send_time"].toString();
        QString msg = obj["message_content"].toString();
        QString myAvatar = m_avatarCache.value(LoginPage::getObject()->getUserName(), "");
        QString fromAvatar = m_avatarCache.value(from, "");
        if(to == username)
            appendSentBubble(time, msg, myAvatar);
        else
            appendReceivedBubble(time, from, msg, fromAvatar);
    }
}

void MainPage::on_lwGroupList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    loadGroupHistory();
}

void MainPage::on_lwGroupList_customContextMenuRequested(const QPoint &pos)
{
    QListWidgetItem *item = ui->lwGroupList->itemAt(pos);
    if(item != nullptr){
        int index = ui->lwGroupList->row(item);
        showGroupManageMenu(index);
    }
}

void MainPage::on_btnCreateGroup_clicked()
{
    bool ok;
    QString groupName = QInputDialog::getText(
        this, "创建群聊", "请输入群名：",
        QLineEdit::Normal, "", &ok);
    if(ok && !groupName.isEmpty()){
        NetContext::getObject()->groupCreate(groupName);
    }
}

void MainPage::on_btnLogout_clicked()
{
    NetContext::getObject()->userLogout();
}

void MainPage::onEditGroupName()
{
    bool ok;
    QString newName = QInputDialog::getText(
        this, "修改群名称", "请输入新的群名：",
        QLineEdit::Normal, "", &ok);
    if(ok && !newName.isEmpty()){
        NetContext::getObject()->groupRename(m_currentGroupId, newName);
    }
}

void MainPage::onInviteMember()
{
    QStringList friendUsernames;
    for(int i = 0; i < m_friendList.size(); i++){
        friendUsernames.append(m_friendList.at(i).toObject().value("username").toString());
    }
    bool ok;
    QString username = QInputDialog::getItem(
        this, "邀请好友进群", "请选择要邀请的好友：",
        friendUsernames, 0, false, &ok);
    if(ok && !username.isEmpty()){
        NetContext::getObject()->groupAddMember(m_currentGroupId, username);
    }
}

void MainPage::onAddAdmin()
{
    NetContext::getObject()->groupGetMembers(m_currentGroupId);
    QStringList memberUsernames;
    for(int i = 0; i < m_groupMembers.size(); i++){
        QString username = m_groupMembers.at(i).toObject().value("username").toString();
        if(username != LoginPage::getObject()->getUserName()){
            memberUsernames.append(username);
        }
    }
    if(memberUsernames.isEmpty()){
        QMessageBox::information(this, "提示", "没有可添加为管理员的成员");
        return;
    }
    bool ok;
    QString username = QInputDialog::getItem(
        this, "添加管理员", "请选择要添加为管理员的成员：",
        memberUsernames, 0, false, &ok);
    if(ok && !username.isEmpty()){
        QStringList admins;
        admins.append(username);
        NetContext::getObject()->groupSetAdmins(m_currentGroupId, admins);
    }
}

void MainPage::onRemoveAdmin()
{
    NetContext::getObject()->groupGetAdmins(m_currentGroupId);
    QStringList adminUsernames;
    for(int i = 0; i < m_groupAdmins.size(); i++){
        QString username = m_groupAdmins.at(i).toObject().value("username").toString();
        if(username != LoginPage::getObject()->getUserName()){
            adminUsernames.append(username);
        }
    }
    if(adminUsernames.isEmpty()){
        QMessageBox::information(this, "提示", "没有可移除的管理员");
        return;
    }
    bool ok;
    QString username = QInputDialog::getItem(
        this, "移除管理员", "请选择要移除管理员权限的成员：",
        adminUsernames, 0, false, &ok);
    if(ok && !username.isEmpty()){
        QStringList emptyList;
        NetContext::getObject()->groupSetAdmins(m_currentGroupId, emptyList);
    }
}

void MainPage::onKickMember()
{
    NetContext::getObject()->groupGetMembers(m_currentGroupId);
    QStringList memberUsernames;
    for(int i = 0; i < m_groupMembers.size(); i++){
        QString username = m_groupMembers.at(i).toObject().value("username").toString();
        if(username != LoginPage::getObject()->getUserName()){
            memberUsernames.append(username);
        }
    }
    if(memberUsernames.isEmpty()){
        QMessageBox::information(this, "提示", "没有可踢出的成员");
        return;
    }
    bool ok;
    QString username = QInputDialog::getItem(
        this, "踢出群成员", "请选择要踢出的成员：",
        memberUsernames, 0, false, &ok);
    if(ok && !username.isEmpty()){
        if(QMessageBox::question(this, "确认", QString("确定要踢出 %1 吗？").arg(username),
                                 QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes){
            NetContext::getObject()->groupRemoveMember(m_currentGroupId, username);
        }
    }
}

void MainPage::onTransferGroup()
{
    NetContext::getObject()->groupGetMembers(m_currentGroupId);
    QStringList memberUsernames;
    for(int i = 0; i < m_groupMembers.size(); i++){
        QString username = m_groupMembers.at(i).toObject().value("username").toString();
        if(username != LoginPage::getObject()->getUserName()){
            memberUsernames.append(username);
        }
    }
    if(memberUsernames.isEmpty()){
        QMessageBox::information(this, "提示", "没有可转让的成员");
        return;
    }
    bool ok;
    QString username = QInputDialog::getItem(
        this, "转让群聊", "请选择要转让给的成员：",
        memberUsernames, 0, false, &ok);
    if(ok && !username.isEmpty()){
        if(QMessageBox::question(this, "确认", QString("确定要将群聊转让给 %1 吗？").arg(username),
                                 QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes){
            NetContext::getObject()->groupTransfer(m_currentGroupId, username);
        }
    }
}

void MainPage::onExitGroup()
{
    if(QMessageBox::question(this, "确认", "确定要退出群聊吗？",
                             QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes){
        NetContext::getObject()->groupExit(m_currentGroupId);
    }
}

QString MainPage::getAvatarHtml(const QString &avatar)
{
    if(avatar.isEmpty()){
        return "<img src='data:image/png;base64,' width='40' height='40' style='border-radius:5px;'>";
    }
    return QString("<img src='data:image/png;base64,%1' width='40' height='40' style='border-radius:5px;'>").arg(avatar);
}

void MainPage::appendSentBubble(const QString &time, const QString &msg, const QString &avatar)
{
    QString html = QString(
        "<table width='100%' cellpadding='0' cellspacing='0'><tr>"
        "<td width='20%'></td>"
        "<td align='right' style='vertical-align:middle;'>"
        "<div style='background-color:#95EC69; border-radius:10px; padding:8px 12px; display:inline-block; max-width:90%;'>"
        "<div style='font-size:10px; color:#666; margin-bottom:4px;'>%1</div>"
        "<div style='word-wrap:break-word;'>%2</div>"
        "</div>"
        "</td>"
        "<td width='50' style='vertical-align:bottom;'>%3</td>"
        "</tr></table>")
        .arg(time.toHtmlEscaped(), msg.toHtmlEscaped().replace("\n", "<br>"), getAvatarHtml(avatar));
    ui->tbReadData->append(html);
}

void MainPage::appendReceivedBubble(const QString &time, const QString &sender, const QString &msg, const QString &avatar)
{
    QString label = sender.isEmpty() ? time : QString("%1  %2").arg(sender, time);
    QString html = QString(
        "<table width='100%' cellpadding='0' cellspacing='0'><tr>"
        "<td width='50' style='vertical-align:bottom;'>%1</td>"
        "<td align='left' style='vertical-align:middle;'>"
        "<div style='background-color:#E0E0E0; border-radius:10px; padding:8px 12px; display:inline-block; max-width:90%;'>"
        "<div style='font-size:10px; color:#666; margin-bottom:4px;'>%2</div>"
        "<div style='word-wrap:break-word;'>%3</div>"
        "</div>"
        "</td>"
        "<td width='20%'></td>"
        "</tr></table>")
        .arg(getAvatarHtml(avatar), label.toHtmlEscaped(), msg.toHtmlEscaped().replace("\n", "<br>"));
    ui->tbReadData->append(html);
}

void MainPage::on_lwFriendList_customContextMenuRequested(const QPoint &pos)
{
    QListWidgetItem *item = ui->lwFriendList->itemAt(pos);
    if(item == nullptr)
        return;
    m_menuFriendUsername = item->data(Qt::UserRole).toString();
    if(m_menuFriendUsername.isEmpty())
        return;

    bool isBlocked = false;
    QString currentRemark;
    for(int i = 0; i < m_friendList.size(); i++){
        QJsonObject obj = m_friendList.at(i).toObject();
        if(obj.value("username").toString() == m_menuFriendUsername){
            isBlocked = obj.value("is_blocked").toInt() == 1;
            currentRemark = obj.value("remark").toString();
            break;
        }
    }

    if(m_friendMenu)
        delete m_friendMenu;
    m_friendMenu = new QMenu(this);

    QAction *remarkAction = m_friendMenu->addAction("设置备注");
    QAction *moveGroupAction = m_friendMenu->addAction("移动到分组");
    QAction *blockAction = isBlocked ? m_friendMenu->addAction("解除拉黑") : m_friendMenu->addAction("拉黑好友");

    connect(remarkAction, &QAction::triggered, this, &MainPage::onSetFriendRemark);
    connect(moveGroupAction, &QAction::triggered, this, &MainPage::onMoveFriendToGroup);
    if(isBlocked)
        connect(blockAction, &QAction::triggered, this, &MainPage::onUnblockFriend);
    else
        connect(blockAction, &QAction::triggered, this, &MainPage::onBlockFriend);

    m_friendMenu->exec(ui->lwFriendList->mapToGlobal(pos));
}

void MainPage::onSetFriendRemark()
{
    if(m_menuFriendUsername.isEmpty())
        return;
    QString currentRemark;
    for(int i = 0; i < m_friendList.size(); i++){
        QJsonObject obj = m_friendList.at(i).toObject();
        if(obj.value("username").toString() == m_menuFriendUsername){
            currentRemark = obj.value("remark").toString();
            break;
        }
    }
    bool ok;
    QString remark = QInputDialog::getText(
        this, "设置备注", "请输入备注名：",
        QLineEdit::Normal, currentRemark, &ok);
    if(ok){
        NetContext::getObject()->friendSetRemark(m_menuFriendUsername, remark);
    }
}

void MainPage::onBlockFriend()
{
    if(m_menuFriendUsername.isEmpty())
        return;
    if(QMessageBox::question(this, "确认拉黑",
        QString("确定要拉黑 %1 吗？拉黑后对方将无法给你发送消息。").arg(m_menuFriendUsername),
        QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes){
        NetContext::getObject()->friendBlock(m_menuFriendUsername);
    }
}

void MainPage::onUnblockFriend()
{
    if(m_menuFriendUsername.isEmpty())
        return;
    NetContext::getObject()->friendUnblock(m_menuFriendUsername);
}

void MainPage::onMoveFriendToGroup()
{
    if(m_menuFriendUsername.isEmpty())
        return;
    QStringList groupNames;
    QList<int> groupIds;
    groupNames.append("默认分组");
    groupIds.append(0);
    for(int i = 0; i < m_friendGroupList.size(); i++){
        QJsonObject obj = m_friendGroupList.at(i).toObject();
        groupNames.append(obj.value("group_name").toString());
        groupIds.append(obj.value("group_id").toInt());
    }
    groupNames.append("新建分组...");
    groupIds.append(-1);

    bool ok;
    QString selected = QInputDialog::getItem(
        this, "移动到分组", "请选择目标分组：",
        groupNames, 0, false, &ok);
    if(!ok || selected.isEmpty())
        return;

    int idx = groupNames.indexOf(selected);
    if(idx < 0)
        return;
    int groupId = groupIds.at(idx);

    if(groupId == -1){
        bool ok2;
        QString newName = QInputDialog::getText(
            this, "新建分组", "请输入分组名称：",
            QLineEdit::Normal, "", &ok2);
        if(ok2 && !newName.isEmpty()){
            NetContext::getObject()->friendGroupCreate(newName);
            m_pendingMoveFriend = m_menuFriendUsername;
        }
    } else if(groupId == 0){
        NetContext::getObject()->friendSetGroup(m_menuFriendUsername, 0);
    } else {
        NetContext::getObject()->friendSetGroup(m_menuFriendUsername, groupId);
    }
}

void MainPage::onCreateFriendGroup()
{
    bool ok;
    QString groupName = QInputDialog::getText(
        this, "创建分组", "请输入分组名称：",
        QLineEdit::Normal, "", &ok);
    if(ok && !groupName.isEmpty()){
        NetContext::getObject()->friendGroupCreate(groupName);
    }
}

void MainPage::onDeleteFriendGroup()
{
    if(m_menuFriendGroupId <= 0)
        return;
    if(QMessageBox::question(this, "删除分组",
        "确定要删除该分组吗？分组内的好友将移动到默认分组。",
        QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes){
        NetContext::getObject()->friendGroupDelete(m_menuFriendGroupId);
    }
}

void MainPage::onRenameFriendGroup()
{
    if(m_menuFriendGroupId <= 0)
        return;
    QString currentName;
    for(int i = 0; i < m_friendGroupList.size(); i++){
        QJsonObject obj = m_friendGroupList.at(i).toObject();
        if(obj.value("group_id").toInt() == m_menuFriendGroupId){
            currentName = obj.value("group_name").toString();
            break;
        }
    }
    bool ok;
    QString newName = QInputDialog::getText(
        this, "重命名分组", "请输入新的分组名称：",
        QLineEdit::Normal, currentName, &ok);
    if(ok && !newName.isEmpty()){
        NetContext::getObject()->friendGroupRename(m_menuFriendGroupId, newName);
    }
}

void MainPage::onSetGroupRemark()
{
    if(m_currentGroupId <= 0)
        return;
    bool ok;
    QString remark = QInputDialog::getText(
        this, "设置群备注", "请输入群备注名：",
        QLineEdit::Normal, "", &ok);
    if(ok){
        NetContext::getObject()->groupSetRemark(m_currentGroupId, remark);
    }
}

void MainPage::friendBlockReSlot(QString username, bool re)
{
    if(re){
        QMessageBox::information(this, "提示", "拉黑成功");
        NetContext::getObject()->userGetFriendList();
    } else {
        QMessageBox::warning(this, "提示", "拉黑失败");
    }
}

void MainPage::friendUnblockReSlot(QString username, bool re)
{
    if(re){
        QMessageBox::information(this, "提示", "已解除拉黑");
        NetContext::getObject()->userGetFriendList();
    } else {
        QMessageBox::warning(this, "提示", "操作失败");
    }
}

void MainPage::friendGroupCreateReSlot(bool re, int groupId, QString groupName)
{
    if(re){
        QMessageBox::information(this, "提示", "分组创建成功");
        NetContext::getObject()->friendGroupGetList();
        if(!m_pendingMoveFriend.isEmpty()){
            NetContext::getObject()->friendSetGroup(m_pendingMoveFriend, groupId);
            m_pendingMoveFriend.clear();
        }
    } else {
        QMessageBox::warning(this, "提示", "分组创建失败");
    }
}

void MainPage::friendGroupDeleteReSlot(int groupId, bool re)
{
    if(re){
        QMessageBox::information(this, "提示", "分组删除成功");
        NetContext::getObject()->friendGroupGetList();
    } else {
        QMessageBox::warning(this, "提示", "分组删除失败");
    }
}

void MainPage::friendGroupRenameReSlot(int groupId, bool re)
{
    if(re){
        QMessageBox::information(this, "提示", "重命名成功");
        NetContext::getObject()->friendGroupGetList();
    } else {
        QMessageBox::warning(this, "提示", "重命名失败");
    }
}

void MainPage::friendGroupListSlot(const QJsonArray &list)
{
    m_friendGroupList = list;
}

void MainPage::friendSetGroupReSlot(QString username, bool re)
{
    if(re){
        NetContext::getObject()->userGetFriendList();
    } else {
        QMessageBox::warning(this, "提示", "移动失败");
    }
}

void MainPage::friendSetRemarkReSlot(QString username, bool re)
{
    if(re){
        QMessageBox::information(this, "提示", "备注设置成功");
        NetContext::getObject()->userGetFriendList();
    } else {
        QMessageBox::warning(this, "提示", "备注设置失败");
    }
}

void MainPage::groupSetRemarkReSlot(int groupId, bool re)
{
    if(re){
        QMessageBox::information(this, "提示", "群备注设置成功");
        loadGroupList();
    } else {
        QMessageBox::warning(this, "提示", "群备注设置失败");
    }
}