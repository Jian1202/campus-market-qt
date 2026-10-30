#include "DbManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QCoreApplication>
#include <QSettings>
#include <QDir>
#include <QDateTime>
#include <QRandomGenerator>
#include <QMessageAuthenticationCode>
#include <algorithm>
#include <cmath>

static QString passwordHash(const QString &password, const QByteArray &salt) {
    // PBKDF2-HMAC-SHA256，120000 次迭代；盐与摘要以十六进制保存。
    QByteArray u = QMessageAuthenticationCode::hash(salt + QByteArray("\0\0\0\1",4),password.toUtf8(),QCryptographicHash::Sha256);
    QByteArray result=u;
    for(int i=1;i<120000;++i) {
        u=QMessageAuthenticationCode::hash(u,password.toUtf8(),QCryptographicHash::Sha256);
        for(int j=0;j<result.size();++j) result[j]=result[j]^u[j];
    }
    return QString::fromLatin1(salt.toHex()+"$"+result.toHex());
}
DbManager& DbManager::instance() { static DbManager manager; return manager; }
DbManager::DbManager() {
    const QString path=QCoreApplication::applicationDirPath()+"/database.ini";
    QSettings cfg(path,QSettings::IniFormat);
    if(!QSqlDatabase::isDriverAvailable("QMYSQL")) {
        m_error="MySQL 驱动未加载，请重新构建工程以自动部署依赖。\n检查 exe 同目录的 sqldrivers/qsqlmysql.dll 和 libmysql.dll 等依赖。\n随包驱动适用于 Qt 6.11.1 MinGW 64 位。\n程序目录："+QCoreApplication::applicationDirPath();
        return;
    }
    m_db=QSqlDatabase::addDatabase("QMYSQL");
    if(!m_db.isValid()) {
        m_error="发现 QMYSQL 插件但未能加载，请检查 libmysql.dll 等依赖及 Qt 套件是否匹配。\n请重新构建以部署完整依赖。\n程序目录："+QCoreApplication::applicationDirPath();
        return;
    }
    m_db.setHostName(cfg.value("mysql/host","127.0.0.1").toString());
    m_db.setPort(cfg.value("mysql/port",3306).toInt());
    m_db.setDatabaseName(cfg.value("mysql/database","campus_market").toString());
    m_db.setUserName(cfg.value("mysql/user","campus_app").toString());
    m_db.setPassword(qEnvironmentVariableIsSet("CAMPUS_DB_PASSWORD")?qEnvironmentVariable("CAMPUS_DB_PASSWORD"):cfg.value("mysql/password").toString());
    m_db.setConnectOptions("MYSQL_OPT_CONNECT_TIMEOUT=5");
    if(!m_db.open()) { m_error="MySQL 驱动已加载，但数据库连接失败："+m_db.lastError().text()+"\n服务器："+m_db.hostName()+":"+QString::number(m_db.port())+"\n配置文件："+path+"\n首次演示请运行 exe 同目录的“运行演示.cmd”，或配置正在运行的自有 MySQL 服务。"; return; }
    execute("SET NAMES utf8mb4");
    execute("SET SESSION TRANSACTION ISOLATION LEVEL READ COMMITTED");
    QSqlQuery q;
    if(!q.exec("SELECT credit_score FROM `User` LIMIT 1")) { m_error="请先执行数据库初始化脚本："+q.lastError().text(); m_db.close(); }
}
DbManager::~DbManager() { m_db.close(); }
bool DbManager::isOpen() const { return m_db.isOpen(); }
bool DbManager::fail(const QString &message) { m_error=message; return false; }
bool DbManager::rollback() { m_db.rollback(); return false; }
bool DbManager::commit() {if(m_db.commit()) return true;m_error=m_db.lastError().text();m_db.rollback();return false;}
bool DbManager::execute(const QString &sql,const QVariantList &args,bool requireChange) {
    QSqlQuery q; q.prepare(sql); for(const auto &v:args) q.addBindValue(v);
    if(!q.exec()) return fail(q.lastError().text());
    if(requireChange && q.numRowsAffected()!=1) return fail("状态已改变或无权操作，请刷新后重试");
    return true;
}
QList<QVariantMap> DbManager::rows(const QString &sql,const QVariantList &args) {
    QList<QVariantMap> result; QSqlQuery q; q.prepare(sql); for(const auto &v:args) q.addBindValue(v);
    if(!q.exec()) {m_error=q.lastError().text();return result;}
    while(q.next()) {QVariantMap row; for(int i=0;i<q.record().count();++i) row[q.record().fieldName(i)]=q.value(i); result.append(row);}
    return result;
}
bool DbManager::isActive() {
    auto r=rows("SELECT id FROM `User` WHERE id=? AND status=0",{m_currentUserId});
    return !r.isEmpty() || fail("请登录正常账户；冻结账户不能执行此操作");
}
bool DbManager::isAdmin() {
    auto r=rows("SELECT id FROM `User` WHERE id=? AND role=2 AND status=0",{m_currentUserId});
    return !r.isEmpty() || fail("此操作仅限管理员");
}
bool DbManager::lockActiveUsers(const QList<int> &ids) {
    auto sorted=ids; std::sort(sorted.begin(),sorted.end());
    for(int id:sorted) if(rows("SELECT id FROM `User` WHERE id=? AND status=0 FOR UPDATE",{id}).isEmpty()) return fail("交易参与者已被冻结或不存在");
    return true;
}
bool DbManager::registerUser(const QString &studentId,const QString &name,const QString &password,const QString &contact) {
    if(studentId.trimmed().isEmpty()||name.trimmed().isEmpty()||password.size()<6||contact.trimmed().isEmpty()) return fail("请填写完整，密码至少六位");
    QByteArray salt; for(int i=0;i<16;++i) salt.append(char(QRandomGenerator::system()->bounded(256)));
    return execute("INSERT INTO `User`(student_id,name,password,contact) VALUES(?,?,?,?)",{studentId.trimmed(),name.trimmed(),passwordHash(password,salt),contact.trimmed()});
}
bool DbManager::loginUser(const QString &studentId,const QString &password) {
    loginAsGuest(); auto r=rows("SELECT * FROM `User` WHERE student_id=? AND status=0",{studentId});
    if(r.isEmpty()) return fail("学号、密码错误或账户已冻结");
    QString stored=r[0]["password"].toString(); QByteArray salt=QByteArray::fromHex(stored.section('$',0,0).toLatin1());
    if(passwordHash(password,salt)!=stored) return fail("学号或密码错误");
    m_currentUserId=r[0]["id"].toInt();m_currentUserRole=static_cast<UserRole>(r[0]["role"].toInt()); return true;
}
void DbManager::loginAsGuest() {m_currentUserId=-1;m_currentUserRole=RoleGuest;}
int DbManager::getCurrentUserId() const {return m_currentUserId;}
UserRole DbManager::getCurrentUserRole() const {return m_currentUserRole;}
bool DbManager::isStudentIdExists(const QString &id) {return !rows("SELECT id FROM `User` WHERE student_id=?",{id}).isEmpty();}
bool DbManager::isUserExists(const QString &name) {return !rows("SELECT id FROM `User` WHERE name=?",{name}).isEmpty();}
QString DbManager::getUserName(int id) {auto r=rows("SELECT name FROM `User` WHERE id=?",{id});return r.isEmpty()?"未知用户":r[0]["name"].toString();}
QString DbManager::getUserContact(int id) {if(!isActive()) return "登录可见";auto r=rows("SELECT contact FROM `User` WHERE id=?",{id});return r.isEmpty()?"":r[0]["contact"].toString();}
double DbManager::credit(int id) {auto r=rows("SELECT credit_score FROM `User` WHERE id=?",{id});return r.isEmpty()?100:r[0]["credit_score"].toDouble();}
UserStatus DbManager::getUserStatus(int id) {auto r=rows("SELECT status FROM `User` WHERE id=?",{id});return r.isEmpty()?StatusDeactivated:static_cast<UserStatus>(r[0]["status"].toInt());}

