#include "edituserinfopage.h"
#include "ui_edituserinfopage.h"
#include "netcontext.h"
#include <QDate>
#include <QMessageBox>
#include <QRegularExpression>
#include <QFileDialog>
#include <QImage>
#include <QPixmap>
#include <QBuffer>

EditUserInfoPage *EditUserInfoPage::obj = nullptr;
EditUserInfoPage::EditUserInfoPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::EditUserInfoPage)
{
    ui->setupUi(this);
    QRegularExpression regex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    QRegularExpressionValidator* validator = new QRegularExpressionValidator(regex, this);
    ui->leEmail->setValidator(validator);
    connect(NetContext::getObject(),
            &NetContext::userInfo,
            this,
            &EditUserInfoPage::userInfoSlot);
    connect(NetContext::getObject(),
            &NetContext::userSetInfoRe,
            this,
            &EditUserInfoPage::userSetInfoReSlot);
    connect(NetContext::getObject(),
            &NetContext::avatarUploadRe,
            this,
            &EditUserInfoPage::avatarUploadReSlot);
}

EditUserInfoPage *EditUserInfoPage::getObject()
{
    if(obj == nullptr)
        obj = new EditUserInfoPage;
    return obj;
}

EditUserInfoPage::~EditUserInfoPage()
{
    delete ui;
}

void EditUserInfoPage::userInfoSlot(QString name, QString phone, QString email, QString info, QString birthday, QString avatar)
{
    ui->leName->setText(name);
    ui->lePhone->setText(phone);
    ui->leEmail->setText(email);
    ui->deBirthday->setDate(QDate::fromString(birthday, "yyyy-MM-dd"));
    ui->teInfo->setText(info);
    if(!avatar.isEmpty()){
        QByteArray avatarData = QByteArray::fromBase64(avatar.toUtf8());
        QImage image;
        image.loadFromData(avatarData);
        if(!image.isNull()){
            QPixmap pixmap = QPixmap::fromImage(image.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->lbAvatar->setPixmap(pixmap);
            ui->lbAvatar->setScaledContents(true);
        }
    }
}

void EditUserInfoPage::userSetInfoReSlot(bool re)
{
    if(re){
        NetContext::getObject()->userGetInfo();
        this->close();
    }
    else
        QMessageBox::information(
            this, "提示", "修改失败");
}

void EditUserInfoPage::avatarUploadReSlot(bool re)
{
    if(re){
        QMessageBox::information(this, "提示", "头像上传成功");
        NetContext::getObject()->userGetInfo();
    }
    else{
        QMessageBox::warning(this, "提示", "头像上传失败");
    }
}

void EditUserInfoPage::on_btnSave_clicked()
{
    NetContext::getObject()
    ->userSetInfo(
        ui->leName->text(),
        ui->lePhone->text(),
        ui->leEmail->text(),
        ui->teInfo->toPlainText(),
        ui->deBirthday->date().toString("yyyy-MM-dd")
        );
}

void EditUserInfoPage::on_btnUploadAvatar_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this, "选择头像", "",
        "图片文件 (*.png *.jpg *.jpeg *.bmp)");
    if(filePath.isEmpty())
        return;

    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this, "提示", "无法打开文件");
        return;
    }

    qint64 fileSize = file.size();
    if(fileSize > 3 * 1024 * 1024){
        QMessageBox::warning(this, "提示", "图片大小不能超过3M");
        return;
    }

    QImage image(filePath);
    if(image.isNull()){
        QMessageBox::warning(this, "提示", "无效的图片文件");
        return;
    }

    if(image.width() != image.height()){
        if(QMessageBox::question(this, "提示", "图片比例不是1:1，是否进行裁剪？",
                                 QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes){
            int minSize = qMin(image.width(), image.height());
            QRect cropRect((image.width() - minSize)/2,
                          (image.height() - minSize)/2,
                          minSize, minSize);
            image = image.copy(cropRect);
        }
        else{
            return;
        }
    }

    QPixmap pixmap = QPixmap::fromImage(image.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->lbAvatar->setPixmap(pixmap);
    ui->lbAvatar->setScaledContents(true);

    QBuffer buffer;
    image.save(&buffer, "PNG");
    QByteArray imageData = buffer.data();

    NetContext::getObject()->userSetAvatar(imageData);
}