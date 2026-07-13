#include "dbexec.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDate>

DBExec::DBExec(QString ip, quint16 port, QString username, QString password, QString dbName, QString conName, QObject *parent)
    : QObject{parent}
    , db{new QSqlDatabase{
            QSqlDatabase::addDatabase("QMYSQL", conName)}}
{
    db->setHostName(ip);
    db->setPort(port);
    db->setUserName(username);
    db->setPassword(password);
    db->setDatabaseName(dbName);
    db->setConnectOptions("MYSQL_OPT_CONNECT_TIMEOUT=3");
    db->open();
}

bool DBExec::userInsert(QString username, QString password)
{
    db->transaction();
    QSqlQuery query(*db);
    //向用户表添加数据
    QString cmd = QString("INSERT INTO chat_userinfo "
                          "(username, password) "
                          "VALUES('%1', '%2');")
                      .arg(username).arg(password);
    bool ok = query.exec(cmd);
    if(!ok){
        db->rollback();
        return false;
    }
    if(query.numRowsAffected() != 1){
        db->rollback();
        return false;
    }
    //第一步成功，查询id
    cmd = QString("SELECT id FROM chat_userinfo "
                  "WHERE username='%1';")
              .arg(username);
    ok = query.exec(cmd);
    if(!ok){
        db->rollback();
        return false;
    }
    if(!query.next()){
        db->rollback();
        return false;
    }
    int id = query.value("id").toInt();
    //向用户信息表添加数据
    cmd = QString("INSERT INTO chat_userinfo_exp (user_id) "
                  "VALUES(%1);")
              .arg(id);
    ok = query.exec(cmd);
    if(!ok){
        db->rollback();
        return false;
    }
    if(query.numRowsAffected() != 1){
        db->rollback();
        return false;
    }
    db->commit();
    return true;
}

bool DBExec::userSelect(QString username, QString password)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT * FROM chat_userinfo "
                          "WHERE username='%1' AND "
                          "password='%2';")
                      .arg(username).arg(password);
    bool ok = query.exec(cmd);
    if(!ok)
        return false;
    if(!query.next())
        return false;
    return true;
}

QJsonObject DBExec::userGetInfo(int userId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT * FROM chat_userinfo_exp "
                          "WHERE user_id=%1;")
                      .arg(userId);
    bool ok = query.exec(cmd);
    if(!ok)
        return QJsonObject();
    if(!query.next())
        return QJsonObject();
    QJsonObject obj;
    obj.insert("name", query.value("name").toString());
    obj.insert("phone", query.value("phone").toString());
    obj.insert("email", query.value("email").toString());
    obj.insert("info", query.value("info").toString());
    obj.insert("birthday",
               query.value("birthday").toDate()
               .toString("yyyy-MM-dd"));
    QByteArray avatar = query.value("avatar").toByteArray();
    if(!avatar.isEmpty())
        obj.insert("avatar", QString::fromUtf8(avatar.toBase64()));
    return obj;
}

int DBExec::userGetId(QString username)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT id FROM chat_userinfo "
                  "WHERE username='%1';")
              .arg(username);
    bool ok = query.exec(cmd);
    if(!ok){
        return -1;
    }
    if(!query.next()){
        return -1;
    }
    int id = query.value("id").toInt();
    return id;
}

bool DBExec::userSetInfo(int userId, QString name, QString phone, QString email, QString info, QString birthday)
{
    QSqlQuery query(*db);
    query.prepare("UPDATE chat_userinfo_exp SET name=:name, phone=:phone, email=:email, info=:info, birthday=:birthday WHERE user_id=:user_id;");
    query.bindValue(":name", name);
    query.bindValue(":phone", phone);
    query.bindValue(":email", email);
    query.bindValue(":info", info);
    query.bindValue(":birthday", birthday);
    query.bindValue(":user_id", userId);
    bool ok = query.exec();
    return ok;
}

bool DBExec::userSetAvatar(int userId, const QByteArray &avatar)
{
    QSqlQuery query(*db);
    query.prepare("UPDATE chat_userinfo_exp SET avatar=:avatar WHERE user_id=:user_id;");
    query.bindValue(":avatar", avatar);
    query.bindValue(":user_id", userId);
    bool ok = query.exec();
    return ok;
}

