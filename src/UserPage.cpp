#include "UserPage.h"
#include "DbManager.h"
#include "Toast.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>
#include <QGraphicsDropShadowEffect>
#include <QDateTime>

UserPage::UserPage(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void UserPage::setupUi()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 1. 侧边栏
    m_sideBar = new SideBar(this);
    m_sideBar->addAction("概览工作台", 0);
    m_sideBar->addAction("我的商品", 1);
    m_sideBar->addAction("咨询消息", 2);
    m_sideBar->addAction("返回首页", 3, true);
    m_sideBar->setChecked(0);
    mainLayout->addWidget(m_sideBar);

    // 2. 右侧内容区
    m_contentStack = new QStackedWidget(this);
    m_contentStack->setStyleSheet("background-color: #F5F5F7;");
    mainLayout->addWidget(m_contentStack);

    // --- 页面0: 概览工作台 ---
    QWidget *dashPage = new QWidget();
    QVBoxLayout *dashLayout = new QVBoxLayout(dashPage);
    dashLayout->setContentsMargins(40, 40, 40, 40);
    dashLayout->setSpacing(30);

    m_userNameLabel = new QLabel("欢迎回来，用户");
    m_userNameLabel->setStyleSheet("font-size: 24px; font-weight: 900; color: #1A1A1A;");
    dashLayout->addWidget(m_userNameLabel);

    // 统计卡片
    QHBoxLayout *statLayout = new QHBoxLayout();
    statLayout->setSpacing(20);

    m_goodsCountLabel = new QLabel("0");
    m_msgCountLabel = new QLabel("0");
    m_unreadMsgLabel = new QLabel("0");

    statLayout->addWidget(createStatCard("在售商品", "0", "#1A1A1A"));
    statLayout->addWidget(createStatCard("收到咨询", "0", "#FF7875"));
    statLayout->addWidget(createStatCard("待完成订单", "0", "#FAAD14"));
    statLayout->addStretch();
    dashLayout->addLayout(statLayout);

    // 最近浏览的商品
    QLabel *recentTitle = new QLabel("交易说明：点击顶部“交易 / 信用 / 举报 / 报表”处理请求、订单与互评");
    recentTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #1A1A1A; margin-top: 20px;");
    dashLayout->addWidget(recentTitle);

    m_recentViewTable = new QTableWidget(0, 3);
    m_recentViewTable->setHorizontalHeaderLabels({"商品名称", "价格", "浏览时间"});
    m_recentViewTable->setMaximumHeight(200);
    m_recentViewTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_recentViewTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_recentViewTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_recentViewTable->setStyleSheet("QTableWidget { background-color: white; border-radius: 12px; border: none; gridline-color: #F0F0F0; } QHeaderView::section { background-color: #FAFAFA; border: none; height: 40px; font-weight: bold; }");
    dashLayout->addWidget(m_recentViewTable);

    dashLayout->addStretch();
    m_contentStack->addWidget(dashPage);

    // --- 页面1: 我的商品 ---
    QWidget *goodsPage = new QWidget();
    QVBoxLayout *goodsLayout = new QVBoxLayout(goodsPage);
    goodsLayout->setContentsMargins(40, 40, 40, 40);

    QLabel *gTitle = new QLabel("管理我的商品");
    gTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #1A1A1A; margin-bottom: 10px;");
    goodsLayout->addWidget(gTitle);

    m_goodsTable = new QTableWidget(0, 4);
    m_goodsTable->setHorizontalHeaderLabels({"ID", "商品名称", "价格", "操作"});
    m_goodsTable->setColumnWidth(0, 60);
    m_goodsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_goodsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_goodsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_goodsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_goodsTable->setStyleSheet("QTableWidget { background-color: white; border-radius: 12px; border: none; gridline-color: #F0F0F0; } QHeaderView::section { background-color: #FAFAFA; border: none; height: 40px; font-weight: bold; }");
    goodsLayout->addWidget(m_goodsTable);
    m_contentStack->addWidget(goodsPage);

    // --- 页面2: 咨询消息 ---
    QWidget *msgPage = new QWidget();
    QVBoxLayout *msgLayout = new QVBoxLayout(msgPage);
    msgLayout->setContentsMargins(40, 40, 40, 40);

    QLabel *mTitle = new QLabel("买家咨询消息");
    mTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #1A1A1A; margin-bottom: 10px;");
    msgLayout->addWidget(mTitle);

    m_msgTable = new QTableWidget(0, 3);
    m_msgTable->setHorizontalHeaderLabels({"咨询人", "咨询内容", "商品与联系方式"});
    m_msgTable->setColumnWidth(0, 80);
    m_msgTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_msgTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_msgTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_msgTable->verticalHeader()->setDefaultSectionSize(80);
    m_msgTable->setStyleSheet("QTableWidget { background-color: white; border-radius: 12px; border: none; gridline-color: #F0F0F0; } QHeaderView::section { background-color: #FAFAFA; border: none; height: 40px; font-weight: bold; }");
    msgLayout->addWidget(m_msgTable);
    m_contentStack->addWidget(msgPage);

    // 信号连接
    connect(m_sideBar, &SideBar::actionTriggered, this, [this](int id){
        if (id == 3) emit goBackSignal();
        else m_contentStack->setCurrentIndex(id);
    });
}

