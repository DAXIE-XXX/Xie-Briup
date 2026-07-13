#ifndef DBEXEC_H
#define DBEXEC_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>
class QSqlDatabase;

/**
 * @brief 数据库操作封装类
 *        执行所有SQL操作，包括用户管理、好友关系、群聊管理
 */
class DBExec : public QObject
{
    Q_OBJECT
    friend class DBContext;
public:
    explicit DBExec(
        QString ip, quint16 port, QString username,
        QString password, QString dbName, QString conName,
        QObject *parent = nullptr);
    //向数据库添加数据

    // ==================== 传感器数据管理 ====================
    /** @brief 插入CO2传感器数据
 *  @param value CO2浓度值 (ppm)
 *  @param time  采集时间
 *  @return 是否成功 */
    bool sensorInsertCo2(int value, const QString &time);

    /** @brief 插入温度传感器数据
 *  @param value 温度值 (℃)
 *  @param time  采集时间
 *  @return 是否成功 */
    bool sensorInsertTemperature(double value, const QString &time);

    /** @brief 插入湿度传感器数据
 *  @param value 湿度值 (%RH)
 *  @param time  采集时间
 *  @return 是否成功 */
    bool sensorInsertHumidity(double value, const QString &time);

    /** @brief 插入PM2.5传感器数据
 *  @param value PM2.5浓度值 (µg/m³)
 *  @param time  采集时间
 *  @return 是否成功 */
    bool sensorInsertPM25(int value, const QString &time);

    /** @brief 插入光照传感器数据
 *  @param value 光照强度值 (Lux)
 *  @param time  采集时间
 *  @return 是否成功 */
    bool sensorInsertLight(int value, const QString &time);

    /** @brief 插入烟雾传感器数据
 *  @param value 烟雾检测状态 (true=检测到烟雾)
 *  @param time  采集时间
 *  @return 是否成功 */
    bool sensorInsertSmoke(bool value, const QString &time);

    /** @brief 插入甲烷传感器数据
 *  @param value 甲烷检测状态 (true=检测到泄漏)
 *  @param time  采集时间
 *  @return 是否成功 */
    bool sensorInsertMethane(bool value, const QString &time);

    /** @brief 插入火焰传感器数据
 *  @param value 火焰检测状态 (true=检测到火焰)
 *  @param time  采集时间
 *  @return 是否成功 */
    bool sensorInsertFire(bool value, const QString &time);

    /** @brief 查询传感器历史数据
     *  @param sensorType 传感器类型 (10001~10008)
     *  @param range      时间范围 ("1h", "24h", "7d", "30d")
     *  @return JSON数组 [{time, value}, ...] */
    QJsonArray sensorQueryHistory(int sensorType, const QString &range);

    // ==================== 用户管理 ====================
    /** @brief 注册新用户（MD5密码），同时创建空的用户信息扩展记录
     *  @param username 用户名
     *  @param password 密码（明文，内部MD5存储）
     *  @return 是否成功 */
    bool userInsert(QString username, QString password);

    /** @brief 验证用户名密码
     *  @param username 用户名
     *  @param password 密码（明文）
     *  @return 匹配返回true */
    bool userSelect(QString username, QString password);

    /** @brief 查询用户详细信息
     *  @param userId 用户ID
     *  @return JSON对象 {name, phone, email, info, birthday} */
    QJsonObject userGetInfo(int userId);

    /** @brief 根据用户名查询用户ID
     *  @param username 用户名
     *  @return 用户ID，不存在返回-1 */
    int userGetId(QString username);

    /** @brief 修改用户个人信息
     *  @param userId   用户ID
     *  @param name     昵称
     *  @param phone    手机号
     *  @param email    邮箱
     *  @param info     签名
     *  @param birthday 生日 (yyyy-MM-dd)
     *  @return 是否成功 */
    bool userSetInfo(int userId, QString name, QString phone, QString email, QString info, QString birthday);

    /** @brief 修改密码
     *  @param username 用户名
     *  @param password 新密码（明文，内部MD5存储）
     *  @return 是否成功 */
    bool userChangePassword(QString username, QString password);

    /** @brief 搜索用户（模糊匹配用户名/昵称），排除指定用户和已是好友的
     *  @param name          搜索关键词
     *  @param excludeUserId 要排除的用户ID（当前用户）
     *  @return 匹配结果数组 [{username, name}, ...] */
    QJsonArray userFind(QString name, int excludeUserId);

    /** @brief 根据用户ID获取用户名
     *  @param userId 用户ID
     *  @return 用户名，不存在返回空字符串 */
    QString userGetUsername(int userId);

    // ==================== 好友关系 ====================
    /** @brief 检查两人是否为好友
     *  @param user_id1 用户1的ID
     *  @param user_id2 用户2的ID
     *  @return 是好友返回true */
    bool friendCheck(int user_id1, int user_id2);

    /** @brief 发送好友申请
     *  @param fromUserId 申请人ID
     *  @param toUserId   被申请人ID
     *  @param msg        申请附言
     *  @param time       申请时间
     *  @return 是否成功 */
    bool userAddFriendRequest(int fromUserId, int toUserId, QString msg, QString time);

    /** @brief 获取待处理的好友申请列表
     *  @param userId 当前用户ID
     *  @return 申请列表 [{username, name, req_msg, req_time, req_id}, ...] */
    QJsonArray userGetRequestList(int userId);

    /** @brief 同意好友申请（事务：更新状态+创建好友关系）
     *  @param reqId 申请记录ID
     *  @return 是否成功 */
    bool userAcceptFriendRequest(int reqId);

