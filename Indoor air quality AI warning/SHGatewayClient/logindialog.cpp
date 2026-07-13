#include "logindialog.h"
#include "ui_logindialog.h"
#include <QTimer>
#include <QPointer>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
    , serverIP("127.0.0.1")
    , serverPort(8888)
{
    ui->setupUi(this);

    // 设置窗口固定大小
    setFixedSize(420, 420);

    // 连接信号槽
    connect(ui->btnConnect, &QPushButton::clicked, this, &LoginDialog::onConnectClicked);
    connect(ui->btnCancel, &QPushButton::clicked, this, &QDialog::reject);

    // 设置回车键触发连接
    connect(ui->editIP, &QLineEdit::returnPressed, this, &LoginDialog::onConnectClicked);
    connect(ui->editPort, &QLineEdit::returnPressed, this, &LoginDialog::onConnectClicked);

    // 设置默认焦点
    ui->editIP->setFocus();
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::onConnectClicked()
{
    QString ip = ui->editIP->text().trimmed();
    QString portStr = ui->editPort->text().trimmed();

    if (ip.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入服务器IP地址");
        ui->editIP->setFocus();
        ui->editIP->selectAll();
        return;
    }

    if (portStr.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入端口号");
        ui->editPort->setFocus();
        ui->editPort->selectAll();
        return;
    }

    bool ok;
    quint16 port = portStr.toUShort(&ok);
    if (!ok || port == 0) {
        QMessageBox::warning(this, "提示", "端口号必须是1-65535之间的数字");
        ui->editPort->setFocus();
        ui->editPort->selectAll();
        return;
    }

    // 保存服务器信息
    serverIP = ip;
    serverPort = port;

    // 禁用按钮防止重复点击
    ui->btnConnect->setEnabled(false);
    ui->btnConnect->setText("连接中...");

    // ✅ 使用 QPointer 保护
    QPointer<LoginDialog> self(this);
    QTimer::singleShot(100, [self]() {
        if (!self.isNull()) {
            self->ui->btnConnect->setEnabled(true);
            self->ui->btnConnect->setText("连接服务器");
            self->accept();
        }
    });
}

QString LoginDialog::getServerIP() const
{
    return serverIP;
}

quint16 LoginDialog::getServerPort() const
{
    return serverPort;
}