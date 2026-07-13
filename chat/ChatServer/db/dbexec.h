#ifndef DBEXEC_H
#define DBEXEC_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>
#include <QStringList>
class QSqlDatabase;
class DBExec : public QObject
{
    Q_OBJECT
    friend class DBContext;
public:
    explicit DBExec(
        QString ip, quint16 port, QString username,
        QString password, QString dbName, QString conName,
        QObject *parent = nullptr);
    bool userInsert(QString username, QString password);
    bool userSelect(QString username, QString password);
    QJsonObject userGetInfo(int userId);
    int userGetId(QString username);
    bool userSetInfo(int userId, QString name, QString phone, QString email, QString info, QString birthday);
    bool userSetAvatar(int userId, const QByteArray &avatar);
    bool userChangePassword(QString username, QString password);
    QJsonArray userFind(QString name, int userId);
    bool friendCheck(int user_id1, int user_id2);
    bool userAddFriendRequest(int fromUserId, int toUserId, QString msg, QString time);
    QJsonArray userGetRequestList(int userId);
    bool userAcceptFriendRequest(int reqId);
    bool userRejectedFriendRequest(int reqId);
    QJsonArray userGetFriendList(int userId);
    QString userGetUsername(int userId);
    bool groupCreate(QString name, int adminUserId, int &groupId);
    QJsonArray groupGetList(int userId);
    bool groupAddMember(int groupId, int userId);
    bool groupRemoveMember(int groupId, int userId);
    QJsonArray groupGetMembers(int groupId);
    bool groupCheckMember(int groupId, int userId);
    QList<int> groupGetMemberIds(int groupId);
    bool groupDissolve(int groupId, int adminUserId);
    bool groupExit(int groupId, int userId);
    QString groupGetName(int groupId);
    QJsonArray groupGetAdmins(int groupId);
    bool groupIsAdmin(int groupId, int userId);
    bool groupIsOwner(int groupId, int userId);
    bool groupSetAdmins(int groupId, const QStringList &usernames);
    bool groupRename(int groupId, const QString &newName);
    bool groupTransfer(int groupId, int oldOwnerId, int newOwnerId);

    bool friendBlock(int userId, int friendId);
    bool friendUnblock(int userId, int friendId);
    bool friendIsBlocked(int userId, int friendId);

    bool friendGroupCreate(int userId, const QString &groupName, int &groupId);
    bool friendGroupDelete(int userId, int groupId);
    bool friendGroupRename(int userId, int groupId, const QString &newName);
    QJsonArray friendGroupGetList(int userId);
    bool friendGroupBelongsToUser(int userId, int groupId);
    bool friendSetGroup(int userId, int friendId, int groupId);

    bool friendSetRemark(int userId, int friendId, const QString &remark);
    QString friendGetRemark(int userId, int friendId);

    bool groupSetRemark(int userId, int groupId, const QString &remark);
    QString groupGetRemark(int userId, int groupId);

    QJsonArray userGetFriendListWithGroup(int userId);

protected:
    QSqlDatabase *db;
};

#endif // DBEXEC_H