bool DBExec::userChangePassword(QString username, QString password)
{
    QSqlQuery query(*db);
    QString cmd = QString("UPDATE chat_userinfo "
                          "SET password = '%1' "
                          "WHERE username = '%2';")
        .arg(password).arg(username);
    bool ok = query.exec(cmd);
    if(!ok){
        return false;
    }
    if(query.numRowsAffected() != 1){
        return false;
    }
    return true;
}

QJsonArray DBExec::userFind(QString name, int userId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT cu.id, cu.username, cue.name "
                          "FROM chat_userinfo cu,"
                          "chat_userinfo_exp cue "
                          "WHERE cu.id = cue.user_id AND "
                          "cu.id != %1 AND "
                          "(cu.username LIKE '%%2%' "
                          "OR cue.name LIKE '%%2%') AND "
                          "NOT EXISTS (SELECT 1 FROM chat_friend cf "
                          "WHERE (cf.user_id1=%1 AND cf.user_id2=cu.id) "
                          "OR (cf.user_id1=cu.id AND cf.user_id2=%1));")
        .arg(userId).arg(name);
    QJsonArray list;
    bool ok = query.exec(cmd);
    if(!ok)
        return list;
    while(query.next()){
        QJsonObject obj;
        obj.insert("id",
                   query.value("id").toInt());
        obj.insert("username",
                   query.value("username").toString());
        obj.insert("name",
                   query.value("name").toString());
        list.append(obj);
    }
    return list;
}

bool DBExec::friendCheck(int user_id1, int user_id2)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT id FROM chat_friend "
                          "WHERE (user_id1=%1 AND user_id2=%2) "
                          "OR (user_id1=%2 AND user_id2=%1);")
        .arg(user_id1).arg(user_id2);
    bool ok = query.exec(cmd);
    if(!ok)
        return false;
    if(!query.next())
        return false;
    return true;
}

bool DBExec::userAddFriendRequest(int fromUserId, int toUserId, QString msg, QString time)
{
    QSqlQuery query(*db);
    QString cmd = QString("INSERT INTO chat_request_friend "
                          "(request_user_id, to_user_id,"
                          "request_msg, request_time) "
                          "VALUES(%1, %2, '%3', '%4');")
        .arg(fromUserId).arg(toUserId)
        .arg(msg).arg(time);
    bool ok = query.exec(cmd);
    if(!ok)
        return false;
    return query.numRowsAffected() == 1;
}

QJsonArray DBExec::userGetRequestList(int userId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT cu.username,cue.name,crf.request_msg,"
                          "crf.id,crf.request_time FROM "
                          "chat_request_friend crf,"
                          "chat_userinfo cu,chat_userinfo_exp cue "
                          "WHERE to_user_id=%1 AND "
                          "crf.request_user_id = cu.id AND "
                          "cue.user_id = cu.id AND "
                          "state='pending';")
        .arg(userId);
    QJsonArray list;
    bool ok = query.exec(cmd);
    if(!ok)
        return list;
    while(query.next()){//name	username	req_msg	req_id
        QJsonObject obj;
        obj.insert("username",
                   query.value("username").toString());
        obj.insert("name",
                   query.value("name").toString());
        obj.insert("req_msg",
                   query.value("request_msg").toString());
        obj.insert("req_time",
                   query.value("request_time").toDateTime()
                       .toString("yyyy-MM-dd hh:mm:ss.zzz"));
        obj.insert("req_id",
                   query.value("id").toInt());
        list.append(obj);
    }
    return list;
}

