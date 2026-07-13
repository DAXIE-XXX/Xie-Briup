#ifndef CONFIGCONTEXT_H
#define CONFIGCONTEXT_H

#include <QObject>

class ConfigContext : public QObject
{
    Q_OBJECT
protected:
    explicit ConfigContext(QObject *parent = nullptr);

public:
    static void getServerInfo(QString &ip, quint16 &port);
    static void getCarInfo(int &id, QString &key);
    static void getCarRunList(QStringList &list);

};

#endif // CONFIGCONTEXT_H






