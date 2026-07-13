#include "dbexec.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDate>
#include <QList>
#include <QStringList>

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
    QString cmd = QString("UPDATE chat_userinfo_exp SET name='%1',"
                          "phone='%2',email='%3',info='%4',"
                          "birthday='%5' WHERE user_id=%6;")
                      .arg(name).arg(phone).arg(email).arg(info)
                      .arg(birthday).arg(userId);
    bool ok = query.exec(cmd);
    if(!ok){
        return false;
    }
    if(query.numRowsAffected() != 1){
        return false;
    }
    return true;
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

QJsonArray DBExec::userFind(QString name, int excludeUserId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT cu.username, cue.name "
                          "FROM chat_userinfo cu,"
                          "chat_userinfo_exp cue "
                          "WHERE cu.id = cue.user_id AND "
                          "cu.id != %2 AND "
                          "(cu.username LIKE '%%1%' "
                          "OR cue.name LIKE '%%1%') "
                          "AND cu.id NOT IN ("
                          "SELECT user_id2 FROM chat_friend WHERE user_id1=%2 "
                          "UNION "
                          "SELECT user_id1 FROM chat_friend WHERE user_id2=%2"
                          ");")
                      .arg(name).arg(excludeUserId);
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
                  "SET state=1 WHERE "
                  "request_user_id=%1 AND "
                  "to_user_id=%2 AND "
                  "state='rejected';").arg(rui).arg(tui);
    ok = query.exec(cmd);
    if(!ok)
        return false;
    if(query.numRowsAffected() != 1)
        return false;
    return true;
}

int DBExec::friendRequestGetFromUserId(int reqId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT request_user_id "
                          "FROM chat_request_friend "
                          "WHERE id=%1;")
                      .arg(reqId);
    bool ok = query.exec(cmd);
    if(!ok)
        return -1;
    if(!query.next())
        return -1;
    return query.value("request_user_id").toInt();
}

QJsonArray DBExec::userGetFriendList(int userId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT cu.username, cue.name,"
                          "cue.phone,cue.email,cue.birthday,"
                          "cue.info,"
                          "cf.user_2_groupname AS grp,"
                          "cf.user_2_name AS remark,"
                          "cf.user_2_state AS state "
                          "FROM chat_friend cf,"
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
                   query.value("birthday").toDate()
                       .toString("yyyy-MM-dd"));
        obj.insert("info",
                   query.value("info").toString());
        obj.insert("group_name",
                   query.value("grp").toString());
        obj.insert("remark",
                   query.value("remark").toString());
        obj.insert("state",
                   query.value("state").toString());
        list.append(obj);
    }
    cmd = QString("SELECT cu.username, cue.name,"
                  "cue.phone,cue.email,cue.birthday,"
                  "cue.info,"
                  "cf.user_1_groupname AS grp,"
                  "cf.user_1_name AS remark,"
                  "cf.user_1_state AS state "
                  "FROM chat_friend cf,"
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
                   query.value("birthday").toDate()
                       .toString("yyyy-MM-dd"));
        obj.insert("info",
                   query.value("info").toString());
        obj.insert("group_name",
                   query.value("grp").toString());
        obj.insert("remark",
                   query.value("remark").toString());
        obj.insert("state",
                   query.value("state").toString());
        list.append(obj);
    }
    return list;
}

QJsonArray DBExec::friendGetGroups(int userId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT DISTINCT user_2_groupname AS grp "
                          "FROM chat_friend "
                          "WHERE user_id1=%1 AND "
                          "user_2_groupname IS NOT NULL AND "
                          "user_2_groupname != '' "
                          "UNION "
                          "SELECT DISTINCT user_1_groupname AS grp "
                          "FROM chat_friend "
                          "WHERE user_id2=%1 AND "
                          "user_1_groupname IS NOT NULL AND "
                          "user_1_groupname != '';")
                      .arg(userId);
    QJsonArray list;
    bool ok = query.exec(cmd);
    if(!ok)
        return list;
    while(query.next()){
        list.append(query.value("grp").toString());
    }
    return list;
}