bool DBExec::userAcceptFriendRequest(int reqId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT request_user_id, to_user_id "
                          "FROM chat_request_friend "
                          "WHERE id=%1;")
        .arg(reqId);
    bool ok = query.exec(cmd);
    if(!ok)
        return false;
    if(!query.next())
        return false;
    db->transaction();
    int rui = query.value("request_user_id").toInt();
    int tui = query.value("to_user_id").toInt();
    cmd = QString("UPDATE chat_request_friend "
                  "SET state='approved' WHERE "
                  "request_user_id=%1 AND "
                  "to_user_id=%2 AND "
                  "state='pending';").arg(rui).arg(tui);
    ok = query.exec(cmd);
    if(!ok){
        db->rollback();
        return false;
    }
    if(query.numRowsAffected() != 1){
        db->rollback();
        return false;
    }
    cmd = QString("INSERT INTO chat_friend "
                  "(user_id1,user_id2) "
                  "VALUES(%1, %2);")
              .arg(rui).arg(tui);
    ok = query.exec(cmd);
    if(!ok){
        db->rollback();
        return false;
    }
    if(query.numRowsAffected() != 1){
        db->rollback();
        return false;
    }
    db->commit();
    return true;
}

bool DBExec::userRejectedFriendRequest(int reqId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT request_user_id, to_user_id "
                          "FROM chat_request_friend "
                          "WHERE id=%1;")
                      .arg(reqId);
    bool ok = query.exec(cmd);
    if(!ok)
        return false;
    if(!query.next())
        return false;
    int rui = query.value("request_user_id").toInt();
    int tui = query.value("to_user_id").toInt();
    cmd = QString("UPDATE chat_request_friend "
                  "SET state='rejected' WHERE "
                  "request_user_id=%1 AND "
                  "to_user_id=%2 AND "
                  "state='pending';").arg(rui).arg(tui);
    ok = query.exec(cmd);
    if(!ok)
        return false;
    if(query.numRowsAffected() != 1)
        return false;
    return true;
}

QJsonArray DBExec::userGetFriendList(int userId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT cu.username, cue.name,"
                          "cue.phone,cue.email,cue.birthday,"
                          "cue.info, cue.avatar FROM "
                          "chat_friend cf,"
                          "chat_userinfo cu,"
                          "chat_userinfo_exp cue "
                          "WHERE user_id1=%1 AND "
                          "cu.id = cue.user_id AND "
                          "cu.id = cf.user_id2;")
                      .arg(userId);
    QJsonArray list;
    bool ok = query.exec(cmd);
    if(!ok)
        return list;
    while(query.next()){
        QJsonObject obj;
        obj.insert("username",
                   query.value("username").toString());
        obj.insert("name",
                   query.value("name").toString());
        obj.insert("phone",
                   query.value("phone").toString());
        obj.insert("email",
                   query.value("email").toString());
        obj.insert("birthday",
                   query.value("birthday").toString());
        obj.insert("info",
                   query.value("info").toString());
        QByteArray avatarData = query.value("avatar").toByteArray();
        QString avatarBase64 = avatarData.isEmpty() ? "" : QString::fromLatin1(avatarData.toBase64());
        obj.insert("avatar", avatarBase64);
        list.append(obj);
    }
    cmd = QString("SELECT cu.username, cue.name,"
                  "cue.phone,cue.email,cue.birthday,"
                  "cue.info, cue.avatar FROM "
                  "chat_friend cf,"
                  "chat_userinfo cu,"
                  "chat_userinfo_exp cue "
                  "WHERE user_id2=%1 AND "
                  "cu.id = cue.user_id AND "
                  "cu.id = cf.user_id1;")
              .arg(userId);
    ok = query.exec(cmd);
    if(!ok)
        return list;
    while(query.next()){
        QJsonObject obj;
        obj.insert("username",
                   query.value("username").toString());
        obj.insert("name",
                   query.value("name").toString());
        obj.insert("phone",
                   query.value("phone").toString());
        obj.insert("email",
                   query.value("email").toString());
        obj.insert("birthday",
                   query.value("birthday").toString());
        obj.insert("info",
                   query.value("info").toString());
        QByteArray avatarData = query.value("avatar").toByteArray();
        QString avatarBase64 = avatarData.isEmpty() ? "" : QString::fromLatin1(avatarData.toBase64());
        obj.insert("avatar", avatarBase64);
        list.append(obj);
    }
    return list;
}

QString DBExec::userGetUsername(int userId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT username FROM chat_userinfo "
                          "WHERE id=%1;")
                      .arg(userId);
    bool ok = query.exec(cmd);
    if(!ok)
        return QString();
    if(!query.next())
        return QString();
    return query.value("username").toString();
}