    /** @brief 拒绝好友申请
     *  @param reqId 申请记录ID
     *  @return 是否成功 */
    bool userRejectedFriendRequest(int reqId);

    /** @brief 查询好友申请的发起者ID
     *  @param reqId 申请记录ID
     *  @return 发起者用户ID，不存在返回-1 */
    int friendRequestGetFromUserId(int reqId);

    /** @brief 获取好友列表（含备注/分组/拉黑状态，双向查询）
     *  @param userId 当前用户ID
     *  @return 好友列表 [{username, name, phone, email, birthday, info, group_name, remark, state}, ...] */
    QJsonArray userGetFriendList(int userId);

    /** @brief 获取用户设置的所有不重复分组名
     *  @param userId 当前用户ID
     *  @return 分组名数组 ["同事", "同学", ...] */
    QJsonArray friendGetGroups(int userId);

    /** @brief 设置好友分组
     *  @param userId     当前用户ID
     *  @param friendId   好友用户ID
     *  @param groupName  分组名（空字符串表示默认分组）
     *  @return 是否成功 */
    bool friendSetGroup(int userId, int friendId, const QString &groupName);

    /** @brief 设置好友备注
     *  @param userId   当前用户ID
     *  @param friendId 好友用户ID
     *  @param remark   备注名
     *  @return 是否成功 */
    bool friendSetRemark(int userId, int friendId, const QString &remark);

    /** @brief 获取指定分组内的好友列表
     *  @param userId    当前用户ID
     *  @param groupName 分组名（空字符串表示未分组好友）
     *  @return 好友列表 */
    QJsonArray friendGetListByGroup(int userId, const QString &groupName);

    /** @brief 拉黑好友
     *  @param userId   当前用户ID
     *  @param friendId 要拉黑的好友ID
     *  @return 是否成功 */
    bool friendBlock(int userId, int friendId);

    /** @brief 解除拉黑
     *  @param userId   当前用户ID
     *  @param friendId 要解除拉黑的好友ID
     *  @return 是否成功 */
    bool friendUnblock(int userId, int friendId);

    /** @brief 检查sender是否被receiver拉黑
     *  @param senderId   发送者ID
     *  @param receiverId 接收者ID
     *  @return 被拉黑返回true */
    bool friendIsBlockedBy(int senderId, int receiverId);

    // ==================== 群聊管理 ====================
    /** @brief 创建群聊，创建者自动成为群主和成员
     *  @param name        群名称
     *  @param adminUserId 群主用户ID
     *  @param groupId     [输出] 创建成功后的群ID
     *  @return 是否成功 */
    bool groupCreate(QString name, int adminUserId, int &groupId);

    /** @brief 获取用户加入的所有群列表（含群主信息）
     *  @param userId 用户ID
     *  @return 群列表 [{group_id, group_name, admin_user_id, admin_username}, ...] */
    QJsonArray groupGetList(int userId);

    /** @brief 添加群成员
     *  @param groupId 群ID
     *  @param userId  要添加的用户ID
     *  @return 是否成功 */
    bool groupAddMember(int groupId, int userId);

    /** @brief 移除群成员
     *  @param groupId 群ID
     *  @param userId  要移除的用户ID
     *  @return 是否成功 */
    bool groupRemoveMember(int groupId, int userId);

    /** @brief 获取群成员列表（含用户名/昵称/群昵称）
     *  @param groupId 群ID
     *  @return 成员列表 [{username, name, group_user_name}, ...] */
    QJsonArray groupGetMembers(int groupId);

    /** @brief 检查用户是否为群成员
     *  @param groupId 群ID
     *  @param userId  用户ID
     *  @return 是群成员返回true */
    bool groupCheckMember(int groupId, int userId);

    /** @brief 获取群内所有成员ID列表
     *  @param groupId 群ID
     *  @return 成员ID列表 */
    QList<int> groupGetMemberIds(int groupId);

    /** @brief 解散群聊（仅群主，级联删除成员和管理员）
     *  @param groupId     群ID
     *  @param adminUserId 操作者ID（需为群主）
     *  @return 是否成功 */
    bool groupDissolve(int groupId, int adminUserId);

    /** @brief 退出群聊（群主不可退出）
     *  @param groupId 群ID
     *  @param userId  退出者ID
     *  @return 是否成功 */
    bool groupExit(int groupId, int userId);

    /** @brief 获取群名称
     *  @param groupId 群ID
     *  @return 群名称 */
    QString groupGetName(int groupId);

    /** @brief 获取群管理员列表（含群主）
     *  @param groupId 群ID
     *  @return 管理员列表 [{username, name}, ...] */
    QJsonArray groupGetAdmins(int groupId);

    /** @brief 批量设置群管理员（覆盖式，群主自动保留）
     *  @param groupId   群ID
     *  @param usernames 管理员用户名列表
     *  @return 是否成功 */
    bool groupSetAdmins(int groupId, const QStringList &usernames);

    /** @brief 判断用户是否为群主或管理员
     *  @param groupId 群ID
     *  @param userId  用户ID
     *  @return 是管理员或群主返回true */
    bool groupIsAdmin(int groupId, int userId);

    /** @brief 判断用户是否为群主
     *  @param groupId 群ID
     *  @param userId  用户ID
     *  @return 是群主返回true */
    bool groupIsOwner(int groupId, int userId);

protected:
    QSqlDatabase *db;
};

#endif // DBEXEC_H