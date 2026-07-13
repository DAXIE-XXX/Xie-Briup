#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>

class MyTcpSocket : public QTcpSocket
{
    friend class MyTcpServer;
    Q_OBJECT
protected:
    explicit MyTcpSocket(QObject *parent = nullptr);

protected slots:
    void readyReadSlot();

protected:
    void handleData(QByteArray &data);
    void handleFrame(const QByteArray &frame);
    void handleRegister(const QJsonObject &obj);
    void handleLogin(const QJsonObject &obj);
    void handleGetUserInfo();
    void handleSetUserInfo(const QJsonObject &obj);
    void handleSetAvatar(const QJsonObject &obj);
    void handleUserChangePassword(const QJsonObject &obj);
    void handleLogout();
    void handleFindUser(const QJsonObject &obj);
    void handleFriendAddRequest(const QJsonObject &obj);
    void handleGetFriendRequestList();
    void handleAcceptFriendRequest(const QJsonObject &obj);
    void handleGetFriendList();
    void handleChatUser(const QJsonObject &obj);
    void handleGroupCreate(const QJsonObject &obj);
    void handleGroupGetList();
    void handleGroupChatMsg(const QJsonObject &obj);
    void handleGroupAddMember(const QJsonObject &obj);
    void handleGroupRemoveMember(const QJsonObject &obj);
    void handleGroupGetMembers(const QJsonObject &obj);
    void handleGroupDissolve(const QJsonObject &obj);
    void handleGroupExit(const QJsonObject &obj);
    void handleGroupGetAdmins(const QJsonObject &obj);
    void handleGroupSetAdmins(const QJsonObject &obj);
    void handleGroupRename(const QJsonObject &obj);
    void handleGroupTransfer(const QJsonObject &obj);

    void handleFriendBlock(const QJsonObject &obj);
    void handleFriendUnblock(const QJsonObject &obj);
    void handleFriendGroupCreate(const QJsonObject &obj);
    void handleFriendGroupDelete(const QJsonObject &obj);
    void handleFriendGroupRename(const QJsonObject &obj);
    void handleFriendGroupGetList();
    void handleFriendSetGroup(const QJsonObject &obj);
    void handleFriendSetRemark(const QJsonObject &obj);
    void handleGroupSetRemark(const QJsonObject &obj);

    QString username;
    int userId = -1;
};

#endif // MYTCPSOCKET_H