bool DBExec::groupCreate(QString name, int adminUserId, int &groupId)
{
    db->transaction();
    QSqlQuery query(*db);
    QString cmd = QString("INSERT INTO chat_groupinfo "
                          "(name, admin_user_id) "
                          "VALUES('%1', %2);")
                      .arg(name).arg(adminUserId);
    bool ok = query.exec(cmd);
    if(!ok){
        db->rollback();
        return false;
    }
    if(query.numRowsAffected() != 1){
        db->rollback();
        return false;
    }
    groupId = query.lastInsertId().toInt();
    cmd = QString("INSERT INTO chat_group_user "
                  "(user_id, group_id) "
                  "VALUES(%1, %2);")
              .arg(adminUserId).arg(groupId);
    ok = query.exec(cmd);
    if(!ok){
        db->rollback();
        return false;
    }
    if(query.numRowsAffected() != 1){
        db->rollback();
        return false;
    }
    db->commit();
    return true;
}

QJsonArray DBExec::groupGetList(int userId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT cgi.id, cgi.name, "
                          "cgi.admin_user_id, cu.username AS admin_username, "
                          "cgu.group_remark "
                          "FROM chat_groupinfo cgi, "
                          "chat_group_user cgu, "
                          "chat_userinfo cu "
                          "WHERE cgu.user_id=%1 AND "
                          "cgu.group_id = cgi.id AND "
                          "cu.id = cgi.admin_user_id;")
                      .arg(userId);
    QJsonArray list;
    bool ok = query.exec(cmd);
    if(!ok)
        return list;
    while(query.next()){
        QJsonObject obj;
        obj.insert("group_id",
                   query.value("id").toInt());
        obj.insert("group_name",
                   query.value("name").toString());
        obj.insert("admin_user_id",
                   query.value("admin_user_id").toInt());
        obj.insert("admin_username",
                   query.value("admin_username").toString());
        obj.insert("group_remark",
                   query.value("group_remark").toString());
        list.append(obj);
    }
    return list;
}

bool DBExec::groupCheckMember(int groupId, int userId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT id FROM chat_group_user "
                          "WHERE user_id=%1 AND group_id=%2;")
                      .arg(userId).arg(groupId);
    bool ok = query.exec(cmd);
    if(!ok)
        return false;
    if(!query.next())
        return false;
    return true;
}

QList<int> DBExec::groupGetMemberIds(int groupId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT user_id FROM chat_group_user "
                          "WHERE group_id=%1;")
                      .arg(groupId);
    QList<int> ids;
    bool ok = query.exec(cmd);
    if(!ok)
        return ids;
    while(query.next()){
        ids.append(query.value("user_id").toInt());
    }
    return ids;
}

bool DBExec::groupAddMember(int groupId, int userId)
{
    QSqlQuery query(*db);
    QString cmd = QString("INSERT INTO chat_group_user "
                          "(user_id, group_id) "
                          "VALUES(%1, %2);")
                      .arg(userId).arg(groupId);
    bool ok = query.exec(cmd);
    if(!ok)
        return false;
    return query.numRowsAffected() == 1;
}

bool DBExec::groupRemoveMember(int groupId, int userId)
{
    QSqlQuery query(*db);
    QString cmd = QString("DELETE FROM chat_group_user "
                          "WHERE user_id=%1 AND group_id=%2;")
                      .arg(userId).arg(groupId);
    bool ok = query.exec(cmd);
    if(!ok)
        return false;
    return query.numRowsAffected() == 1;
}