static QString goodsSelect() {
    return "SELECT g.*,c.name category,u.name publisher,u.contact,u.credit_score,u.id publisherId,g.image_path imagePath FROM Goods g JOIN Category c ON c.id=g.category_id JOIN `User` u ON u.id=g.publisher_id ";
}
static QList<QVariantMap> formatGoods(QList<QVariantMap> data) {
    for(auto &r:data) r["price"]=QString("¥ %1").arg(r["price"].toDouble(),0,'f',2);
    return data;
}
QList<QVariantMap> DbManager::getAllGoods(const QString &keyword,const QString &category) {
    QString sql=goodsSelect()+"WHERE g.is_deleted=0 AND g.status='available' AND u.status=0"; QVariantList args;
    if(!keyword.trimmed().isEmpty()) {
        // 全文索引适合至少两个汉字的词；单字检索使用参数化 LIKE。
        if(keyword.trimmed().size()>=2) {sql+=" AND MATCH(g.name,g.description) AGAINST(? IN NATURAL LANGUAGE MODE)";args<<keyword.trimmed();}
        else {sql+=" AND (g.name LIKE ? OR g.description LIKE ?)";args<<"%"+keyword.trimmed()+"%"<<"%"+keyword.trimmed()+"%";}
    }
    if(category!="全部") {sql+=" AND c.name=?";args<<category;}
    auto data=formatGoods(rows(sql+" ORDER BY g.id DESC",args));
    if(m_currentUserId<0) for(auto &r:data) r["contact"]="登录可见";
    return data;
}
QList<QVariantMap> DbManager::getMyGoods(int id) {
    if(!isActive()||id!=m_currentUserId) return {};
    return formatGoods(rows(goodsSelect()+"WHERE g.publisher_id=? AND g.is_deleted=0 ORDER BY g.id DESC",{id}));
}
bool DbManager::addProduct(const QString &title,double price,const QString &description,const QString &condition,int categoryId,const QString &imagePath) {
    if(!isActive()) return false;
    if(title.trimmed().isEmpty()||description.trimmed().isEmpty()||condition.trimmed().isEmpty()||!std::isfinite(price)||price<0||price>99999999.99) return fail("请填写标题、成色、描述和有效价格");
    return execute("INSERT INTO Goods(name,price,description,condition_desc,category_id,publisher_id,image_path) SELECT ?,?,?,?,?,id,? FROM `User` WHERE id=? AND status=0",{title.trimmed(),price,description.trimmed(),condition.trimmed(),categoryId,imagePath,m_currentUserId},true);
}
bool DbManager::addGoods(const QString &name,const QString &price,const QString &imagePath,const QString &desc,int publisherId,const QString &category) {
    if(publisherId!=m_currentUserId) return fail("不能代他人发布商品");
    auto c=rows("SELECT id FROM Category WHERE name=?",{category});bool ok; double amount=price.toDouble(&ok);
    if(c.isEmpty()||!ok) return fail("分类或价格无效");
    return addProduct(name,amount,desc,"详见商品描述",c[0]["id"].toInt(),imagePath);
}
bool DbManager::softDeleteGoods(int id) {
    auto g=rows("SELECT * FROM Goods WHERE id=? FOR UPDATE",{id});
    if(g.isEmpty()||(g[0]["publisher_id"].toInt()!=m_currentUserId&&!isAdmin())) return fail("无权下架此商品");
    if(g[0]["is_deleted"].toInt()) return true;
    return execute("UPDATE TradeOrder SET status='cancelled' WHERE goods_id=? AND status='confirmed'",{id})
    &&execute("UPDATE PurchaseRequest SET status='rejected' WHERE goods_id=? AND status='pending'",{id})
    &&execute("UPDATE Goods SET is_deleted=1,status=IF(status='reserved','available',status) WHERE id=?",{id},true);
}
bool DbManager::removeGoods(int id) {
    if(!isActive()||!m_db.transaction()) return false;
    if(!softDeleteGoods(id)) return rollback();
    return commit();
}
QList<QVariantMap> DbManager::getAllUsers() {if(!isAdmin()) return {};return rows("SELECT id,name,contact,status,credit_score FROM `User` WHERE role=1 ORDER BY id DESC");}
bool DbManager::freezeUser(int id) {
    if(id==m_currentUserId||rows("SELECT id FROM `User` WHERE id=? AND role=1 FOR UPDATE",{id}).isEmpty()) return fail("只能冻结其他普通用户");
    auto orders=rows("SELECT * FROM TradeOrder WHERE (buyer_id=? OR seller_id=?) AND status='confirmed' FOR UPDATE",{id,id});
    for(const auto &o:orders) {
        if(!execute("UPDATE TradeOrder SET status='cancelled' WHERE id=?",{o["id"]})
        ||!execute("UPDATE Goods SET status='available' WHERE id=? AND status='reserved'",{o["goods_id"]})) return false;
    }
    return execute("UPDATE PurchaseRequest r JOIN Goods g ON g.id=r.goods_id SET r.status='rejected' WHERE r.status='pending' AND (r.buyer_id=? OR g.publisher_id=?)",{id,id})
    &&execute("UPDATE Goods SET is_deleted=1 WHERE publisher_id=?",{id})
    &&execute("UPDATE `User` SET status=1 WHERE id=?",{id});
}
bool DbManager::banUser(int id) {
    if(!isAdmin()||!m_db.transaction()) return false;
    if(!freezeUser(id)) return rollback();
    return commit();
}
bool DbManager::unbanUser(int id) {return isAdmin()&&execute("UPDATE `User` SET status=0 WHERE id=? AND role=1 AND status=1",{id},true);}
bool DbManager::removeUser(int id) {return banUser(id);}