bool DBExec::friendSetGroup(int userId, int friendId, const QString &groupName)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT id, user_id1 FROM chat_friend "
                          "WHERE (user_id1=%1 AND user_id2=%2) "
                          "OR (user_id1=%2 AND user_id2=%1);")
                      .arg(userId).arg(friendId);
    bool ok = query.exec(cmd);
    if(!ok || !query.next())
        return false;
    bool isFirst = (query.value("user_id1").toInt() == userId);
    QString field = isFirst ? "user_2_groupname" : "user_1_groupname";
    int id = query.value("id").toInt();
    cmd = QString("UPDATE chat_friend SET %1='%2' WHERE id=%3;")
              .arg(field).arg(groupName).arg(id);
    ok = query.exec(cmd);
    if(!ok)
        return false;
    return query.numRowsAffected() == 1;
}

bool DBExec::friendSetRemark(int userId, int friendId, const QString &remark)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT id, user_id1 FROM chat_friend "
                          "WHERE (user_id1=%1 AND user_id2=%2) "
                          "OR (user_id1=%2 AND user_id2=%1);")
                      .arg(userId).arg(friendId);
    bool ok = query.exec(cmd);
    if(!ok || !query.next())
        return false;
    bool isFirst = (query.value("user_id1").toInt() == userId);
    QString field = isFirst ? "user_2_name" : "user_1_name";
    int id = query.value("id").toInt();
    cmd = QString("UPDATE chat_friend SET %1='%2' WHERE id=%3;")
              .arg(field).arg(remark).arg(id);
    ok = query.exec(cmd);
    if(!ok)
        return false;
    return query.numRowsAffected() == 1;
}

QJsonArray DBExec::friendGetListByGroup(int userId, const QString &groupName)
{
    QSqlQuery query(*db);
    QString groupCondition;
    if(groupName.isEmpty()){
        groupCondition = "(user_2_groupname IS NULL OR user_2_groupname = '')";
    } else {
        groupCondition = QString("user_2_groupname='%1'").arg(groupName);
    }
    QString cmd = QString("SELECT cu.username, cue.name,"
                          "cue.phone,cue.email,cue.birthday,"
                          "cue.info,"
                          "cf.user_2_groupname AS grp,"
                          "cf.user_2_name AS remark,"
                          "cf.user_2_state AS state "
                          "FROM chat_friend cf,"
                          "chat_userinfo cu,"
                          "chat_userinfo_exp cue "
                          "WHERE cf.user_id1=%1 AND "
                          "cu.id = cf.user_id2 AND "
                          "cu.id = cue.user_id AND %2;")
                      .arg(userId).arg(groupCondition);
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
                   query.value("birthday").toDate()
                       .toString("yyyy-MM-dd"));
        obj.insert("info",
                   query.value("info").toString());
        obj.insert("group_name",
                   query.value("grp").toString());
        obj.insert("remark",
                   query.value("remark").toString());
        obj.insert("state",
                   query.value("state").toString());
        list.append(obj);
    }

    if(groupName.isEmpty()){
        groupCondition = "(user_1_groupname IS NULL OR user_1_groupname = '')";
    } else {
        groupCondition = QString("user_1_groupname='%1'").arg(groupName);
    }
    cmd = QString("SELECT cu.username, cue.name,"
                  "cue.phone,cue.email,cue.birthday,"
                  "cue.info,"
                  "cf.user_1_groupname AS grp,"
                  "cf.user_1_name AS remark,"
                  "cf.user_1_state AS state "
                  "FROM chat_friend cf,"
                  "chat_userinfo cu,"
                  "chat_userinfo_exp cue "
                  "WHERE cf.user_id2=%1 AND "
                  "cu.id = cf.user_id1 AND "
                  "cu.id = cue.user_id AND %2;")
              .arg(userId).arg(groupCondition);
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
                   query.value("birthday").toDate()
                       .toString("yyyy-MM-dd"));
        obj.insert("info",
                   query.value("info").toString());
        obj.insert("group_name",
                   query.value("grp").toString());
        obj.insert("remark",
                   query.value("remark").toString());
        obj.insert("state",
                   query.value("state").toString());
        list.append(obj);
    }
    return list;
}

