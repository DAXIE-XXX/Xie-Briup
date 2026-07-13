#include "chathistorymanager.h"
#include <QStandardPaths>
#include <QDir>
#include <QSqlError>
#include <QDebug>

ChatHistoryManager *ChatHistoryManager::obj = nullptr;

ChatHistoryManager::ChatHistoryManager(QObject *parent)
    : QObject(parent)
{
}

ChatHistoryManager *ChatHistoryManager::getObject()
{
    if (obj == nullptr)
        obj = new ChatHistoryManager;
    return obj;
}

bool ChatHistoryManager::init(const QString &username)
{
    if (m_db.isOpen() && m_username == username)
        return true;

    close();

    m_username = username;
    m_connectionName = QString("ChatHistory_%1").arg(username);

    QString dbDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
                    + "/ChatClient";
    QDir().mkpath(dbDir);
    QString dbPath = dbDir + "/" + username + ".sqlite";

    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qWarning() << "ChatHistoryManager: failed to open DB:" << m_db.lastError().text();
        return false;
    }

    QSqlQuery pragma(m_db);
    pragma.exec("PRAGMA journal_mode=WAL");
    pragma.exec("PRAGMA synchronous=NORMAL");
    pragma.exec("PRAGMA foreign_keys=ON");

    if (!initTables()) {
        qWarning() << "ChatHistoryManager: failed to init tables";
        return false;
    }

    qDebug() << "ChatHistoryManager: opened" << dbPath;
    return true;
}

void ChatHistoryManager::close()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
    if (!m_connectionName.isEmpty()) {
        QSqlDatabase::removeDatabase(m_connectionName);
        m_connectionName.clear();
    }
    m_username.clear();
}

bool ChatHistoryManager::isOpen() const
{
    return m_db.isOpen();
}

bool ChatHistoryManager::initTables()
{
    QSqlQuery q(m_db);

    if (!q.exec(
        "CREATE TABLE IF NOT EXISTS private_messages ("
        "  id              INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  sender_username TEXT    NOT NULL,"
        "  receiver_username TEXT  NOT NULL,"
        "  send_status     INTEGER NOT NULL DEFAULT 0,"
        "  send_time       TEXT    NOT NULL,"
        "  message_content TEXT    NOT NULL"
        ");"
    )) {
        qWarning() << "Create private_messages failed:" << q.lastError().text();
        return false;
    }

    if (!q.exec(
        "CREATE TABLE IF NOT EXISTS group_messages ("
        "  id              INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  group_id        TEXT    NOT NULL,"
        "  sender_username TEXT    NOT NULL,"
        "  send_status     INTEGER NOT NULL DEFAULT 0,"
        "  send_time       TEXT    NOT NULL,"
        "  message_content TEXT    NOT NULL"
        ");"
    )) {
        qWarning() << "Create group_messages failed:" << q.lastError().text();
        return false;
    }

    q.exec("CREATE INDEX IF NOT EXISTS idx_pm_peer_time "
           "ON private_messages (receiver_username, send_time DESC);");
    q.exec("CREATE INDEX IF NOT EXISTS idx_pm_sender_time "
           "ON private_messages (sender_username, send_time DESC);");
    q.exec("CREATE INDEX IF NOT EXISTS idx_gm_group_time "
           "ON group_messages (group_id, send_time DESC);");

    return true;
}

QSqlQuery ChatHistoryManager::prepareAndExec(const QString &sql, const QVariantList &bindValues)
{
    QSqlQuery q(m_db);
    q.prepare(sql);
    for (int i = 0; i < bindValues.size(); ++i)
        q.bindValue(i, bindValues[i]);
    if (!q.exec()) {
        qWarning() << "SQL exec failed:" << q.lastError().text()
                    << "SQL:" << sql;
    }
    return q;
}

qint64 ChatHistoryManager::insertPrivateMessage(const QString &senderUsername,
                                                 const QString &receiverUsername,
                                                 MessageSendStatus status,
                                                 const QString &sendTime,
                                                 const QString &content)
{
    QSqlQuery q = prepareAndExec(
        "INSERT INTO private_messages (sender_username, receiver_username, "
        "send_status, send_time, message_content) VALUES (?, ?, ?, ?, ?);",
        {senderUsername, receiverUsername, static_cast<int>(status),
         sendTime, content}
    );
    return q.lastInsertId().toLongLong();
}

bool ChatHistoryManager::updatePrivateMessageStatus(qint64 messageId,
                                                     MessageSendStatus newStatus)
{
    QSqlQuery q = prepareAndExec(
        "UPDATE private_messages SET send_status = ? WHERE id = ?;",
        {static_cast<int>(newStatus), messageId}
    );
    return q.numRowsAffected() > 0;
}

QJsonArray ChatHistoryManager::queryPrivateMessages(const QString &peerUsername,
                                                     int limit, int offset)
{
    QJsonArray arr;
    QSqlQuery q = prepareAndExec(
        "SELECT id, sender_username, receiver_username, send_status, "
        "send_time, message_content "
        "FROM private_messages "
        "WHERE (sender_username = ? AND receiver_username = ?) "
        "   OR (sender_username = ? AND receiver_username = ?) "
        "ORDER BY send_time DESC LIMIT ? OFFSET ?;",
        {m_username, peerUsername, peerUsername, m_username, limit, offset}
    );
    while (q.next()) {
        QJsonObject obj;
        obj["id"] = q.value(0).toLongLong();
        obj["sender_username"] = q.value(1).toString();
        obj["receiver_username"] = q.value(2).toString();
        obj["send_status"] = q.value(3).toInt();
        obj["send_time"] = q.value(4).toString();
        obj["message_content"] = q.value(5).toString();
        arr.append(obj);
    }
    return arr;
}