QJsonArray DBExec::groupGetMembers(int groupId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT cu.username, cue.name, cue.avatar, "
                          "cgu.group_user_name "
                          "FROM chat_group_user cgu, "
                          "chat_userinfo cu, "
                          "chat_userinfo_exp cue "
                          "WHERE cgu.group_id=%1 AND "
                          "cu.id = cgu.user_id AND "
                          "cue.user_id = cgu.user_id;")
                      .arg(groupId);
    QJsonArray list;
    bool ok = query.exec(cmd);
    if(!ok)
        return list;
    while(query.next()){
        QJsonObject obj;
        obj.insert("username",
                   query.value("username").toString());
        obj.insert("name",
                   query.value("name").toString());
        obj.insert("group_user_name",
                   query.value("group_user_name").toString());
        QByteArray avatarData = query.value("avatar").toByteArray();
        QString avatarBase64 = avatarData.isEmpty() ? "" : QString::fromLatin1(avatarData.toBase64());
        obj.insert("avatar", avatarBase64);
        list.append(obj);
    }
    return list;
}

bool DBExec::groupDissolve(int groupId, int adminUserId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT admin_user_id FROM chat_groupinfo "
                          "WHERE id=%1;")
                      .arg(groupId);
    bool ok = query.exec(cmd);
    if(!ok)
        return false;
    if(!query.next())
        return false;
    if(query.value("admin_user_id").toInt() != adminUserId)
        return false;
    cmd = QString("DELETE FROM chat_groupinfo WHERE id=%1;")
              .arg(groupId);
    ok = query.exec(cmd);
    if(!ok)
        return false;
    return true;
}

bool DBExec::groupExit(int groupId, int userId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT admin_user_id FROM chat_groupinfo "
                          "WHERE id=%1;")
                      .arg(groupId);
    bool ok = query.exec(cmd);
    if(ok && query.next()){
        if(query.value("admin_user_id").toInt() == userId)
            return false;
    }
    cmd = QString("DELETE FROM chat_group_user "
                  "WHERE user_id=%1 AND group_id=%2;")
              .arg(userId).arg(groupId);
    ok = query.exec(cmd);
    if(!ok)
        return false;
    return query.numRowsAffected() == 1;
}

QString DBExec::groupGetName(int groupId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT name FROM chat_groupinfo "
                          "WHERE id=%1;")
                      .arg(groupId);
    bool ok = query.exec(cmd);
    if(!ok)
        return QString();
    if(!query.next())
        return QString();
    return query.value("name").toString();
}

QJsonArray DBExec::groupGetAdmins(int groupId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT cu.username, cue.name, cue.avatar "
                          "FROM chat_group_admin_user cga, "
                          "chat_userinfo cu, "
                          "chat_userinfo_exp cue "
                          "WHERE cga.group_id=%1 AND "
                          "cu.id = cga.admin_user_id AND "
                          "cue.user_id = cga.admin_user_id;")
                      .arg(groupId);
    QJsonArray list;
    bool ok = query.exec(cmd);
    if(!ok)
        return list;
    while(query.next()){
        QJsonObject obj;
        obj.insert("username",
                   query.value("username").toString());
        obj.insert("name",
                   query.value("name").toString());
        QByteArray avatarData = query.value("avatar").toByteArray();
        QString avatarBase64 = avatarData.isEmpty() ? "" : QString::fromLatin1(avatarData.toBase64());
        obj.insert("avatar", avatarBase64);
        list.append(obj);
    }
    return list;
}

bool DBExec::groupIsAdmin(int groupId, int userId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT admin_user_id FROM chat_groupinfo "
                          "WHERE id=%1;")
                      .arg(groupId);
    bool ok = query.exec(cmd);
    if(ok && query.next()){
        if(query.value("admin_user_id").toInt() == userId)
            return true;
    }
    cmd = QString("SELECT id FROM chat_group_admin_user "
                  "WHERE group_id=%1 AND admin_user_id=%2;")
              .arg(groupId).arg(userId);
    ok = query.exec(cmd);
    if(ok && query.next())
        return true;
    return false;
}

bool DBExec::groupIsOwner(int groupId, int userId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT admin_user_id FROM chat_groupinfo "
                          "WHERE id=%1;")
                      .arg(groupId);
    bool ok = query.exec(cmd);
    if(!ok || !query.next())
        return false;
    return query.value("admin_user_id").toInt() == userId;
}