QWidget* UserPage::createStatCard(const QString &title, const QString &value, const QString &color)
{
    QWidget *card = new QWidget();
    card->setFixedSize(200, 120);
    card->setStyleSheet("background-color: white; border-radius: 16px;");

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 15));
    shadow->setOffset(0, 4);
    card->setGraphicsEffect(shadow);

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 20, 20, 20);

    QLabel *lblTitle = new QLabel(title);
    lblTitle->setStyleSheet("color: #999; font-size: 14px;");

    QLabel *lblVal = new QLabel(value);
    lblVal->setStyleSheet(QString("color: %1; font-size: 32px; font-weight: 900;").arg(color));

    // 缓存引用以便更新
    if (title == "在售商品") m_goodsCountLabel = lblVal;
    else if (title == "收到咨询") m_msgCountLabel = lblVal;
    else if (title == "待完成订单") m_unreadMsgLabel = lblVal;

    layout->addWidget(lblTitle);
    layout->addWidget(lblVal);
    return card;
}

void UserPage::refreshData()
{
    int uid = DbManager::instance().getCurrentUserId();
    m_userNameLabel->setText(QString("欢迎回来，%1").arg(DbManager::instance().getUserName(uid)));

    // 1. 刷新商品
    auto goods = DbManager::instance().getMyGoods(uid);
    m_goodsCountLabel->setText(QString::number(goods.size()));
    m_goodsTable->setRowCount(0);
    for (const auto &g : goods) {
        int row = m_goodsTable->rowCount();
        m_goodsTable->insertRow(row);
        m_goodsTable->setItem(row, 0, new QTableWidgetItem(g["id"].toString()));
        m_goodsTable->setItem(row, 1, new QTableWidgetItem(g["name"].toString()));
        m_goodsTable->setItem(row, 2, new QTableWidgetItem(g["price"].toString()));

        QPushButton *btnDel = new QPushButton("下架商品");
        btnDel->setCursor(Qt::PointingHandCursor);
        btnDel->setStyleSheet("QPushButton { color: #FF4D4F; border: none; font-weight: bold; } QPushButton:hover { text-decoration: underline; }");
        connect(btnDel, &QPushButton::clicked, this, [=](){
            if(DbManager::instance().removeGoods(g["id"].toInt())) {
                Toast::show(this->window(), "已成功下架商品");
                refreshData();
            }
        });
        m_goodsTable->setCellWidget(row, 3, btnDel);
    }

    // 2. 刷新消息
    auto msgs = DbManager::instance().getMyReceivedMessages(uid);
    m_msgCountLabel->setText(QString::number(msgs.size()));

    int pending=0;
    for(const auto &o:DbManager::instance().myOrders()) if(o["status"]=="confirmed") ++pending;
    m_unreadMsgLabel->setText(QString::number(pending));

    m_msgTable->setRowCount(0);
    for (const auto &m : msgs) {
        int row = m_msgTable->rowCount();
        m_msgTable->insertRow(row);

        m_msgTable->setItem(row, 0, new QTableWidgetItem(m["buyer_name"].toString()));
        m_msgTable->setItem(row, 1, new QTableWidgetItem(m["content"].toString()));

        QString buyerInfo = QString("商品：%1\n联系：%2").arg(m["goods_name"].toString(), m["buyer_contact"].toString());
        m_msgTable->setItem(row, 2, new QTableWidgetItem(buyerInfo));
    }

    // 订单在交易中心集中处理。
    m_recentViewTable->setRowCount(0);
    m_recentViewTable->hide();
}