bool DbManager::requestPurchase(int goodsId) {
    if(!isActive()||!m_db.transaction()) return false;
    auto g=rows("SELECT * FROM Goods WHERE id=? FOR UPDATE",{goodsId});
    if(g.isEmpty()||g[0]["is_deleted"].toInt()||g[0]["status"]!="available"||g[0]["publisher_id"].toInt()==m_currentUserId) {fail("商品不可购买或属于自己");return rollback();}
    if(!lockActiveUsers({m_currentUserId,g[0]["publisher_id"].toInt()})
    ||!execute("INSERT INTO PurchaseRequest(goods_id,buyer_id) VALUES(?,?)",{goodsId,m_currentUserId})) return rollback();
    return commit();
}
bool DbManager::confirmRequest(int requestId) {
    if(!isActive()||!m_db.transaction()) return false;
    auto r=rows("SELECT r.*,g.publisher_id,g.price,g.status goods_status,g.is_deleted FROM PurchaseRequest r JOIN Goods g ON g.id=r.goods_id WHERE r.id=? FOR UPDATE",{requestId});
    if(r.isEmpty()||r[0]["publisher_id"].toInt()!=m_currentUserId||r[0]["status"]!="pending"||r[0]["goods_status"]!="available"||r[0]["is_deleted"].toInt()) {fail("请求已失效、商品不可售或无权确认");return rollback();}
    auto v=r[0];
    if(!lockActiveUsers({m_currentUserId,v["buyer_id"].toInt()})
    ||!execute("INSERT INTO TradeOrder(request_id,goods_id,buyer_id,seller_id,amount) VALUES(?,?,?,?,?)",{requestId,v["goods_id"],v["buyer_id"],m_currentUserId,v["price"]})
    ||!execute("UPDATE Goods SET status='reserved' WHERE id=? AND status='available' AND is_deleted=0",{v["goods_id"]},true)
    ||!execute("UPDATE PurchaseRequest SET status='accepted' WHERE id=? AND status='pending'",{requestId},true)
    ||!execute("UPDATE PurchaseRequest SET status='rejected' WHERE goods_id=? AND status='pending' AND id<>?",{v["goods_id"],requestId})) return rollback();
    return commit();
}
bool DbManager::rejectRequest(int id) {
    if(!isActive()) return false;
    return execute("UPDATE PurchaseRequest r JOIN Goods g ON g.id=r.goods_id SET r.status=IF(r.buyer_id=?,'withdrawn','rejected') WHERE r.id=? AND r.status='pending' AND (r.buyer_id=? OR g.publisher_id=?)",{m_currentUserId,id,m_currentUserId,m_currentUserId},true);
}
bool DbManager::finishOrder(int id) {
    if(!isActive()||!m_db.transaction()) return false;
    auto r=rows("SELECT * FROM TradeOrder WHERE id=? AND buyer_id=? AND status='confirmed' FOR UPDATE",{id,m_currentUserId});
    if(r.isEmpty()) {fail("仅买家可确认收货，且订单必须待完成");return rollback();}
    auto o=r[0];
    if(!lockActiveUsers({m_currentUserId,o["seller_id"].toInt()})
    ||!execute("UPDATE Goods SET status='sold' WHERE id=? AND status='reserved' AND is_deleted=0",{o["goods_id"]},true)
    ||!execute("UPDATE TradeOrder SET status='completed',completed_at=NOW() WHERE id=? AND status='confirmed'",{id},true)
    ||!execute("INSERT INTO Review(order_id) VALUES(?)",{id})) return rollback();
    return commit();
}
bool DbManager::cancelOrder(int id) {
    if(!isActive()||!m_db.transaction()) return false;
    auto r=rows("SELECT * FROM TradeOrder WHERE id=? AND status='confirmed' AND (buyer_id=? OR seller_id=?) FOR UPDATE",{id,m_currentUserId,m_currentUserId});
    if(r.isEmpty()) {fail("订单不可取消或无权操作");return rollback();}
    if(!execute("UPDATE TradeOrder SET status='cancelled' WHERE id=? AND status='confirmed'",{id},true)
    ||!execute("UPDATE Goods SET status='available' WHERE id=? AND status='reserved'",{r[0]["goods_id"]},true)) return rollback();
    return commit();
}
bool DbManager::submitReview(int id,int rating,const QString &comment) {
    if(!isActive()||rating<1||rating>5||comment.size()>500) return fail("评分须为1—5，评价最多500字，且账户必须正常");
    auto r=rows("SELECT * FROM TradeOrder WHERE id=? AND status='completed' AND (buyer_id=? OR seller_id=?)",{id,m_currentUserId,m_currentUserId});
    if(r.isEmpty()) return fail("仅交易完成后的双方可评价");
    QString side=r[0]["buyer_id"].toInt()==m_currentUserId?"buyer":"seller";
    if(!m_db.transaction()) return false;
    if(!lockActiveUsers({r[0]["buyer_id"].toInt(),r[0]["seller_id"].toInt()})
    ||!execute(QString("UPDATE Review SET %1_rating=?,%1_comment=?,%1_reviewed_at=NOW() WHERE order_id=? AND %1_rating IS NULL").arg(side),{rating,comment,id},true)) return rollback();
    return commit();
}
bool DbManager::submitReport(const QString &type,int targetId,const QString &reason) {
    if(!isActive()) return false;
    if(reason.trimmed().isEmpty()||reason.size()>500) return fail("举报理由不能为空，且最多500字");
    if(type=="goods") {
        if(rows("SELECT id FROM Goods WHERE id=? AND publisher_id<>?",{targetId,m_currentUserId}).isEmpty()) return fail("商品不存在或属于自己");
        return execute("INSERT INTO Report(reporter_id,target_type,goods_id,reason) VALUES(?,'goods',?,?)",{m_currentUserId,targetId,reason.trimmed()});
    }
    if(type=="user"&&targetId!=m_currentUserId) return execute("INSERT INTO Report(reporter_id,target_type,user_id,reason) SELECT ?,'user',id,? FROM `User` WHERE id=? AND role=1",{m_currentUserId,reason.trimmed(),targetId},true);
    return fail("举报对象无效");
}
bool DbManager::handleReport(int id,bool uphold,const QString &note) {
    if(!isAdmin()) return false;
    if(note.trimmed().isEmpty()||note.size()>500) return fail("处理说明须为1—500字");
    if(!m_db.transaction()) return false;
    auto r=rows("SELECT * FROM Report WHERE id=? AND status='pending' FOR UPDATE",{id});
    if(r.isEmpty()) {fail("举报已经处理或不存在");return rollback();}
    if(uphold) {
        auto v=r[0];
        bool ok=v["target_type"]=="goods"?softDeleteGoods(v["goods_id"].toInt()):freezeUser(v["user_id"].toInt());
        if(!ok) return rollback();
    }
    if(!execute("UPDATE Report SET status=?,handler_id=?,result_note=?,handled_at=NOW() WHERE id=? AND status='pending'",{uphold?"resolved":"dismissed",m_currentUserId,note.trimmed(),id},true)) return rollback();
    return commit();
}
QList<QVariantMap> DbManager::myOrders() {
    if(!isActive()) return {};
    return rows("SELECT o.*,g.name goods_name,b.name buyer_name,s.name seller_name,r.buyer_rating,r.seller_rating FROM TradeOrder o JOIN Goods g ON g.id=o.goods_id JOIN `User` b ON b.id=o.buyer_id JOIN `User` s ON s.id=o.seller_id LEFT JOIN Review r ON r.order_id=o.id WHERE o.buyer_id=? OR o.seller_id=? ORDER BY o.id DESC",{m_currentUserId,m_currentUserId});
}
QList<QVariantMap> DbManager::myRequests() {
    if(!isActive()) return {};
    return rows("SELECT r.*,g.name goods_name,g.publisher_id,b.name buyer_name,s.name seller_name FROM PurchaseRequest r JOIN Goods g ON g.id=r.goods_id JOIN `User` b ON b.id=r.buyer_id JOIN `User` s ON s.id=g.publisher_id WHERE r.buyer_id=? OR g.publisher_id=? ORDER BY r.id DESC",{m_currentUserId,m_currentUserId});
}
QList<QVariantMap> DbManager::allReports() {
    if(!isAdmin()) return {};
    return rows("SELECT r.*,u.name reporter_name,COALESCE(g.name,t.name) target_name FROM Report r JOIN `User` u ON u.id=r.reporter_id LEFT JOIN Goods g ON g.id=r.goods_id LEFT JOIN `User` t ON t.id=r.user_id ORDER BY r.id DESC");
}
QList<QVariantMap> DbManager::myReports() {
    if(!isActive()) return {};
    return rows("SELECT r.*,COALESCE(g.name,t.name) target_name FROM Report r LEFT JOIN Goods g ON g.id=r.goods_id LEFT JOIN `User` t ON t.id=r.user_id WHERE reporter_id=? ORDER BY r.id DESC",{m_currentUserId});
}
QList<QVariantMap> DbManager::reportData(int index,int threshold) {
    if(!isAdmin()) return {};
    if(index==0) return rows("SELECT c.name 分类,COUNT(o.id) 成交数,ROUND(AVG(o.amount),2) 成交均价 FROM Category c JOIN Goods g ON g.category_id=c.id JOIN TradeOrder o ON o.goods_id=g.id WHERE o.status='completed' GROUP BY c.id,c.name ORDER BY 成交数 DESC");
    if(index==1) return rows("SELECT u.id 用户编号,u.name 卖家,u.credit_score 信用分,COUNT(DISTINCT g.id) 近30天发布数,COUNT(DISTINCT o.id) 成交数 FROM `User` u JOIN Goods g ON g.publisher_id=u.id LEFT JOIN TradeOrder o ON o.goods_id=g.id AND o.status='completed' WHERE u.role=1 AND u.status=0 AND u.credit_score<? AND g.created_at>=DATE_SUB(NOW(),INTERVAL 30 DAY) GROUP BY u.id,u.name,u.credit_score ORDER BY u.credit_score",{threshold});
    if(index==2) return rows("SELECT o.id 订单号,g.name 商品,c.name 分类,b.name 买家,s.name 卖家,o.amount 成交金额,o.status 订单状态,r.buyer_rating 买家评分,r.seller_rating 卖家评分 FROM TradeOrder o JOIN Goods g ON g.id=o.goods_id JOIN Category c ON c.id=g.category_id JOIN `User` b ON b.id=o.buyer_id JOIN `User` s ON s.id=o.seller_id LEFT JOIN Review r ON r.order_id=o.id ORDER BY o.id DESC");
    if(index==3) return rows("SELECT g.id 商品编号,g.name 商品,u.name 卖家,COUNT(DISTINCT r.reporter_id) 举报人数 FROM Report r JOIN Goods g ON g.id=r.goods_id JOIN `User` u ON u.id=g.publisher_id WHERE r.status='pending' GROUP BY g.id,g.name,u.name HAVING COUNT(DISTINCT r.reporter_id)>?",{threshold});
    return rows("SELECT u.id 用户编号,u.name 用户,u.credit_score 信用分,COUNT(DISTINCT r.reporter_id) 举报人数 FROM Report r JOIN `User` u ON u.id=r.user_id WHERE r.status='pending' GROUP BY u.id,u.name,u.credit_score HAVING COUNT(DISTINCT r.reporter_id)>?",{threshold});
}
bool DbManager::addMessage(int goodsId,int senderId,const QString &content) {
    if(!isActive()||senderId!=m_currentUserId||content.trimmed().isEmpty()||content.size()>1000) return fail("留言无效或无权操作");
    return execute("INSERT INTO Message(goods_id,sender_id,content) SELECT id,?,? FROM Goods WHERE id=? AND is_deleted=0",{senderId,content,goodsId},true);
}
QList<QVariantMap> DbManager::getMessagesForGoods(int id) {return rows("SELECT u.name username,m.content,m.timestamp time FROM Message m JOIN `User` u ON u.id=m.sender_id WHERE m.goods_id=? ORDER BY m.timestamp DESC",{id});}
QList<QVariantMap> DbManager::getMyReceivedMessages(int id) {
    if(!isActive()||id!=m_currentUserId) return {};
    return rows("SELECT m.content,m.timestamp time,g.name goods_name,u.name buyer_name,u.contact buyer_contact FROM Message m JOIN Goods g ON g.id=m.goods_id JOIN `User` u ON u.id=m.sender_id WHERE g.publisher_id=? ORDER BY m.timestamp DESC",{id});
}
QString DbManager::getPresetImagePath(const QString &name) {return ":/resources/images/preset/"+name;}
QString DbManager::getUploadImageDir() {QString d=QCoreApplication::applicationDirPath()+"/AppData/uploads";QDir().mkpath(d);return d;}
QString DbManager::saveUploadedImage(const QPixmap &pixmap) {
    QString relative="AppData/uploads/"+QString::number(QDateTime::currentMSecsSinceEpoch())+"_"+QString::number(QRandomGenerator::global()->generate())+".png";
    getUploadImageDir();return pixmap.save(QCoreApplication::applicationDirPath()+"/"+relative)?relative:"";
}
