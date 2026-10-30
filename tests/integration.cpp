#include "DbManager.h"
#include <QCoreApplication>
#include <QSqlQuery>
#include <QSqlError>
#include <QTextStream>
#include <cstdlib>

int main(int argc,char **argv) {
    QCoreApplication app(argc,argv);
    auto &db=DbManager::instance();int passed=0;
    auto check=[&](bool ok,const QString &label){
        QTextStream(stdout)<<(ok?"通过：":"失败：")<<label<<Qt::endl;
        if(!ok){QTextStream(stderr)<<db.lastError()<<Qt::endl;std::exit(1);}++passed;
    };
    check(db.isOpen(),"Qt QMYSQL 连接真实 MySQL");
    check(!db.addProduct("越权",1,"描述","九成新",1,""),"游客不能发布");
    check(db.registerUser("202699999999","注册测试","abcdef","测试联系方式"),"注册新用户");
    check(!db.registerUser("202699999999","重复学号","abcdef","测试联系方式"),"拒绝重复学号");
    check(db.loginUser("202699999999","abcdef"),"新注册账号可登录");
    check(db.loginUser("202600000001","123456"),"卖家登录及 PBKDF2 密码校验");
    int seller=db.getCurrentUserId();
    check(!db.loginUser("202600000001","错误密码"),"错误密码被拒绝且清除会话");
    check(db.loginUser("202600000001","123456"),"重新登录卖家");
    check(db.addProduct("测试平板交易",88.50,"测试平板电脑全文检索","九成新",1,""),"发布包含成色分类价格的商品");
    int goods=db.rows("SELECT MAX(id) id FROM Goods")[0]["id"].toInt();
    check(!db.requestPurchase(goods),"不能购买自己的商品");
    check(!db.getAllGoods("平板").isEmpty(),"中文 ngram 全文检索");
    check(db.loginUser("202600000002","123456"),"买家登录");int buyer=db.getCurrentUserId();
    check(db.requestPurchase(goods),"买家发起请求");
    check(!db.requestPurchase(goods),"同一买家重复待处理请求被唯一索引拒绝");
    int request=db.rows("SELECT MAX(id) id FROM PurchaseRequest")[0]["id"].toInt();
    check(!db.confirmRequest(request),"买家不能代卖家接受请求");
    check(db.loginUser("202600000003","123456")&&db.requestPurchase(goods),"第二买家也可发起购买请求");
    int other=db.rows("SELECT MAX(id) id FROM PurchaseRequest")[0]["id"].toInt();
    check(db.loginUser("202600000001","123456")&&db.confirmRequest(request),"卖家接受后生成订单并锁定库存");
    int firstOrder=db.rows("SELECT MAX(id) id FROM TradeOrder")[0]["id"].toInt();
    check(!db.confirmRequest(other),"其他请求不可重复成交");
    check(db.rows("SELECT status FROM Goods WHERE id=?",{goods})[0]["status"]=="reserved","商品库存状态变为预订");
    bool hidden=true;for(const auto &g:db.getAllGoods("测试平板交易"))if(g["id"].toInt()==goods)hidden=false;
    check(hidden,"预订商品不出现在可购买列表");
    check(!db.submitReview(firstOrder,5,"提前评价"),"未完成订单不能评价");
    check(!db.finishOrder(firstOrder),"卖家不能代买家确认收货");
    check(db.cancelOrder(firstOrder),"参与者取消订单并释放库存");
    check(db.rows("SELECT status FROM Goods WHERE id=?",{goods})[0]["status"]=="available","取消后库存恢复");
    check(db.loginUser("202600000002","123456")&&db.requestPurchase(goods),"取消后再次请求");
    request=db.rows("SELECT MAX(id) id FROM PurchaseRequest")[0]["id"].toInt();
    check(db.loginUser("202600000001","123456")&&db.confirmRequest(request),"同一商品生成第二笔尝试订单");
    int order=db.rows("SELECT MAX(id) id FROM TradeOrder")[0]["id"].toInt();
    check(db.loginUser("202600000002","123456")&&db.finishOrder(order),"买家收货完成交易");
    check(!db.finishOrder(order),"不能重复确认收货");
    check(!db.cancelOrder(order),"已完成订单不能取消");
    check(db.rows("SELECT status FROM Goods WHERE id=?",{goods})[0]["status"]=="sold","成交联动已售状态");
    check(db.submitReview(order,2,"如实评价"),"买家评价卖家");
    check(!db.submitReview(order,5,"重复评价"),"买家不能重复评价");
    check(db.credit(seller)==40,"触发器将卖家信用更新为40");
    check(db.loginUser("202600000001","123456")&&db.submitReview(order,4,"交易顺利"),"卖家评价买家");
    check(db.credit(buyer)==80,"触发器将买家信用更新为80");
    check(db.rows("SELECT COUNT(*) n FROM Review WHERE order_id=?",{order})[0]["n"].toInt()==1,"双向互评仍只有一条订单评价");
    QSqlQuery q;
    check(!q.exec(QString("INSERT INTO Review(order_id) VALUES(%1)").arg(order)),"数据库拒绝重复订单评价记录");
    check(!q.exec(QString("UPDATE Review SET buyer_rating=5 WHERE order_id=%1").arg(order)),"触发器拒绝改写已提交评分");
    check(!q.exec("INSERT INTO Report(reporter_id,target_type,reason) VALUES(2,'goods','无对象')"),"CHECK 拒绝无对象的举报");
    check(!q.exec("INSERT INTO Report(reporter_id,target_type,goods_id,reason) VALUES(2,'goods',999999,'不存在')"),"外键拒绝不存在的举报对象");
    check(!q.exec("DELETE FROM Goods WHERE id=1"),"应用数据库账号没有物理删除权限");
    check(!db.removeGoods(3),"卖家不能下架他人商品");
    check(db.loginUser("202600000002","123456")&&db.submitReport("goods",2,"违规描述"),"举报商品");
    check(!db.submitReport("goods",2,"重复举报"),"重复举报被拒绝");
    check(db.submitReport("user",seller,"用户违规"),"举报用户");
    check(db.loginUser("202600000003","123456")&&db.submitReport("goods",2,"违规描述")&&db.submitReport("user",seller,"用户违规"),"第二位举报人");
    check(db.loginUser("202600000004","123456")&&db.submitReport("goods",2,"违规描述")&&db.submitReport("user",seller,"用户违规"),"第三位举报人");
    check(db.reportData(0).isEmpty(),"普通用户无权访问管理员报表");
    check(db.loginUser("admin","admin123"),"管理员登录");
    for(int i=0;i<5;++i) check(!db.reportData(i,i==1?80:2).isEmpty(),QString("多表报表%1返回真实数据").arg(i+1));
    int rid=db.rows("SELECT MIN(id) id FROM Report WHERE target_type='goods'")[0]["id"].toInt();
    check(db.handleReport(rid,true,"管理员核实违规，下架"),"管理员处理举报下架商品");
    check(db.rows("SELECT is_deleted FROM Goods WHERE id=2")[0]["is_deleted"].toInt()==1,"商品软删除且实体保留");
    check(db.banUser(seller),"管理员冻结卖家");
    check(!db.loginUser("202600000001","123456"),"被冻结用户无法登录");
    check(db.loginUser("admin","admin123")&&db.unbanUser(seller),"管理员解冻用户");
    check(db.loginUser("202600000001","123456"),"解冻后可以登录");
    check(db.rows("SELECT is_deleted FROM Goods WHERE id=2")[0]["is_deleted"].toInt()==1,"解冻不自动恢复违规商品");
    check(db.rows("SELECT COUNT(*) n FROM TradeOrder WHERE goods_id=?",{goods})[0]["n"].toInt()==2,"保留商品与多笔历史订单的一对多关系");
    check(db.addProduct("冻结联动测试",15,"联动取消测试","八成新",2,""),"准备冻结联动商品");
    int fg=db.rows("SELECT MAX(id) id FROM Goods")[0]["id"].toInt();
    check(db.loginUser("202600000002","123456")&&db.requestPurchase(fg),"冻结前买家发起请求");
    int fr=db.rows("SELECT MAX(id) id FROM PurchaseRequest")[0]["id"].toInt();
    check(db.loginUser("202600000001","123456")&&db.confirmRequest(fr),"冻结前生成待收货订单");
    check(db.loginUser("admin","admin123")&&db.banUser(buyer),"管理员冻结买家并取消未完成交易");
    check(db.rows("SELECT status FROM Goods WHERE id=?",{fg})[0]["status"]=="available","冻结买家释放其他卖家的库存");
    check(db.unbanUser(buyer)&&db.loginUser("202600000002","123456"),"买家解冻重新登录");
    check(q.exec(QString("UPDATE `User` SET status=1 WHERE id=%1").arg(buyer)),"模拟其他会话冻结当前登录用户");
    check(!db.requestPurchase(fg)&&!db.addProduct("冻结越权",1,"测试","九成新",1,""),"已登录旧会话也被禁止交易与发布");
    check(db.loginUser("admin","admin123")&&db.unbanUser(buyer),"恢复测试账号");
    QTextStream(stdout)<<"全部通过："<<passed<<" 项"<<Qt::endl;
    return 0;
}
