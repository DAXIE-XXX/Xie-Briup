#ifndef CHATHISTORYMANAGER_H
#define CHATHISTORYMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>

enum class MessageSendStatus : int {
    Sent = 0,
    Delivered = 1,
    Failed = 2
};

class ChatHistoryManager : public QObject
{
    Q_OBJECT

protected:
    explicit ChatHistoryManager(QObject *parent = nullptr);

public:
    static ChatHistoryManager *getObject();

    bool init(const QString &username);
    void close();
    bool isOpen() const;

    qint64 insertPrivateMessage(const QString &senderUsername,
                               const QString &receiverUsername,
                               MessageSendStatus status,
                               const QString &sendTime,
                               const QString &content);

    bool updatePrivateMessageStatus(qint64 messageId,
                                    MessageSendStatus newStatus);

    QJsonArray queryPrivateMessages(const QString &peerUsername,
                                    int limit = 50,
                                    int offset = 0);

    QJsonArray queryPrivateMessagesByTimeRange(const QString &peerUsername,
                                               const QString &startTime,
                                               const QString &endTime);

    QJsonArray queryPrivateMessagesByKeyword(const QString &peerUsername,
                                             const QString &keyword);

    qint64 insertGroupMessage(const QString &groupId,
                             const QString &senderUsername,
                             MessageSendStatus status,
                             const QString &sendTime,
                             const QString &content);

    bool updateGroupMessageStatus(qint64 messageId,
                                  MessageSendStatus newStatus);

    QJsonArray queryGroupMessages(const QString &groupId,
                                  int limit = 50,
                                  int offset = 0);

    QJsonArray queryGroupMessagesByTimeRange(const QString &groupId,
                                             const QString &startTime,
                                             const QString &endTime);

    QJsonArray queryGroupMessagesByKeyword(const QString &groupId,
                                           const QString &keyword);

private:
    bool initTables();
    QSqlQuery prepareAndExec(const QString &sql, const QVariantList &bindValues = {});

    static ChatHistoryManager *obj;

    QSqlDatabase m_db;
    QString m_username;
    QString m_connectionName;
};

#endif // CHATHISTORYMANAGER_H
