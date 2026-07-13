#ifndef MAINPAGE_H
#define MAINPAGE_H

#include <QWidget>
#include <QJsonArray>
#include <QMenu>
#include <QMap>

namespace Ui {
class MainPage;
}
class QListWidgetItem;
class MainPage : public QWidget
{
    Q_OBJECT

private:
    explicit MainPage(QWidget *parent = nullptr);


public:
    static MainPage *getObject();
    ~MainPage();

private slots:
    void userInfoSlot(QString name, QString phone, QString email,
                  QString info, QString birthday, QString avatar);
    void userFriendListSlot(const QJsonArray &list);
    void readSubscriptionMessageSlot(const QString &topic, const QByteArray &value);
    void mqttConnectedSlot();
    void userSendChatSlot(bool re, int index);
    void groupListSlot(const QJsonArray &list);
    void groupCreateSlot(bool re, int groupId);
    void groupSendChatSlot(bool re, int index);
    void logoutReSlot(bool re);
    void groupAddMemberReSlot(bool re);
    void groupRemoveMemberReSlot(bool re);
    void groupMembersListSlot(const QJsonArray &list);
    void groupSetAdminsReSlot(bool re);
    void groupDissolveReSlot(bool re);
    void groupExitReSlot(bool re);
    void groupAdminsListSlot(const QJsonArray &list);
    void groupRenameReSlot(bool re);
    void groupTransferReSlot(bool re);

    void friendBlockReSlot(QString username, bool re);
    void friendUnblockReSlot(QString username, bool re);
    void friendGroupCreateReSlot(bool re, int groupId, QString groupName);
    void friendGroupDeleteReSlot(int groupId, bool re);
    void friendGroupRenameReSlot(int groupId, bool re);
    void friendGroupListSlot(const QJsonArray &list);
    void friendSetGroupReSlot(QString username, bool re);
    void friendSetRemarkReSlot(QString username, bool re);
    void groupSetRemarkReSlot(int groupId, bool re);

    void on_lwFriendList_customContextMenuRequested(const QPoint &pos);
    void onSetFriendRemark();
    void onBlockFriend();
    void onUnblockFriend();
    void onMoveFriendToGroup();
    void onCreateFriendGroup();
    void onDeleteFriendGroup();
    void onRenameFriendGroup();
    void onSetGroupRemark();

    void on_btnEditInfo_clicked();

    void on_btnFindUser_clicked();

    void on_btnFriendReq_clicked();

    void on_btnChangePassword_clicked();

    void on_btnSend_clicked();

    void on_btnCreateGroup_clicked();

    void on_btnLogout_clicked();

    void on_lwFriendList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_lwGroupList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_lwGroupList_customContextMenuRequested(const QPoint &pos);

    void onEditGroupName();
    void onInviteMember();
    void onAddAdmin();
    void onRemoveAdmin();
    void onKickMember();
    void onTransferGroup();
    void onExitGroup();

private:
    void loadGroupList();
    void loadGroupHistory();
    void appendSentBubble(const QString &time, const QString &msg, const QString &avatar);
    void appendReceivedBubble(const QString &time, const QString &sender, const QString &msg, const QString &avatar);
    void showGroupManageMenu(int groupIndex);
    void showGroupManageMenuWithPermission();
    QString getAvatarHtml(const QString &avatar);

    static MainPage *obj;
    Ui::MainPage *ui;
    QJsonArray m_groupList;
    QJsonArray m_friendList;
    QJsonArray m_groupMembers;
    QJsonArray m_groupAdmins;
    QMenu *m_groupMenu;
    QMenu *m_friendMenu;
    QMenu *m_friendGroupMenu;
    int m_currentGroupId;
    int m_groupMenuGroupIndex;
    bool m_groupMenuIsOwner;
    QString m_menuFriendUsername;
    QString m_pendingMoveFriend;
    int m_menuFriendGroupId;
    QJsonArray m_friendGroupList;
    QMap<QString, QString> m_avatarCache;
};

#endif // MAINPAGE_H