QJsonArray ChatHistoryManager::queryPrivateMessagesByTimeRange(
        const QString &peerUsername,
        const QString &startTime, const QString &endTime)
{
    QJsonArray arr;
    QSqlQuery q = prepareAndExec(
        "SELECT id, sender_username, receiver_username, send_status, "
        "send_time, message_content "
        "FROM private_messages "
        "WHERE ((sender_username = ? AND receiver_username = ?) "
        "    OR (sender_username = ? AND receiver_username = ?)) "
        "  AND send_time BETWEEN ? AND ? "
        "ORDER BY send_time ASC;",
        {m_username, peerUsername, peerUsername, m_username,
         startTime, endTime}
    );
    while (q.next()) {
        QJsonObject obj;
        obj["id"] = q.value(0).toLongLong();
        obj["sender_username"] = q.value(1).toString();
        obj["receiver_username"] = q.value(2).toString();
        obj["send_status"] = q.value(3).toInt();
        obj["send_time"] = q.value(4).toString();
        obj["message_content"] = q.value(5).toString();
        arr.append(obj);
    }
    return arr;
}

QJsonArray ChatHistoryManager::queryPrivateMessagesByKeyword(
        const QString &peerUsername, const QString &keyword)
{
    QJsonArray arr;
    QSqlQuery q = prepareAndExec(
        "SELECT id, sender_username, receiver_username, send_status, "
        "send_time, message_content "
        "FROM private_messages "
        "WHERE ((sender_username = ? AND receiver_username = ?) "
        "    OR (sender_username = ? AND receiver_username = ?)) "
        "  AND message_content LIKE ? "
        "ORDER BY send_time DESC;",
        {m_username, peerUsername, peerUsername, m_username,
         "%" + keyword + "%"}
    );
    while (q.next()) {
        QJsonObject obj;
        obj["id"] = q.value(0).toLongLong();
        obj["sender_username"] = q.value(1).toString();
        obj["receiver_username"] = q.value(2).toString();
        obj["send_status"] = q.value(3).toInt();
        obj["send_time"] = q.value(4).toString();
        obj["message_content"] = q.value(5).toString();
        arr.append(obj);
    }
    return arr;
}

qint64 ChatHistoryManager::insertGroupMessage(const QString &groupId,
                                               const QString &senderUsername,
                                               MessageSendStatus status,
                                               const QString &sendTime,
                                               const QString &content)
{
    QSqlQuery q = prepareAndExec(
        "INSERT INTO group_messages (group_id, sender_username, "
        "send_status, send_time, message_content) VALUES (?, ?, ?, ?, ?);",
        {groupId, senderUsername, static_cast<int>(status),
         sendTime, content}
    );
    return q.lastInsertId().toLongLong();
}

bool ChatHistoryManager::updateGroupMessageStatus(qint64 messageId,
                                                    MessageSendStatus newStatus)
{
    QSqlQuery q = prepareAndExec(
        "UPDATE group_messages SET send_status = ? WHERE id = ?;",
        {static_cast<int>(newStatus), messageId}
    );
    return q.numRowsAffected() > 0;
}

QJsonArray ChatHistoryManager::queryGroupMessages(const QString &groupId,
                                                    int limit, int offset)
{
    QJsonArray arr;
    QSqlQuery q = prepareAndExec(
        "SELECT id, group_id, sender_username, send_status, "
        "send_time, message_content "
        "FROM group_messages WHERE group_id = ? "
        "ORDER BY send_time DESC LIMIT ? OFFSET ?;",
        {groupId, limit, offset}
    );
    while (q.next()) {
        QJsonObject obj;
        obj["id"] = q.value(0).toLongLong();
        obj["group_id"] = q.value(1).toString();
        obj["sender_username"] = q.value(2).toString();
        obj["send_status"] = q.value(3).toInt();
        obj["send_time"] = q.value(4).toString();
        obj["message_content"] = q.value(5).toString();
        arr.append(obj);
    }
    return arr;
}

QJsonArray ChatHistoryManager::queryGroupMessagesByTimeRange(
        const QString &groupId,
        const QString &startTime, const QString &endTime)
{
    QJsonArray arr;
    QSqlQuery q = prepareAndExec(
        "SELECT id, group_id, sender_username, send_status, "
        "send_time, message_content "
        "FROM group_messages "
        "WHERE group_id = ? AND send_time BETWEEN ? AND ? "
        "ORDER BY send_time ASC;",
        {groupId, startTime, endTime}
    );
    while (q.next()) {
        QJsonObject obj;
        obj["id"] = q.value(0).toLongLong();
        obj["group_id"] = q.value(1).toString();
        obj["sender_username"] = q.value(2).toString();
        obj["send_status"] = q.value(3).toInt();
        obj["send_time"] = q.value(4).toString();
        obj["message_content"] = q.value(5).toString();
        arr.append(obj);
    }
    return arr;
}

QJsonArray ChatHistoryManager::queryGroupMessagesByKeyword(
        const QString &groupId, const QString &keyword)
{
    QJsonArray arr;
    QSqlQuery q = prepareAndExec(
        "SELECT id, group_id, sender_username, send_status, "
        "send_time, message_content "
        "FROM group_messages "
        "WHERE group_id = ? AND message_content LIKE ? "
        "ORDER BY send_time DESC;",
        {groupId, "%" + keyword + "%"}
    );
    while (q.next()) {
        QJsonObject obj;
        obj["id"] = q.value(0).toLongLong();
        obj["group_id"] = q.value(1).toString();
        obj["sender_username"] = q.value(2).toString();
        obj["send_status"] = q.value(3).toInt();
        obj["send_time"] = q.value(4).toString();
        obj["message_content"] = q.value(5).toString();
        arr.append(obj);
    }
    return arr;
}
