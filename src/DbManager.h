#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QString>
#include <QSqlDatabase>
#include <QVariantMap>
#include <QList>
#include <QPixmap>

// 角色枚举
enum UserRole {
    RoleGuest = 0,
    RoleUser = 1,
    RoleAdmin = 2
};

// 用户状态枚举
enum UserStatus {
    StatusActive = 0,    // 正常
    StatusBanned = 1,    // 已封禁
    StatusPending = 2,   // 待审核
    StatusDeactivated = 3 // 已注销
};

class DbManager
{
public:
    static DbManager& instance();
    bool isOpen() const;

    // 用户相关
    bool registerUser(const QString &studentId, const QString &name, const QString &password, const QString &contact);
    bool loginUser(const QString &studentId, const QString &password);
    void loginAsGuest();

    bool isStudentIdExists(const QString &studentId);
    bool isUserExists(const QString &username);

    int getCurrentUserId() const;
    UserRole getCurrentUserRole() const;

    QString getUserContact(int userId);
    QString getUserName(int userId);

    // 管理员接口
    QList<QVariantMap> getAllUsers();
    bool removeUser(int userId);
    bool banUser(int userId);      // 封禁用户
    bool unbanUser(int userId);    // 解封用户
    UserStatus getUserStatus(int userId); // 获取用户状态

    // 商品相关
    bool addGoods(const QString &name, const QString &price, const QString &imagePath, const QString &desc, int publisherId, const QString &category = "其他");
    QList<QVariantMap> getAllGoods(const QString &keyword = "", const QString &category = "全部");
    QList<QVariantMap> getMyGoods(int userId);
    bool removeGoods(int goodsId);

    // 在 DbManager 类中添加
    bool addMessage(int goodsId, int senderId, const QString &content);
    QList<QVariantMap> getMessagesForGoods(int goodsId);
    // 获取卖家收到的所有留言
    QList<QVariantMap> getMyReceivedMessages(int sellerId);

    // 图片管理相关
    static QString getPresetImagePath(const QString &imageName);
    static QString getUploadImageDir();
    static QString saveUploadedImage(const QPixmap &pixmap);

    // 实验新增接口，所有写操作重新检查当前用户与对象归属。
    QString lastError() const { return m_error; }
    QList<QVariantMap> rows(const QString &sql, const QVariantList &args = {});
    bool requestPurchase(int goodsId);
    bool confirmRequest(int requestId);
    bool rejectRequest(int requestId);
    bool finishOrder(int orderId);
    bool cancelOrder(int orderId);
    bool submitReview(int orderId, int rating, const QString &comment);
    bool submitReport(const QString &type, int targetId, const QString &reason);
    bool handleReport(int reportId, bool uphold, const QString &note);
    bool addProduct(const QString &title, double price, const QString &description,
                    const QString &condition, int categoryId, const QString &imagePath);
    bool isActive();
    bool isAdmin();
    QList<QVariantMap> myOrders();
    QList<QVariantMap> myRequests();
    QList<QVariantMap> reportData(int reportIndex, int threshold = 2);
    QList<QVariantMap> allReports();
    QList<QVariantMap> myReports();
    double credit(int userId);
private:
    bool execute(const QString &sql, const QVariantList &args = {}, bool requireChange = false);
    bool fail(const QString &message);
    bool lockActiveUsers(const QList<int> &ids);
    bool rollback();
    bool commit();
    bool softDeleteGoods(int id);
    bool freezeUser(int id);
    QString m_error;
    DbManager();
    ~DbManager();

    void initDb();
    void initSampleData(); // 👈 新增：初始化样本数据函数

    QSqlDatabase m_db;
    int m_currentUserId = -1;
    UserRole m_currentUserRole = RoleGuest;
};

#endif // DBMANAGER_H
