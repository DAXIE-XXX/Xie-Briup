#ifndef EDITUSERINFOPAGE_H
#define EDITUSERINFOPAGE_H

#include <QWidget>

namespace Ui {
class EditUserInfoPage;
}

class EditUserInfoPage : public QWidget
{
    Q_OBJECT

private:
    explicit EditUserInfoPage(QWidget *parent = nullptr);

public:
    static EditUserInfoPage *getObject();
    ~EditUserInfoPage();

private slots:
    void userInfoSlot(QString name, QString phone, QString email,
                      QString info, QString birthday, QString avatar);
    void userSetInfoReSlot(bool re);
    void avatarUploadReSlot(bool re);
    void on_btnSave_clicked();
    void on_btnUploadAvatar_clicked();

private:
    static EditUserInfoPage *obj;
    Ui::EditUserInfoPage *ui;
};

#endif // EDITUSERINFOPAGE_H