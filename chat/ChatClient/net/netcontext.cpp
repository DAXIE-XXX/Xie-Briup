#include "netcontext.h"
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimerEvent>
#include <QJsonArray>

NetContext *NetContext::obj = nullptr;
NetContext::NetContext(QObject *parent)
    : QObject{parent}
    , socket{new QTcpSocket(this)}
{
    connect(socket, &QTcpSocket::connected,
            this, &NetContext::connectedSlot);
    connect(socket, &QTcpSocket::disconnected,
            this, &NetContext::disconnectedSlot);
    connect(socket, &QTcpSocket::readyRead,
            this, &NetContext::readyReadSlot);
}

NetContext *NetContext::getObject()
{
    if(obj == nullptr)
        obj = new NetContext;
    return obj;
}

void NetContext::connectToServer(QString ip, quint16 port)
{
    if(socket->state() == QTcpSocket::ConnectedState)
        socket->disconnectFromHost();
    socket->connectToHost(QHostAddress(ip), port);
    this->ip = ip;
    this->port = port;
    reConId = startTimer(10000);
}

void NetContext::userRegister(QString username, QString password)
{
    QJsonObject obj;
    obj.insert("type", 1001);
    obj.insert("username", username);
    obj.insert("password", password);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::userLogin(QString username, QString password)
{
    QJsonObject obj;
    obj.insert("type", 1002);
    obj.insert("username", username);
    obj.insert("password", password);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::userGetInfo()
{
    QJsonObject obj;
    obj.insert("type", 1003);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::userSetInfo(QString name, QString phone, QString email, QString info, QString birthday)
{
    QJsonObject obj;
    obj.insert("type", 1004);
    obj.insert("name", name);
    obj.insert("phone", phone);
    obj.insert("email", email);
    obj.insert("info", info);
    obj.insert("birthday", birthday);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::userChangePassword(QString oldPd, QString newPd)
{
    QJsonObject obj;
    obj.insert("type", 1005);
    obj.insert("old_password", oldPd);
    obj.insert("new_password", newPd);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::userSetAvatar(const QByteArray &avatar)
{
    QJsonObject obj;
    obj.insert("type", 1007);
    obj.insert("avatar", QString::fromUtf8(avatar.toBase64()));
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::userFind(QString name)
{
    QJsonObject obj;
    obj.insert("type", 2001);
    obj.insert("name", name);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::userAddFriendRequest(QString username, QString msg)
{
    QJsonObject obj;
    obj.insert("type", 2002);
    obj.insert("username", username);
    obj.insert("req_msg", msg);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::userGetReqList()
{
    QJsonObject obj;
    obj.insert("type", 2003);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::userAcceptRequest(int reqId)
{
    QJsonObject obj;
    obj.insert("type", 2004);
    obj.insert("req_id", reqId);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::userGetFriendList()
{
    QJsonObject obj;
    obj.insert("type", 2005);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::userSendChatMsg(QString username, QString msg, QString time, int index)
{
    QJsonObject obj;
    obj.insert("type", 3001);
    obj.insert("to_username", username);
    obj.insert("msg", msg);
    obj.insert("time", time);
    obj.insert("index", index);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::userLogout()
{
    QJsonObject obj;
    obj.insert("type", 1006);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::groupCreate(QString groupName)
{
    QJsonObject obj;
    obj.insert("type", 4001);
    obj.insert("group_name", groupName);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::groupGetList()
{
    QJsonObject obj;
    obj.insert("type", 4002);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::groupSendChatMsg(int groupId, QString msg, QString time, int index)
{
    QJsonObject obj;
    obj.insert("type", 3002);
    obj.insert("group_id", groupId);
    obj.insert("msg", msg);
    obj.insert("time", time);
    obj.insert("index", index);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::groupAddMember(int groupId, QString username)
{
    QJsonObject obj;
    obj.insert("type", 4003);
    obj.insert("group_id", groupId);
    obj.insert("username", username);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::groupRemoveMember(int groupId, QString username)
{
    QJsonObject obj;
    obj.insert("type", 4004);
    obj.insert("group_id", groupId);
    obj.insert("username", username);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::groupGetMembers(int groupId)
{
    QJsonObject obj;
    obj.insert("type", 4005);
    obj.insert("group_id", groupId);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::groupSetAdmins(int groupId, const QStringList &usernames)
{
    QJsonObject obj;
    obj.insert("type", 4006);
    obj.insert("group_id", groupId);
    QJsonArray arr;
    for(const QString &name : usernames)
        arr.append(name);
    obj.insert("usernames", arr);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::groupDissolve(int groupId)
{
    QJsonObject obj;
    obj.insert("type", 4007);
    obj.insert("group_id", groupId);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::groupExit(int groupId)
{
    QJsonObject obj;
    obj.insert("type", 4008);
    obj.insert("group_id", groupId);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::groupGetAdmins(int groupId)
{
    QJsonObject obj;
    obj.insert("type", 4009);
    obj.insert("group_id", groupId);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::groupRename(int groupId, QString newName)
{
    QJsonObject obj;
    obj.insert("type", 4010);
    obj.insert("group_id", groupId);
    obj.insert("new_name", newName);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::groupTransfer(int groupId, QString username)
{
    QJsonObject obj;
    obj.insert("type", 4011);
    obj.insert("group_id", groupId);
    obj.insert("username", username);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::friendBlock(QString username)
{
    QJsonObject obj;
    obj.insert("type", 2007);
    obj.insert("username", username);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::friendUnblock(QString username)
{
    QJsonObject obj;
    obj.insert("type", 2008);
    obj.insert("username", username);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::friendGroupCreate(QString groupName)
{
    QJsonObject obj;
    obj.insert("type", 2009);
    obj.insert("group_name", groupName);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::friendGroupDelete(int groupId)
{
    QJsonObject obj;
    obj.insert("type", 2010);
    obj.insert("group_id", groupId);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::friendGroupRename(int groupId, QString newName)
{
    QJsonObject obj;
    obj.insert("type", 2011);
    obj.insert("group_id", groupId);
    obj.insert("new_name", newName);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::friendGroupGetList()
{
    QJsonObject obj;
    obj.insert("type", 2012);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::friendSetGroup(QString username, int groupId)
{
    QJsonObject obj;
    obj.insert("type", 2013);
    obj.insert("username", username);
    obj.insert("group_id", groupId);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::friendSetRemark(QString username, QString remark)
{
    QJsonObject obj;
    obj.insert("type", 2014);
    obj.insert("username", username);
    obj.insert("remark", remark);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::groupSetRemark(int groupId, QString remark)
{
    QJsonObject obj;
    obj.insert("type", 4012);
    obj.insert("group_id", groupId);
    obj.insert("remark", remark);
    socket->write(QJsonDocument(obj).toJson());
}

void NetContext::connectedSlot()
{
    if(reConId != -1)
        killTimer(reConId);
}

void NetContext::disconnectedSlot()
{
    reConId = startTimer(5000);
}

void NetContext::readyReadSlot()
{
    QByteArray data = socket->readAll();
    handleData(data);
}

void NetContext::timerEvent(QTimerEvent *e)
{
    if(e->timerId() == reConId){
        socket->connectToHost(QHostAddress(ip), port);
    }
}

void NetContext::handleData(QByteArray &data)
{
    int count = 0;
    for(int i = 0; i < data.length(); i++){
        if(data.at(i) == '{')
            count++;
        else if(data.at(i) == '}'){
            count--;
            if(count == 0){
                QByteArray frame = data.mid(0, i+1);
                handleFrame(frame);
                if(frame.length() == data.length())
                    return;
                data = data.mid(i+1);
                i = -1;
            }
        }
    }
}

void NetContext::handleFrame(const QByteArray &frame)
{
    QJsonObject rf = QJsonDocument::fromJson(frame).object();
    int type = rf.value("type").toInt();
    if(type == 1001)
        handleRegisterRe(rf);
    else if(type == 1002)
        handleLoginRe(rf);
    else if(type == 1003)
        handleUserInfo(rf);
    else if(type == 1004)
        handleUserSetInfoRe(rf);
    else if(type == 1005)
        handleUserChangePdRe(rf);
    else if(type == 1007)
        handleAvatarUploadRe(rf);
    else if(type == 2001)
        handleUserFindList(rf);
    else if(type == 2002)
        handleUserAddFriendRequestRe(rf);
    else if(type == 2003)
        handleUserGetRequestList(rf);
    else if(type == 2004)
        handleUserAcceptReqRe(rf);
    else if(type == 2005)
        handleUserGetFriendList(rf);
    else if(type == 3001)
        handleUserSendChatRe(rf);
    else if(type == 1006)
        handleLogoutRe(rf);
    else if(type == 4001)
        handleGroupCreateRe(rf);
    else if(type == 4002)
        handleGroupGetList(rf);
    else if(type == 3002)
        handleGroupSendChatRe(rf);
    else if(type == 4003)
        handleGroupAddMemberRe(rf);
    else if(type == 4004)
        handleGroupRemoveMemberRe(rf);
    else if(type == 4005)
        handleGroupGetMembers(rf);
    else if(type == 4006)
        handleGroupSetAdminsRe(rf);
    else if(type == 4007)
        handleGroupDissolveRe(rf);
    else if(type == 4008)
        handleGroupExitRe(rf);
    else if(type == 4009)
        handleGroupGetAdmins(rf);
    else if(type == 4010)
        handleGroupRenameRe(rf);
    else if(type == 4011)
        handleGroupTransferRe(rf);
    else if(type == 2007)
        handleFriendBlockRe(rf);
    else if(type == 2008)
        handleFriendUnblockRe(rf);
    else if(type == 2009)
        handleFriendGroupCreateRe(rf);
    else if(type == 2010)
        handleFriendGroupDeleteRe(rf);
    else if(type == 2011)
        handleFriendGroupRenameRe(rf);
    else if(type == 2012)
        handleFriendGroupList(rf);
    else if(type == 2013)
        handleFriendSetGroupRe(rf);
    else if(type == 2014)
        handleFriendSetRemarkRe(rf);
    else if(type == 4012)
        handleGroupSetRemarkRe(rf);
}

void NetContext::handleRegisterRe(const QJsonObject &obj)
{
    QString username = obj.value("username").toString();
    bool re = obj.value("re").toBool();
    emit registerRe(username, re);
}

void NetContext::handleLoginRe(const QJsonObject &obj)
{
    QString username = obj.value("username").toString();
    bool re = obj.value("re").toBool();
    emit loginRe(username, re);
}

void NetContext::handleUserInfo(const QJsonObject &obj)
{
    QString name = obj.value("name").toString();
    QString phone = obj.value("phone").toString();
    QString email = obj.value("email").toString();
    QString info = obj.value("info").toString();
    QString birthday = obj.value("birthday").toString();
    QString avatar = obj.value("avatar").toString();
    emit userInfo(name, phone, email, info, birthday, avatar);
}

void NetContext::handleUserSetInfoRe(const QJsonObject &obj)
{
    bool re = obj.value("re").toBool();
    emit userSetInfoRe(re);
}

void NetContext::handleUserChangePdRe(const QJsonObject &obj)
{
    bool re = obj.value("re").toBool();
    emit userChangePdRe(re);
}

void NetContext::handleUserFindList(const QJsonObject &obj)
{
    QJsonArray list = obj.value("list").toArray();
    emit userFindList(list);
}

void NetContext::handleUserAddFriendRequestRe(const QJsonObject &obj)
{
    bool re = obj.value("re").toBool();
    emit userAddFriendRequestRe(re);
}

void NetContext::handleUserGetRequestList(const QJsonObject &obj)
{
    QJsonArray list = obj.value("list").toArray();
    emit userRequestList(list);
}

void NetContext::handleUserAcceptReqRe(const QJsonObject &obj)
{
    bool re = obj.value("re").toBool();
    emit userAcceptReqRe(re);
}

void NetContext::handleUserGetFriendList(const QJsonObject &obj)
{
    QJsonArray list = obj.value("list").toArray();
    emit userFriendList(list);
}

void NetContext::handleUserSendChatRe(const QJsonObject &obj)
{
    bool re = obj.value("re").toBool();
    int index = obj.value("index").toInt();
    emit userSendChatRe(re, index);
}

void NetContext::handleLogoutRe(const QJsonObject &obj)
{
    bool re = obj.value("re").toBool();
    emit logoutRe(re);
}

void NetContext::handleGroupCreateRe(const QJsonObject &obj)
{
    bool re = obj.value("re").toBool();
    int groupId = obj.value("group_id").toInt();
    emit groupCreateRe(re, groupId);
}

void NetContext::handleGroupGetList(const QJsonObject &obj)
{
    QJsonArray list = obj.value("list").toArray();
    emit groupList(list);
}

void NetContext::handleGroupSendChatRe(const QJsonObject &obj)
{
    bool re = obj.value("re").toBool();
    int index = obj.value("index").toInt();
    emit groupSendChatRe(re, index);
}

void NetContext::handleGroupAddMemberRe(const QJsonObject &obj)
{
    bool re = obj.value("re").toBool();
    emit groupAddMemberRe(re);
}

void NetContext::handleGroupRemoveMemberRe(const QJsonObject &obj)
{
    bool re = obj.value("re").toBool();
    emit groupRemoveMemberRe(re);
}

void NetContext::handleGroupGetMembers(const QJsonObject &obj)
{
    QJsonArray list = obj.value("list").toArray();
    emit groupMembersList(list);
}

void NetContext::handleGroupSetAdminsRe(const QJsonObject &obj)
{
    bool re = obj.value("re").toBool();
    emit groupSetAdminsRe(re);
}

void NetContext::handleGroupDissolveRe(const QJsonObject &obj)
{
    bool re = obj.value("re").toBool();
    emit groupDissolveRe(re);
}

void NetContext::handleGroupExitRe(const QJsonObject &obj)
{
    bool re = obj.value("re").toBool();
    emit groupExitRe(re);
}

void NetContext::handleGroupGetAdmins(const QJsonObject &obj)
{
    QJsonArray list = obj.value("list").toArray();
    emit groupAdminsList(list);
}

void NetContext::handleGroupRenameRe(const QJsonObject &obj)
{
    bool re = obj.value("re").toBool();
    emit groupRenameRe(re);
}

void NetContext::handleGroupTransferRe(const QJsonObject &obj)
{
    bool re = obj.value("re").toBool();
    emit groupTransferRe(re);
}

void NetContext::handleAvatarUploadRe(const QJsonObject &obj)
{
    bool re = obj.value("re").toBool();
    emit avatarUploadRe(re);
}

void NetContext::handleFriendBlockRe(const QJsonObject &obj)
{
    QString username = obj.value("username").toString();
    bool re = obj.value("re").toBool();
    emit friendBlockRe(username, re);
}

void NetContext::handleFriendUnblockRe(const QJsonObject &obj)
{
    QString username = obj.value("username").toString();
    bool re = obj.value("re").toBool();
    emit friendUnblockRe(username, re);
}

void NetContext::handleFriendGroupCreateRe(const QJsonObject &obj)
{
    bool re = obj.value("re").toBool();
    int groupId = obj.value("group_id").toInt();
    QString groupName = obj.value("group_name").toString();
    emit friendGroupCreateRe(re, groupId, groupName);
}

void NetContext::handleFriendGroupDeleteRe(const QJsonObject &obj)
{
    int groupId = obj.value("group_id").toInt();
    bool re = obj.value("re").toBool();
    emit friendGroupDeleteRe(groupId, re);
}

void NetContext::handleFriendGroupRenameRe(const QJsonObject &obj)
{
    int groupId = obj.value("group_id").toInt();
    bool re = obj.value("re").toBool();
    emit friendGroupRenameRe(groupId, re);
}

void NetContext::handleFriendGroupList(const QJsonObject &obj)
{
    QJsonArray list = obj.value("list").toArray();
    emit friendGroupList(list);
}

void NetContext::handleFriendSetGroupRe(const QJsonObject &obj)
{
    QString username = obj.value("username").toString();
    bool re = obj.value("re").toBool();
    emit friendSetGroupRe(username, re);
}

void NetContext::handleFriendSetRemarkRe(const QJsonObject &obj)
{
    QString username = obj.value("username").toString();
    bool re = obj.value("re").toBool();
    emit friendSetRemarkRe(username, re);
}

void NetContext::handleGroupSetRemarkRe(const QJsonObject &obj)
{
    int groupId = obj.value("group_id").toInt();
    bool re = obj.value("re").toBool();
    emit groupSetRemarkRe(groupId, re);
}