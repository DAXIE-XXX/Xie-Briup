#ifndef FINDUSERPAGE_H
#define FINDUSERPAGE_H

#include <QWidget>
#include <QJsonArray>
class QListWidgetItem;
namespace Ui {
class FindUserPage;
}

class FindUserPage : public QWidget
{
    Q_OBJECT

private:
    explicit FindUserPage(QWidget *parent = nullptr);

public:
    static FindUserPage * getObject();
    ~FindUserPage();

private slots:
    void userFindListSlot(const QJsonArray &list);
    void userAddFriendRequestReSlot(bool re);
    void on_btnFind_clicked();

    void on_lwUserList_itemDoubleClicked(QListWidgetItem *item);

private:
    static FindUserPage *obj;
    Ui::FindUserPage *ui;
};

#endif // FINDUSERPAGE_H
