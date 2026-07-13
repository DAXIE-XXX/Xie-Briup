#ifndef NETCONTEXT_H
#define NETCONTEXT_H

#include <QObject>
#include <QJsonArray>
class QTcpSocket;
class NetContext : public QObject
{
    Q_OBJECT
protected:
    explicit NetContext(QObject *parent = nullptr);

public:
    static NetContext *getObject();
    void connectToServer(QString ip, quint16 port);
    void userRegister(QString username, QString password);
    void userLogin(QString username, QString password);
    void userGetInfo();
    void userSetInfo(QString name, QString phone, QString email,
                     QString info, QString birthday);
    void userChangePassword(QString oldPd, QString newPd);
    void userSetAvatar(const QByteArray &avatar);
    void userFind(QString name);
    void userAddFriendRequest(QString username, QString msg);
    void userGetReqList();
    void userAcceptRequest(int reqId);
    void userGetFriendList();
    void userSendChatMsg(QString username, QString msg,
                         QString time, int index);
    void userLogout();
    void groupCreate(QString groupName);
    void groupGetList();
    void groupSendChatMsg(int groupId, QString msg,
                          QString time, int index);
    void groupAddMember(int groupId, QString username);
    void groupRemoveMember(int groupId, QString username);
    void groupGetMembers(int groupId);
    void groupSetAdmins(int groupId, const QStringList &usernames);
    void groupDissolve(int groupId);
    void groupExit(int groupId);
    void groupGetAdmins(int groupId);
    void groupRename(int groupId, QString newName);
    void groupTransfer(int groupId, QString username);

    void friendBlock(QString username);
    void friendUnblock(QString username);
    void friendGroupCreate(QString groupName);
    void friendGroupDelete(int groupId);
    void friendGroupRename(int groupId, QString newName);
    void friendGroupGetList();
    void friendSetGroup(QString username, int groupId);
    void friendSetRemark(QString username, QString remark);
    void groupSetRemark(int groupId, QString remark);

signals:
    void registerRe(QString username, bool re);
    void loginRe(QString username, bool re);
    void userInfo(QString name, QString phone, QString email,
                  QString info, QString birthday, QString avatar);
    void userSetInfoRe(bool re);
    void userChangePdRe(bool re);
    void avatarUploadRe(bool re);
    void userFindList(const QJsonArray &list);
    void userAddFriendRequestRe(bool re);
    void userRequestList(const QJsonArray &list);
    void userAcceptReqRe(bool re);
    void userFriendList(const QJsonArray &list);
    void userSendChatRe(bool re, int index);
    void logoutRe(bool re);
    void groupCreateRe(bool re, int groupId);
    void groupList(const QJsonArray &list);
    void groupSendChatRe(bool re, int index);
    void groupAddMemberRe(bool re);
    void groupRemoveMemberRe(bool re);
    void groupMembersList(const QJsonArray &list);
    void groupSetAdminsRe(bool re);
    void groupDissolveRe(bool re);
    void groupExitRe(bool re);
    void groupAdminsList(const QJsonArray &list);
    void groupRenameRe(bool re);
    void groupTransferRe(bool re);

    void friendBlockRe(QString username, bool re);
    void friendUnblockRe(QString username, bool re);
    void friendGroupCreateRe(bool re, int groupId, QString groupName);
    void friendGroupDeleteRe(int groupId, bool re);
    void friendGroupRenameRe(int groupId, bool re);
    void friendGroupList(const QJsonArray &list);
    void friendSetGroupRe(QString username, bool re);
    void friendSetRemarkRe(QString username, bool re);
    void groupSetRemarkRe(int groupId, bool re);

protected slots:
    void connectedSlot();
    void disconnectedSlot();
    void readyReadSlot();

protected:
    void timerEvent(QTimerEvent *e);
    void handleData(QByteArray &data);
    void handleFrame(const QByteArray &frame);
    void handleRegisterRe(const QJsonObject &obj);
    void handleLoginRe(const QJsonObject &obj);
    void handleUserInfo(const QJsonObject &obj);
    void handleUserSetInfoRe(const QJsonObject &obj);
    void handleUserChangePdRe(const QJsonObject &obj);
    void handleAvatarUploadRe(const QJsonObject &obj);
    void handleUserFindList(const QJsonObject &obj);
    void handleUserAddFriendRequestRe(const QJsonObject &obj);
    void handleUserGetRequestList(const QJsonObject &obj);
    void handleUserAcceptReqRe(const QJsonObject &obj);
    void handleUserGetFriendList(const QJsonObject &obj);
    void handleUserSendChatRe(const QJsonObject &obj);
    void handleLogoutRe(const QJsonObject &obj);
    void handleGroupCreateRe(const QJsonObject &obj);
    void handleGroupGetList(const QJsonObject &obj);
    void handleGroupSendChatRe(const QJsonObject &obj);
    void handleGroupAddMemberRe(const QJsonObject &obj);
    void handleGroupRemoveMemberRe(const QJsonObject &obj);
    void handleGroupGetMembers(const QJsonObject &obj);
    void handleGroupSetAdminsRe(const QJsonObject &obj);
    void handleGroupDissolveRe(const QJsonObject &obj);
    void handleGroupExitRe(const QJsonObject &obj);
    void handleGroupGetAdmins(const QJsonObject &obj);
    void handleGroupRenameRe(const QJsonObject &obj);
    void handleGroupTransferRe(const QJsonObject &obj);
    void handleFriendBlockRe(const QJsonObject &obj);
    void handleFriendUnblockRe(const QJsonObject &obj);
    void handleFriendGroupCreateRe(const QJsonObject &obj);
    void handleFriendGroupDeleteRe(const QJsonObject &obj);
    void handleFriendGroupRenameRe(const QJsonObject &obj);
    void handleFriendGroupList(const QJsonObject &obj);
    void handleFriendSetGroupRe(const QJsonObject &obj);
    void handleFriendSetRemarkRe(const QJsonObject &obj);
    void handleGroupSetRemarkRe(const QJsonObject &obj);
    static NetContext *obj;
    QString ip;
    quint16 port;
    QTcpSocket *socket;
    int reConId = -1;
};

#endif // NETCONTEXT_H