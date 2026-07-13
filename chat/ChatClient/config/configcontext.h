#ifndef CONFIGCONTEXT_H
#define CONFIGCONTEXT_H

#include <QObject>
#include <QJsonObject>

class ConfigContext : public QObject
{
    Q_OBJECT
protected:
    explicit ConfigContext(QObject *parent = nullptr);

public:
    static ConfigContext *getObject();
    static void getServerInfo(QString &ip, quint16 &port);

    Q_INVOKABLE QJsonObject getConfigPageInfo();
    Q_INVOKABLE void setConfigPageInfo(QString ip, int port);
    /**
     * @brief 设置配置文件路径
     * @param path
     */
    static void setConfigFilePath(QString path);

protected:
    static ConfigContext *obj;
    static QString CONF_FILENAME;
};

#endif // CONFIGCONTEXT_H