bool DBExec::groupSetAdmins(int groupId, const QStringList &usernames)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT admin_user_id FROM chat_groupinfo "
                          "WHERE id=%1;")
                      .arg(groupId);
    bool ok = query.exec(cmd);
    if(!ok)
        return false;
    if(!query.next())
        return false;
    int ownerUserId = query.value("admin_user_id").toInt();

    db->transaction();
    cmd = QString("DELETE FROM chat_group_admin_user "
                  "WHERE group_id=%1;")
              .arg(groupId);
    ok = query.exec(cmd);
    if(!ok){
        db->rollback();
        return false;
    }

    cmd = QString("INSERT INTO chat_group_admin_user "
                  "(admin_user_id, group_id) "
                  "VALUES(%1, %2);")
              .arg(ownerUserId).arg(groupId);
    ok = query.exec(cmd);
    if(!ok){
        db->rollback();
        return false;
    }

    for(const QString &uname : usernames){
        int uid = userGetId(uname);
        if(uid <= 0 || uid == ownerUserId)
            continue;
        bool isMember = groupCheckMember(groupId, uid);
        if(!isMember)
            continue;
        cmd = QString("INSERT INTO chat_group_admin_user "
                      "(admin_user_id, group_id) "
                      "VALUES(%1, %2);")
                  .arg(uid).arg(groupId);
        ok = query.exec(cmd);
        if(!ok){
            db->rollback();
            return false;
        }
    }

    db->commit();
    return true;
}

bool DBExec::groupRename(int groupId, const QString &newName)
{
    QSqlQuery query(*db);
    QString cmd = QString("UPDATE chat_groupinfo SET name='%1' WHERE id=%2;")
              .arg(newName).arg(groupId);
    bool ok = query.exec(cmd);
    if(!ok)
        return false;
    return query.numRowsAffected() == 1;
}

bool DBExec::groupTransfer(int groupId, int oldOwnerId, int newOwnerId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT admin_user_id FROM chat_groupinfo "
                          "WHERE id=%1;")
                      .arg(groupId);
    bool ok = query.exec(cmd);
    if(!ok)
        return false;
    if(!query.next())
        return false;
    if(query.value("admin_user_id").toInt() != oldOwnerId)
        return false;
    bool isMember = groupCheckMember(groupId, newOwnerId);
    if(!isMember)
        return false;
    db->transaction();
    cmd = QString("UPDATE chat_groupinfo SET admin_user_id=%1 WHERE id=%2;")
              .arg(newOwnerId).arg(groupId);
    ok = query.exec(cmd);
    if(!ok){
        db->rollback();
        return false;
    }
    cmd = QString("DELETE FROM chat_group_admin_user "
                  "WHERE group_id=%1;")
              .arg(groupId);
    ok = query.exec(cmd);
    if(!ok){
        db->rollback();
        return false;
    }
    cmd = QString("INSERT INTO chat_group_admin_user "
                  "(admin_user_id, group_id) "
                  "VALUES(%1, %2);")
              .arg(newOwnerId).arg(groupId);
    ok = query.exec(cmd);
    if(!ok){
        db->rollback();
        return false;
    }
    db->commit();
    return true;
}

bool DBExec::friendBlock(int userId, int friendId)
{
    QSqlQuery query(*db);
    QString cmd = QString("UPDATE chat_friend SET is_blocked=1 "
                          "WHERE (user_id1=%1 AND user_id2=%2) "
                          "OR (user_id1=%2 AND user_id2=%1);")
        .arg(userId).arg(friendId);
    bool ok = query.exec(cmd);
    if(!ok)
        return false;
    return query.numRowsAffected() > 0;
}

bool DBExec::friendUnblock(int userId, int friendId)
{
    QSqlQuery query(*db);
    QString cmd = QString("UPDATE chat_friend SET is_blocked=0 "
                          "WHERE (user_id1=%1 AND user_id2=%2) "
                          "OR (user_id1=%2 AND user_id2=%1);")
        .arg(userId).arg(friendId);
    bool ok = query.exec(cmd);
    if(!ok)
        return false;
    return query.numRowsAffected() > 0;
}

