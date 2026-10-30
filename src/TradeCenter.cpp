#include "TradeCenter.h"
#include "DbManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QTimer>

static QString state(const QString &s) {
    static const QMap<QString,QString> names={{"pending","待处理"},{"accepted","已接受"},{"rejected","已拒绝"},{"withdrawn","已撤回"},{"confirmed","待收货"},{"completed","已完成"},{"cancelled","已取消"},{"resolved","已处置"},{"dismissed","已驳回"}};
    return names.value(s,s);
}
static void prepare(QTableWidget *t,const QStringList &labels) {
    t->setColumnCount(labels.size());t->setHorizontalHeaderLabels(labels);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    t->horizontalHeader()->setStretchLastSection(false);
    t->verticalHeader()->setVisible(false);
    t->setShowGrid(false);
    t->setFrameShape(QFrame::NoFrame);
    t->setWordWrap(false);
    t->horizontalHeader()->setMinimumSectionSize(72);
    t->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    for(int i=0;i<labels.size();++i) {
        if(labels[i]=="商品" || labels[i]=="对象" || labels[i]=="理由" || labels[i]=="处理说明")
            t->horizontalHeader()->setSectionResizeMode(i,QHeaderView::Stretch);
    }
    for(int i=0;i<labels.size();++i) {
        if(labels[i]=="违规处置"||labels[i]=="驳回"||labels[i]=="确认收货"||labels[i]=="取消"||labels[i]=="评价"||labels[i]=="拒绝 / 撤回"||labels[i]=="接受并生成订单") {
            t->horizontalHeader()->setSectionResizeMode(i,QHeaderView::Fixed);
            t->setColumnWidth(i,labels[i]=="接受并生成订单"?150:124);
        }
    }
    t->verticalHeader()->setDefaultSectionSize(52);
    t->setAlternatingRowColors(true);
}
static void cell(QTableWidget *t,int row,int col,const QVariant &v) {
    t->setItem(row,col,new QTableWidgetItem(v.isNull()?"—":v.toString()));
}
TradeCenter::TradeCenter(QWidget *parent):QDialog(parent) {
    setWindowTitle("交易、信用与举报中心");resize(1100,720);setMinimumSize(860,580);
    setStyleSheet(R"(
        QDialog {background:#F5F5F7;color:#222;}
        QTabWidget::pane {background:white;border:1px solid #E5E7EB;border-radius:8px;top:-1px;}
        QTabBar::tab {background:transparent;color:#73777F;padding:12px 20px;border:none;border-bottom:2px solid transparent;}
        QTabBar::tab:selected {color:#202124;border-bottom:2px solid #202124;font-weight:600;}
        QTabBar::tab:hover {color:#202124;background:#ECEEF1;}
        QTableWidget {background:white;alternate-background-color:#FAFBFC;color:#30343B;border:none;selection-background-color:#EDF1F5;selection-color:#202124;}
        QTableWidget::item {padding:10px;border-bottom:1px solid #F0F1F3;}
        QHeaderView::section {padding:12px 10px;background:#F5F6F8;color:#73777F;border:none;font-weight:600;}
        QPushButton {min-height:30px;padding:2px 14px;color:#30343B;background:white;border:1px solid #DFE2E7;border-radius:6px;}
        QPushButton:hover {background:#F0F2F5;border-color:#B9BFC8;}
        QPushButton[role="action"] {background:#EFF2F5;border:none;color:#30343B;font-size:12px;}
        QPushButton[role="danger"] {background:#FFF0EF;color:#B8433A;border:none;font-size:12px;}
        QLabel {color:#454B54;background:transparent;}
        QComboBox,QSpinBox {background:white;color:#30343B;padding:8px 12px;border:1px solid #DFE2E7;border-radius:6px;min-height:24px;}
    )");
    auto layout=new QVBoxLayout(this);layout->setContentsMargins(24,22,24,24);layout->setSpacing(18);
    auto title=new QLabel("交易与信用中心");title->setStyleSheet("font-size:24px;font-weight:700;color:#202124;");layout->addWidget(title);
    auto bar=new QHBoxLayout;
    summary=new QLabel;bar->addWidget(summary);bar->addStretch();
    auto reload=new QPushButton("刷新");bar->addWidget(reload);layout->addLayout(bar);
    tabs=new QTabWidget;layout->addWidget(tabs);
    requests=new QTableWidget;requests->setObjectName("requestsTable");
    orders=new QTableWidget;orders->setObjectName("ordersTable");mine=new QTableWidget;
    prepare(requests,{"请求号","商品","买家","卖家","状态","接受并生成订单","拒绝 / 撤回"});
    prepare(orders,{"订单号","商品","买家","卖家","金额","状态","买家评分","卖家评分","确认收货","取消","评价"});
    prepare(mine,{"举报号","对象","理由","状态","处理说明"});
    tabs->addTab(requests,"购买请求");tabs->addTab(orders,"我的订单与互评");tabs->addTab(mine,"我的举报");
    if(DbManager::instance().isAdmin()) {
        reports=new QTableWidget;reports->setObjectName("reportsTable");
        prepare(reports,{"举报号","举报人","对象类型","对象","理由","状态","处理说明","违规处置","驳回"});tabs->addTab(reports,"举报处理");
        auto page=new QWidget;auto l=new QVBoxLayout(page);auto controls=new QHBoxLayout;
        reportChoice=new QComboBox;reportChoice->addItems({"各分类成交均价","低信用活跃卖家","订单交易明细","商品举报超过阈值","用户举报超过阈值"});
        threshold=new QSpinBox;threshold->setRange(0,100000);threshold->setValue(2);threshold->setEnabled(false);
        l->setContentsMargins(18,18,18,18);l->setSpacing(16);
        reportChoice->setMinimumWidth(220);threshold->setFixedWidth(120);
        controls->addWidget(reportChoice);controls->addStretch();controls->addWidget(new QLabel("筛选阈值"));controls->addWidget(threshold);l->addLayout(controls);
        stats=new QTableWidget;stats->setObjectName("statsTable");l->addWidget(stats);tabs->addTab(page,"多表统计报表");
        connect(reportChoice,&QComboBox::currentIndexChanged,this,[this](int i){threshold->setEnabled(i==1||i>=3);threshold->setValue(i==1?80:2);refresh();});
        connect(threshold,&QSpinBox::valueChanged,this,[this](){refresh();});
    }
    connect(reload,&QPushButton::clicked,this,[this](){refresh();});refresh();
}
void TradeCenter::action(QTableWidget *t,int row,int col,const QString &label,std::function<void()> callback) {
    auto container=new QWidget;container->setStyleSheet("background:transparent;");
    auto layout=new QHBoxLayout(container);layout->setContentsMargins(6,6,6,6);layout->setAlignment(Qt::AlignCenter);
    auto b=new QPushButton(label);b->setCursor(Qt::PointingHandCursor);b->setMinimumHeight(0);b->setFixedSize(label.size()>5?110:96,32);
    b->setProperty("role",label.contains("下架")||label.contains("取消")?"danger":"action");
    layout->addWidget(b);t->setCellWidget(row,col,container);connect(b,&QPushButton::clicked,this,std::move(callback));
}
void TradeCenter::result(bool ok) {
    if(!ok) QMessageBox::warning(this,"操作未完成",DbManager::instance().lastError());
    else QMessageBox::information(this,"完成","操作成功，数据已保存");
    QTimer::singleShot(0,this,[this](){refresh();});
}
void TradeCenter::refresh() {
    auto &db=DbManager::instance();int uid=db.getCurrentUserId();
    summary->setText(QString("%1  |  信用分 %2 / 100  |  交易完成后双方各评一次").arg(db.getUserName(uid)).arg(db.credit(uid),0,'f',2));
    auto req=db.myRequests();requests->setRowCount(0);requests->setRowCount(req.size());
    for(int i=0;i<req.size();++i) {
        auto r=req[i];int id=r["id"].toInt();cell(requests,i,0,id);cell(requests,i,1,r["goods_name"]);cell(requests,i,2,r["buyer_name"]);cell(requests,i,3,r["seller_name"]);cell(requests,i,4,state(r["status"].toString()));
        if(r["status"]=="pending") {
            if(r["publisher_id"].toInt()==uid) action(requests,i,5,"接受请求",[this,id](){result(DbManager::instance().confirmRequest(id));});
            action(requests,i,6,r["buyer_id"].toInt()==uid?"撤回请求":"拒绝请求",[this,id](){result(DbManager::instance().rejectRequest(id));});
        }
    }
    auto ord=db.myOrders();orders->setRowCount(0);orders->setRowCount(ord.size());
    for(int i=0;i<ord.size();++i) {
        auto o=ord[i];int id=o["id"].toInt();cell(orders,i,0,id);cell(orders,i,1,o["goods_name"]);cell(orders,i,2,o["buyer_name"]);cell(orders,i,3,o["seller_name"]);cell(orders,i,4,QString::number(o["amount"].toDouble(),'f',2));cell(orders,i,5,state(o["status"].toString()));cell(orders,i,6,o["buyer_rating"]);cell(orders,i,7,o["seller_rating"]);
        if(o["status"]=="confirmed") {
            if(o["buyer_id"].toInt()==uid) action(orders,i,8,"确认收货",[this,id](){result(DbManager::instance().finishOrder(id));});
            action(orders,i,9,"取消订单",[this,id](){result(DbManager::instance().cancelOrder(id));});
        }
        QString key=o["buyer_id"].toInt()==uid?"buyer_rating":"seller_rating";
        if(o["status"]=="completed"&&o[key].isNull()) action(orders,i,10,"评价对方",[this,id](){
            bool ok=false;int score=QInputDialog::getInt(this,"评价对方","评分（1—5）",5,1,5,1,&ok);if(!ok) return;
            QString text=QInputDialog::getMultiLineText(this,"评价对方","评价内容（可留空，最多500字）","",&ok);if(ok) result(DbManager::instance().submitReview(id,score,text));
        });
    }
    auto mr=db.myReports();mine->setRowCount(mr.size());
    for(int i=0;i<mr.size();++i){auto r=mr[i];cell(mine,i,0,r["id"]);cell(mine,i,1,r["target_name"]);cell(mine,i,2,r["reason"]);cell(mine,i,3,state(r["status"].toString()));cell(mine,i,4,r["result_note"]);}
    if(!reports) return;
    auto rs=db.allReports();reports->setRowCount(0);reports->setRowCount(rs.size());
    for(int i=0;i<rs.size();++i) {
        auto r=rs[i];int id=r["id"].toInt();cell(reports,i,0,id);cell(reports,i,1,r["reporter_name"]);cell(reports,i,2,r["target_type"]=="goods"?"商品":"用户");cell(reports,i,3,r["target_name"]);cell(reports,i,4,r["reason"]);cell(reports,i,5,state(r["status"].toString()));cell(reports,i,6,r["result_note"]);
        if(r["status"]=="pending") for(bool uphold:{true,false}) action(reports,i,uphold?7:8,uphold?"下架 / 冻结":"驳回",[this,id,uphold](){
            bool ok;QString note=QInputDialog::getMultiLineText(this,"处理举报",uphold?"确认违规后将下架商品或冻结用户；填写处理说明":"填写驳回说明","",&ok);
            if(ok&&!note.trimmed().isEmpty()) result(DbManager::instance().handleReport(id,uphold,note));
        });
    }
    auto data=db.reportData(reportChoice->currentIndex(),threshold->value());stats->clear();stats->setRowCount(0);
    if(data.isEmpty()) {prepare(stats,{"查询结果"});stats->setRowCount(1);cell(stats,0,0,"当前没有符合条件的数据");}
    else {QStringList keys;
        switch(reportChoice->currentIndex()) {
        case 0:keys={"分类","成交数","成交均价"};break;
        case 1:keys={"用户编号","卖家","信用分","近30天发布数","成交数"};break;
        case 2:keys={"订单号","商品","分类","买家","卖家","成交金额","订单状态","买家评分","卖家评分"};break;
        case 3:keys={"商品编号","商品","卖家","举报人数"};break;
        default:keys={"用户编号","用户","信用分","举报人数"};break;
        }
        prepare(stats,keys);for(int col=0;col<keys.size();++col)stats->horizontalHeader()->setSectionResizeMode(col,QHeaderView::Stretch);stats->setRowCount(data.size());for(int i=0;i<data.size();++i)for(int j=0;j<keys.size();++j)cell(stats,i,j,data[i][keys[j]]);}
}
