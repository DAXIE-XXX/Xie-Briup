/*=====================================================================
   tcpclient.h
   功能：封装 QTcpSocket，向 GUI（MainWindow）提供
          • 连接 / 断开 / 状态查询
          • 发送 JSON / 字符串指令（行结束符 \n）
          • 连接、断开、错误、收到数据等信号
   兼容：Qt6 (Core, Network) 、C++17 及以上
   =====================================================================*/

#ifndef TCPCLIENT_H
#define TCPCLIENT_H

/* --------------------------------------------------------------
   必须的 Qt 头文件——没有它们编译器会报 “file not found”。
   -------------------------------------------------------------- */
#include <QObject>          // QObject、Q_OBJECT
#include <QTcpSocket>       // QTcpSocket
#include <QJsonObject>      // QJsonObject
#include <QJsonDocument>    // QJsonDocument
#include <QString>          // QString
#include <QByteArray>       // QByteArray
#include <QHostAddress>      // QHostAddress
#include <QDebug>           // qDebug / qWarning

/* --------------------------------------------------------------
   TcpClient 公开的 API
   -------------------------------------------------------------- */
class TcpClient : public QObject
{
    Q_OBJECT               // 必须放在类定义的第一行，开启 MOC

public:
    explicit TcpClient(QObject *parent = nullptr);
    ~TcpClient() override;

    /* 连接管理 ------------------------------------------------- */
    /** 通过 IP+Port 异步发起连接。成功/失败由信号通知。 */
    bool    connectToServer(const QString &ip, quint16 port);
    void    disconnectFromServer();
    bool    isConnected() const;

    /* 数据发送 ------------------------------------------------- */
    void    sendCommand(const QJsonObject &command);
    void    sendCommand(const QString &command);

signals:
    /* 与 UI 层交互的信号 ------------------------------------ */
    void    connected();                     // 与服务器建立 TCP 连接
    void    disconnected();                  // 连接被关闭（主动或被动）
    void    errorOccurred(const QString &msg);
    void    dataReceived(const QJsonObject &data); // 完整的一行 JSON
    void    serverCommandReceived(const QJsonObject &cmd);  // 新增：服务器下发的联动指令

private slots:
    /* QTcpSocket 的回调 -------------------------------------- */
    void    onConnected();
    void    onDisconnected();
    void    onError(QAbstractSocket::SocketError error);
    void    onReadyRead();

private:
    QTcpSocket *socket = nullptr;   // 实际的网络套接字
    QByteArray   buffer;            // 用来缓存半包数据（按 '\n' 分割）
};

#endif // TCPCLIENT_H