bool DBExec::friendIsBlocked(int userId, int friendId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT is_blocked FROM chat_friend "
                          "WHERE (user_id1=%1 AND user_id2=%2) "
                          "OR (user_id1=%2 AND user_id2=%1);")
        .arg(userId).arg(friendId);
    bool ok = query.exec(cmd);
    if(!ok)
        return false;
    if(!query.next())
        return false;
    return query.value("is_blocked").toInt() == 1;
}

bool DBExec::friendGroupCreate(int userId, const QString &groupName, int &groupId)
{
    QSqlQuery query(*db);
    QString cmd = QString("INSERT INTO chat_friend_group (user_id, group_name) "
                          "VALUES(%1, '%2');")
        .arg(userId).arg(groupName);
    bool ok = query.exec(cmd);
    if(!ok)
        return false;
    if(query.numRowsAffected() != 1)
        return false;
    groupId = query.lastInsertId().toInt();
    return true;
}

bool DBExec::friendGroupDelete(int userId, int groupId)
{
    db->transaction();
    QSqlQuery query(*db);
    QString cmd = QString("UPDATE chat_friend SET group_id=NULL "
                          "WHERE group_id=%1 AND "
                          "(user_id1=%2 OR user_id2=%2);")
        .arg(groupId).arg(userId);
    bool ok = query.exec(cmd);
    if(!ok){
        db->rollback();
        return false;
    }
    cmd = QString("DELETE FROM chat_friend_group "
                  "WHERE id=%1 AND user_id=%2;")
              .arg(groupId).arg(userId);
    ok = query.exec(cmd);
    if(!ok){
        db->rollback();
        return false;
    }
    if(query.numRowsAffected() != 1){
        db->rollback();
        return false;
    }
    db->commit();
    return true;
}

bool DBExec::friendGroupRename(int userId, int groupId, const QString &newName)
{
    QSqlQuery query(*db);
    QString cmd = QString("UPDATE chat_friend_group SET group_name='%1' "
                          "WHERE id=%2 AND user_id=%3;")
        .arg(newName).arg(groupId).arg(userId);
    bool ok = query.exec(cmd);
    if(!ok)
        return false;
    return query.numRowsAffected() == 1;
}

QJsonArray DBExec::friendGroupGetList(int userId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT id, group_name, sort_order "
                          "FROM chat_friend_group "
                          "WHERE user_id=%1 "
                          "ORDER BY sort_order ASC, id ASC;")
                      .arg(userId);
    QJsonArray list;
    bool ok = query.exec(cmd);
    if(!ok)
        return list;
    while(query.next()){
        QJsonObject obj;
        obj.insert("group_id", query.value("id").toInt());
        obj.insert("group_name", query.value("group_name").toString());
        obj.insert("sort_order", query.value("sort_order").toInt());
        list.append(obj);
    }
    return list;
}

bool DBExec::friendGroupBelongsToUser(int userId, int groupId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT id FROM chat_friend_group "
                          "WHERE id=%1 AND user_id=%2;")
        .arg(groupId).arg(userId);
    bool ok = query.exec(cmd);
    if(!ok)
        return false;
    return query.next();
}

bool DBExec::friendSetGroup(int userId, int friendId, int groupId)
{
    QSqlQuery query(*db);
    QString groupIdStr = (groupId == 0) ? "NULL" : QString::number(groupId);
    QString cmd = QString("UPDATE chat_friend SET group_id=%1 "
                          "WHERE (user_id1=%2 AND user_id2=%3) "
                          "OR (user_id1=%3 AND user_id2=%2);")
        .arg(groupIdStr).arg(userId).arg(friendId);
    bool ok = query.exec(cmd);
    if(!ok)
        return false;
    return query.numRowsAffected() > 0;
}

bool DBExec::friendSetRemark(int userId, int friendId, const QString &remark)
{
    QSqlQuery query(*db);
    QString cmd = QString("UPDATE chat_friend SET remark='%1' "
                          "WHERE (user_id1=%2 AND user_id2=%3) "
                          "OR (user_id1=%3 AND user_id2=%2);")
        .arg(remark).arg(userId).arg(friendId);
    bool ok = query.exec(cmd);
    if(!ok)
        return false;
    return query.numRowsAffected() > 0;
}