bool DBExec::friendBlock(int userId, int friendId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT id, user_id1 FROM chat_friend "
                          "WHERE (user_id1=%1 AND user_id2=%2) "
                          "OR (user_id1=%2 AND user_id2=%1);")
                      .arg(userId).arg(friendId);
    bool ok = query.exec(cmd);
    if(!ok || !query.next())
        return false;
    bool isFirst = (query.value("user_id1").toInt() == userId);
    QString field = isFirst ? "user_2_state" : "user_1_state";
    int id = query.value("id").toInt();
    cmd = QString("UPDATE chat_friend SET %1='blocked' WHERE id=%2;")
              .arg(field).arg(id);
    ok = query.exec(cmd);
    if(!ok)
        return false;
    return query.numRowsAffected() == 1;
}

bool DBExec::friendUnblock(int userId, int friendId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT id, user_id1 FROM chat_friend "
                          "WHERE (user_id1=%1 AND user_id2=%2) "
                          "OR (user_id1=%2 AND user_id2=%1);")
                      .arg(userId).arg(friendId);
    bool ok = query.exec(cmd);
    if(!ok || !query.next())
        return false;
    bool isFirst = (query.value("user_id1").toInt() == userId);
    QString field = isFirst ? "user_2_state" : "user_1_state";
    int id = query.value("id").toInt();
    cmd = QString("UPDATE chat_friend SET %1='normal' WHERE id=%2;")
              .arg(field).arg(id);
    ok = query.exec(cmd);
    if(!ok)
        return false;
    return query.numRowsAffected() == 1;
}

