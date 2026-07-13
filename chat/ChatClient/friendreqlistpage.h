#ifndef FRIENDREQLISTPAGE_H
#define FRIENDREQLISTPAGE_H

#include <QWidget>

namespace Ui {
class FriendReqListPage;
}

class FriendReqListPage : public QWidget
{
    Q_OBJECT

private:
    explicit FriendReqListPage(QWidget *parent = nullptr);

public:
    static FriendReqListPage *getObject();
    ~FriendReqListPage();

private slots:
    void userRequestListSlot(const QJsonArray &list);
    void userAcceptReqReSlot(bool re);
    void on_btnAccept_clicked();

private:
    static FriendReqListPage *obj;
    Ui::FriendReqListPage *ui;
};

#endif // FRIENDREQLISTPAGE_H
