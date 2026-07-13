#include "tcpclient.h"
#include <QDebug>

TcpClient::TcpClient(QObject *parent)
    : QObject(parent)
    , socket(new QTcpSocket(this))
{
    connect(socket, &QTcpSocket::connected, this, &TcpClient::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &TcpClient::onDisconnected);
    connect(socket, &QTcpSocket::errorOccurred, this, &TcpClient::onError);
    connect(socket, &QTcpSocket::readyRead, this, &TcpClient::onReadyRead);
}

TcpClient::~TcpClient()
{
    if (socket->state() == QTcpSocket::ConnectedState) {
        socket->disconnectFromHost();
    }
}

bool TcpClient::connectToServer(const QString &ip, quint16 port)
{
    // 如果已经在连接中或已连接，先断开并等待
    if (socket->state() != QTcpSocket::UnconnectedState) {
        socket->abort();  // 立即中止当前操作
    }
    socket->connectToHost(QHostAddress(ip), port);
    return true;  // 异步连接，通过信号通知结果
}

void TcpClient::disconnectFromServer()
{
    if (socket->state() == QTcpSocket::ConnectedState) {
        socket->disconnectFromHost();
    }
}

bool TcpClient::isConnected() const
{
    return socket->state() == QTcpSocket::ConnectedState;
}

void TcpClient::sendCommand(const QJsonObject &command)
{
    if (!isConnected()) {
        qWarning() << "Not connected to server!";
        return;
    }
    QByteArray data = QJsonDocument(command).toJson(QJsonDocument::Compact);
    data.append('\n');  // 关键：添加换行符
    socket->write(data);
    socket->flush();
    qDebug() << "📤 Sent:" << data;
}

void TcpClient::sendCommand(const QString &command)
{
    if (!isConnected()) {
        qWarning() << "Not connected to server!";
        return;
    }
    QByteArray data = command.toUtf8();
    if (!data.endsWith('\n')) {
        data.append('\n');
    }
    socket->write(data);
    socket->flush();
    qDebug() << "📤 Sent:" << data;
}

void TcpClient::onConnected()
{
    qDebug() << "✅ Connected to server!";
    emit connected();
}

void TcpClient::onDisconnected()
{
    qDebug() << "❌ Disconnected from server!";
    emit disconnected();
}

void TcpClient::onError(QAbstractSocket::SocketError error)
{
    QString errorMsg = socket->errorString();
    qDebug() << "⚠️ Socket error:" << errorMsg;
    emit errorOccurred(errorMsg);
}

void TcpClient::onReadyRead()
{
    buffer.append(socket->readAll());

    while (true) {
        int index = buffer.indexOf('\n');
        if (index == -1) {
            break;
        }

        QByteArray line = buffer.left(index).trimmed();
        buffer.remove(0, index + 1);

        if (line.isEmpty()) {
            continue;
        }

        QJsonDocument doc = QJsonDocument::fromJson(line);
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            int type = obj.value("type").toInt();

            qDebug() << "📥 Received:" << obj;

            // 服务器联动的type范围：20050~20059（服务器自动下发）
            if (type >= 20050 && type <= 20059) {
                emit serverCommandReceived(obj);
            } else {
                // 普通数据（传感器数据等）
                emit dataReceived(obj);
            }
        } else {
            qWarning() << "⚠️ Invalid JSON:" << line;
        }
    }
}