QString DBExec::friendGetRemark(int userId, int friendId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT remark FROM chat_friend "
                          "WHERE (user_id1=%1 AND user_id2=%2) "
                          "OR (user_id1=%2 AND user_id2=%1);")
        .arg(userId).arg(friendId);
    bool ok = query.exec(cmd);
    if(!ok)
        return QString();
    if(!query.next())
        return QString();
    return query.value("remark").toString();
}

bool DBExec::groupSetRemark(int userId, int groupId, const QString &remark)
{
    QSqlQuery query(*db);
    QString cmd = QString("UPDATE chat_group_user SET group_remark='%1' "
                          "WHERE user_id=%2 AND group_id=%3;")
        .arg(remark).arg(userId).arg(groupId);
    bool ok = query.exec(cmd);
    if(!ok)
        return false;
    return query.numRowsAffected() == 1;
}

QString DBExec::groupGetRemark(int userId, int groupId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT group_remark FROM chat_group_user "
                          "WHERE user_id=%1 AND group_id=%2;")
                      .arg(userId).arg(groupId);
    bool ok = query.exec(cmd);
    if(!ok)
        return QString();
    if(!query.next())
        return QString();
    return query.value("group_remark").toString();
}

QJsonArray DBExec::userGetFriendListWithGroup(int userId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT cu.username, cue.name,"
                          "cue.phone,cue.email,cue.birthday,"
                          "cue.info, cue.avatar, cf.remark, cf.group_id, cf.is_blocked FROM "
                          "chat_friend cf,"
                          "chat_userinfo cu,"
                          "chat_userinfo_exp cue "
                          "WHERE cf.user_id1=%1 AND "
                          "cu.id = cue.user_id AND "
                          "cu.id = cf.user_id2;")
                      .arg(userId);
    QJsonArray list;
    bool ok = query.exec(cmd);
    if(!ok)
        return list;
    while(query.next()){
        QJsonObject obj;
        obj.insert("username",
                   query.value("username").toString());
        obj.insert("name",
                   query.value("name").toString());
        obj.insert("phone",
                   query.value("phone").toString());
        obj.insert("email",
                   query.value("email").toString());
        obj.insert("birthday",
                   query.value("birthday").toString());
        obj.insert("info",
                   query.value("info").toString());
        QByteArray avatarData = query.value("avatar").toByteArray();
        QString avatarBase64 = avatarData.isEmpty() ? "" : QString::fromLatin1(avatarData.toBase64());
        obj.insert("avatar", avatarBase64);
        obj.insert("remark",
                   query.value("remark").toString());
        obj.insert("group_id",
                   query.value("group_id").toInt());
        obj.insert("is_blocked",
                   query.value("is_blocked").toInt());
        list.append(obj);
    }
    cmd = QString("SELECT cu.username, cue.name,"
                  "cue.phone,cue.email,cue.birthday,"
                  "cue.info, cue.avatar, cf.remark, cf.group_id, cf.is_blocked FROM "
                  "chat_friend cf,"
                  "chat_userinfo cu,"
                  "chat_userinfo_exp cue "
                  "WHERE cf.user_id2=%1 AND "
                  "cu.id = cue.user_id AND "
                  "cu.id = cf.user_id1;")
              .arg(userId);
    ok = query.exec(cmd);
    if(!ok)
        return list;
    while(query.next()){
        QJsonObject obj;
        obj.insert("username",
                   query.value("username").toString());
        obj.insert("name",
                   query.value("name").toString());
        obj.insert("phone",
                   query.value("phone").toString());
        obj.insert("email",
                   query.value("email").toString());
        obj.insert("birthday",
                   query.value("birthday").toString());
        obj.insert("info",
                   query.value("info").toString());
        QByteArray avatarData = query.value("avatar").toByteArray();
        QString avatarBase64 = avatarData.isEmpty() ? "" : QString::fromLatin1(avatarData.toBase64());
        obj.insert("avatar", avatarBase64);
        obj.insert("remark",
                   query.value("remark").toString());
        obj.insert("group_id",
                   query.value("group_id").toInt());
        obj.insert("is_blocked",
                   query.value("is_blocked").toInt());
        list.append(obj);
    }
    return list;
}