bool DBExec::friendIsBlockedBy(int senderId, int receiverId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT user_id1, user_1_state, user_2_state "
                          "FROM chat_friend "
                          "WHERE (user_id1=%1 AND user_id2=%2) "
                          "OR (user_id1=%2 AND user_id2=%1);")
                      .arg(senderId).arg(receiverId);
    bool ok = query.exec(cmd);
    if(!ok || !query.next())
        return false;
    bool receiverIsFirst = (query.value("user_id1").toInt() == receiverId);
    QString state = receiverIsFirst
                        ? query.value("user_1_state").toString()
                        : query.value("user_2_state").toString();
    return state == "blocked";
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
                          "cgi.admin_user_id, cu.username AS admin_username "
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
        list.append(obj);
    }
    return list;
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
    QString cmd = QString("SELECT cu.username, cue.name, "
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

QJsonArray DBExec::groupGetAdmins(int groupId)
{
    QSqlQuery query(*db);
    QString cmd = QString("SELECT cu.username, cue.name "
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

// ==================== 传感器数据管理 ====================

    bool DBExec::sensorInsertCo2(int value, const QString &time)
{
    QSqlQuery query(*db);
    QString cmd = QString("INSERT INTO Co2 (value, time) VALUES(%1, '%2');")
                      .arg(value).arg(time);
    bool ok = query.exec(cmd);
    if(!ok)
        return false;
    return query.numRowsAffected() == 1;
}

bool DBExec::sensorInsertTemperature(double value, const QString &time)
{
    QSqlQuery query(*db);
    QString cmd = QString("INSERT INTO Temperature (value, time) VALUES(%1, '%2');")
                      .arg(value, 0, 'f', 2).arg(time);
    bool ok = query.exec(cmd);
    if(!ok)
        return false;
    return query.numRowsAffected() == 1;
}

bool DBExec::sensorInsertHumidity(double value, const QString &time)
{
    QSqlQuery query(*db);
    QString cmd = QString("INSERT INTO Humidity (value, time) VALUES(%1, '%2');")
                      .arg(value, 0, 'f', 2).arg(time);
    bool ok = query.exec(cmd);
    if(!ok)
        return false;
    return query.numRowsAffected() == 1;
}

bool DBExec::sensorInsertPM25(int value, const QString &time)
{
    QSqlQuery query(*db);
    QString cmd = QString("INSERT INTO PM25 (value, time) VALUES(%1, '%2');")
                      .arg(value).arg(time);
    bool ok = query.exec(cmd);
    if(!ok)
        return false;
    return query.numRowsAffected() == 1;
}

bool DBExec::sensorInsertLight(int value, const QString &time)
{
    QSqlQuery query(*db);
    QString cmd = QString("INSERT INTO Light (value, time) VALUES(%1, '%2');")
                      .arg(value).arg(time);
    bool ok = query.exec(cmd);
    if(!ok)
        return false;
    return query.numRowsAffected() == 1;
}

bool DBExec::sensorInsertSmoke(bool value, const QString &time)
{
    QSqlQuery query(*db);
    QString cmd = QString("INSERT INTO Smoke (value, time) VALUES(%1, '%2');")
                      .arg(value ? 1 : 0).arg(time);
    bool ok = query.exec(cmd);
    if(!ok)
        return false;
    return query.numRowsAffected() == 1;
}

bool DBExec::sensorInsertMethane(bool value, const QString &time)
{
    QSqlQuery query(*db);
    QString cmd = QString("INSERT INTO Methane (value, time) VALUES(%1, '%2');")
                      .arg(value ? 1 : 0).arg(time);
    bool ok = query.exec(cmd);
    if(!ok)
        return false;
    return query.numRowsAffected() == 1;
}

bool DBExec::sensorInsertFire(bool value, const QString &time)
{
    QSqlQuery query(*db);
    QString cmd = QString("INSERT INTO Fire (value, time) VALUES(%1, '%2');")
                      .arg(value ? 1 : 0).arg(time);
    bool ok = query.exec(cmd);
    if(!ok)
        return false;
    return query.numRowsAffected() == 1;
}

// ========== 传感器历史数据查询 ==========

QJsonArray DBExec::sensorQueryHistory(int sensorType, const QString &range)
{
    // 传感器类型 → 表名映射
    static const QMap<int, QString> tableMap = {
        {10001, "Co2"},        {10002, "Temperature"},
        {10003, "Fire"},       {10004, "PM25"},
        {10005, "Methane"},    {10006, "Light"},
        {10007, "Smoke"},      {10008, "Humidity"}
    };

    if (!tableMap.contains(sensorType))
        return QJsonArray();

    QString tableName = tableMap[sensorType];

    // 时间范围 → SQL 条件
    QString timeCondition;
    if (range == "1h")
        timeCondition = "time >= NOW() - INTERVAL 1 HOUR";
    else if (range == "24h")
        timeCondition = "time >= NOW() - INTERVAL 1 DAY";
    else if (range == "7d")
        timeCondition = "time >= NOW() - INTERVAL 7 DAY";
    else if (range == "30d")
        timeCondition = "time >= NOW() - INTERVAL 30 DAY";
    else
        timeCondition = "time >= NOW() - INTERVAL 1 DAY";  // 默认24h

    QSqlQuery query(*db);
    QString cmd = QString("SELECT value, time FROM %1 WHERE %2 ORDER BY time ASC;")
                      .arg(tableName).arg(timeCondition);
    bool ok = query.exec(cmd);
    if (!ok)
        return QJsonArray();

    QJsonArray result;
    while (query.next()) {
        QJsonObject item;
        item["value"] = query.value("value").toDouble();
        item["time"]  = query.value("time").toString();
        result.append(item);
    }
    return result;
}


