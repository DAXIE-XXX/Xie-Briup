#ifndef USERCHANGEPASSWORDPAGE_H
#define USERCHANGEPASSWORDPAGE_H

#include <QWidget>

namespace Ui {
class UserChangePasswordPage;
}

class UserChangePasswordPage : public QWidget
{
    Q_OBJECT

private:
    explicit UserChangePasswordPage(QWidget *parent = nullptr);

public:
    static UserChangePasswordPage *getObject();
    ~UserChangePasswordPage();

private slots:
    void userChangePdReSlot(bool re);
    void on_btnChanged_clicked();

private:
    static UserChangePasswordPage *obj;
    Ui::UserChangePasswordPage *ui;
};

#endif // USERCHANGEPASSWORDPAGE_H
