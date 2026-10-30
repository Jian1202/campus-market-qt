#include "AdminPage.h"
#include "DbManager.h"
#include "Toast.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QChart>
#include <QChartView>
#include <QPieSeries>
#include <QPieSlice>
#include <QBarSeries>
#include <QBarSet>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QSqlQuery>
#include <QColor>
#include <QFont>
#include <QScrollArea>
#include <algorithm>

AdminPage::AdminPage(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void AdminPage::setupUi()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 1. 侧边栏
    m_sideBar = new SideBar(this);
    m_sideBar->addAction("全站概览", 0);
    m_sideBar->addAction("商品管理", 1);
    m_sideBar->addAction("用户管理", 2);
    m_sideBar->addAction("返回首页", 3, true);
    m_sideBar->setChecked(0);
    mainLayout->addWidget(m_sideBar);

    // 2. 内容区
    m_contentStack = new QStackedWidget(this);
    m_contentStack->setStyleSheet("background-color: #F5F5F7;");
    mainLayout->addWidget(m_contentStack);

    // --- 页面0: 全站概览 ---
    QWidget *dashPage = new QWidget();
    QVBoxLayout *dashLayout = new QVBoxLayout(dashPage);
    dashLayout->setContentsMargins(28, 28, 28, 28);
    dashLayout->setSpacing(20);

    QLabel *title = new QLabel("管理控制台");
    title->setStyleSheet("font-size: 24px; font-weight: 900; color: #1A1A1A;");
    dashLayout->addWidget(title);

    QHBoxLayout *statLayout = new QHBoxLayout();
    statLayout->setSpacing(20);

    m_totalGoodsLabel = new QLabel("0");
    m_totalUsersLabel = new QLabel("0");

    statLayout->addWidget(createStatCard("全站商品总数", "0", "#1A1A1A"));
    statLayout->addWidget(createStatCard("注册用户总数", "0", "#1A1A1A"));
    statLayout->addStretch();
    dashLayout->addLayout(statLayout);

    // 图表区域
    QHBoxLayout *chartLayout1 = new QHBoxLayout();
    chartLayout1->setSpacing(20);

    // 商品分类饼图
    m_categoryChartView = new QChartView();
    m_categoryChartView->setRenderHint(QPainter::Antialiasing);
    m_categoryChartView->setStyleSheet("background-color: white; border-radius: 12px;");
    chartLayout1->addWidget(m_categoryChartView);

    // 商品数量柱状图
    m_goodsChartView = new QChartView();
    m_goodsChartView->setRenderHint(QPainter::Antialiasing);
    m_goodsChartView->setStyleSheet("background-color: white; border-radius: 12px;");
    chartLayout1->addWidget(m_goodsChartView);

    dashLayout->addLayout(chartLayout1);

    // 图表区域 - 第二行
    QHBoxLayout *chartLayout2 = new QHBoxLayout();
    chartLayout2->setSpacing(20);

    // 价格分布柱状图
    m_priceChartView = new QChartView();
    m_priceChartView->setRenderHint(QPainter::Antialiasing);
    m_priceChartView->setStyleSheet("background-color: white; border-radius: 12px;");
    chartLayout2->addWidget(m_priceChartView);


    dashLayout->addLayout(chartLayout2);
    dashLayout->addStretch();
    for(auto chart:{m_categoryChartView,m_goodsChartView,m_priceChartView}) chart->setMinimumHeight(260);
    auto dashboardScroll=new QScrollArea;dashboardScroll->setWidgetResizable(true);dashboardScroll->setFrameShape(QFrame::NoFrame);dashboardScroll->setWidget(dashPage);
    m_contentStack->addWidget(dashboardScroll);

    // --- 页面1: 商品管理 ---
    QWidget *goodsPage = new QWidget();
    QVBoxLayout *goodsLayout = new QVBoxLayout(goodsPage);
    goodsLayout->setContentsMargins(40, 40, 40, 40);

    QLabel *gTitle = new QLabel("全站商品管理");
    gTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #1A1A1A; margin-bottom: 10px;");
    goodsLayout->addWidget(gTitle);

    m_goodsTable = new QTableWidget();
    m_goodsTable->setStyleSheet("QTableWidget { background-color: white; border-radius: 12px; border: none; gridline-color: #F0F0F0; } QHeaderView::section { background-color: #FAFAFA; border: none; height: 40px; font-weight: bold; }");
    goodsLayout->addWidget(m_goodsTable);
    m_contentStack->addWidget(goodsPage);

    // --- 页面2: 用户管理 ---
    QWidget *usersPage = new QWidget();
    QVBoxLayout *usersLayout = new QVBoxLayout(usersPage);
    usersLayout->setContentsMargins(40, 40, 40, 40);

    QLabel *uTitle = new QLabel("全站用户管理");
    uTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #1A1A1A; margin-bottom: 10px;");
    usersLayout->addWidget(uTitle);

    m_usersTable = new QTableWidget();
    m_usersTable->setStyleSheet("QTableWidget { background-color: white; border-radius: 12px; border: none; gridline-color: #F0F0F0; } QHeaderView::section { background-color: #FAFAFA; border: none; height: 40px; font-weight: bold; }");
    usersLayout->addWidget(m_usersTable);
    m_contentStack->addWidget(usersPage);

    // 信号连接
    connect(m_sideBar, &SideBar::actionTriggered, this, [this](int id){
        if (id == 3) emit goHomeSignal();
        else m_contentStack->setCurrentIndex(id);
    });
}

QWidget* AdminPage::createStatCard(const QString &title, const QString &value, const QString &color)
{
    QWidget *card = new QWidget();
    card->setFixedSize(240, 120);
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

    if (title == "全站商品总数") m_totalGoodsLabel = lblVal;
    else if (title == "注册用户总数") m_totalUsersLabel = lblVal;

    layout->addWidget(lblTitle);
    layout->addWidget(lblVal);
    return card;
}

void AdminPage::refreshData()
{
    setupGoodsTab();
    setupUsersTab();

    m_totalGoodsLabel->setText(QString::number(m_goodsTable->rowCount()));
    m_totalUsersLabel->setText(QString::number(m_usersTable->rowCount()));

    // 刷新图表
    updateCharts();
}

void AdminPage::updateCharts()
{
    // 1. 商品分类饼图
    QMap<QString, int> categoryCount;
    QSqlQuery query;
    query.exec("SELECT c.name category,COUNT(*) count FROM Goods g JOIN Category c ON c.id=g.category_id WHERE g.is_deleted=0 GROUP BY c.id,c.name");
    while (query.next()) {
        QString category = query.value("category").toString();
        int count = query.value("count").toInt();
        categoryCount[category] = count;
    }

    // 创建饼图
    QPieSeries *pieSeries = new QPieSeries();
    QStringList colors = {"#637B87", "#8BA7A0", "#9EAFC1", "#C2AF97", "#B4C7BC", "#C9C6AD", "#A8A5BB", "#9CB9C2"};
    int colorIndex = 0;

    for (auto it = categoryCount.begin(); it != categoryCount.end(); ++it) {
        QPieSlice *slice = pieSeries->append(it.key(), it.value());
        slice->setColor(QColor(colors[colorIndex % colors.size()]));
        slice->setLabelVisible(true);
        slice->setLabel(QString("%1 (%2)").arg(it.key()).arg(it.value()));
        colorIndex++;
    }

    QChart *categoryChart = new QChart();
    categoryChart->addSeries(pieSeries);
    categoryChart->setTitle("商品分类分布");
    categoryChart->setAnimationOptions(QChart::NoAnimation);
    categoryChart->setBackgroundBrush(QBrush(Qt::white));
    categoryChart->setTitleBrush(QBrush(QColor("#1A1A1A")));

    categoryChart->legend()->hide();
    categoryChart->setMargins(QMargins(12,8,12,8));
    pieSeries->setPieSize(0.65);
    m_categoryChartView->setChart(categoryChart);

    // 2. 商品数量柱状图（按发布人统计前5名）
    QMap<QString, int> publisherCount;
    QSqlQuery query2;
    query2.exec("SELECT u.name, COUNT(g.id) as count FROM Goods g JOIN User u ON g.publisher_id = u.id GROUP BY g.publisher_id ORDER BY count DESC LIMIT 5");
    while (query2.next()) {
        QString name = query2.value("name").toString();
        int count = query2.value("count").toInt();
        publisherCount[name] = count;
    }

    // 创建柱状图
    QBarSet *barSet = new QBarSet("商品数量");
    barSet->setColor(QColor("#1A1A1A"));

    QStringList categories;
    for (auto it = publisherCount.begin(); it != publisherCount.end(); ++it) {
        *barSet << it.value();
        categories << it.key();
    }

    QBarSeries *barSeries = new QBarSeries();
    barSeries->append(barSet);

    QChart *goodsChart = new QChart();
    goodsChart->addSeries(barSeries);
    goodsChart->setTitle("发布人商品数量TOP 5");
    goodsChart->setAnimationOptions(QChart::NoAnimation);
    goodsChart->setBackgroundBrush(QBrush(Qt::white));
    goodsChart->setTitleBrush(QBrush(QColor("#1A1A1A")));

    // 设置坐标轴
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    goodsChart->addAxis(axisX, Qt::AlignBottom);
    barSeries->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    goodsChart->addAxis(axisY, Qt::AlignLeft);
    barSeries->attachAxis(axisY);

    goodsChart->legend()->hide();goodsChart->setMargins(QMargins(8,8,8,8));
    int highestPublisher=1;for(auto count:publisherCount)highestPublisher=std::max(highestPublisher,count);
    axisY->setRange(0,highestPublisher+1);axisY->setLabelFormat("%d");axisY->setTickCount(std::min(highestPublisher+2,6));
    axisX->setLabelsFont(QFont("Microsoft YaHei UI",9));
    m_goodsChartView->setChart(goodsChart);

    // 3. 价格分布柱状图
    QMap<QString, int> priceRangeCount;
    QSqlQuery query3;
    query3.exec("SELECT price FROM Goods WHERE is_deleted=0");

    // 统计价格分布
    while (query3.next()) {
        double price = query3.value("price").toDouble();
        QString range;
        if (price < 50) range = "0-50";
        else if (price < 100) range = "50-100";
        else if (price < 200) range = "100-200";
        else if (price < 500) range = "200-500";
        else if (price < 1000) range = "500-1000";
        else range = "1000+";

        priceRangeCount[range]++;
    }

    // 创建价格分布柱状图
    QBarSet *priceSet = new QBarSet("商品数量");
    priceSet->setColor(QColor("#718791"));

    QStringList priceCategories = {"0-50", "50-100", "100-200", "200-500", "500-1000", "1000+"};
    for (const QString &range : priceCategories) {
        *priceSet << priceRangeCount[range];
    }

    QBarSeries *priceSeries = new QBarSeries();
    priceSeries->append(priceSet);

    QChart *priceChart = new QChart();
    priceChart->addSeries(priceSeries);
    priceChart->setTitle("商品价格分布");
    priceChart->setAnimationOptions(QChart::NoAnimation);
    priceChart->setBackgroundBrush(QBrush(Qt::white));
    priceChart->setTitleBrush(QBrush(QColor("#1A1A1A")));

    // 设置坐标轴
    QBarCategoryAxis *priceAxisX = new QBarCategoryAxis();
    priceAxisX->append(priceCategories);
    priceChart->addAxis(priceAxisX, Qt::AlignBottom);
    priceSeries->attachAxis(priceAxisX);

    QValueAxis *priceAxisY = new QValueAxis();
    priceChart->addAxis(priceAxisY, Qt::AlignLeft);
    priceSeries->attachAxis(priceAxisY);

    priceChart->legend()->hide();priceChart->setMargins(QMargins(8,8,8,8));
    int highestPrice=1;for(auto count:priceRangeCount)highestPrice=std::max(highestPrice,count);
    priceAxisY->setRange(0,highestPrice+1);priceAxisY->setLabelFormat("%d");priceAxisY->setTickCount(std::min(highestPrice+2,6));
    priceAxisX->setLabelsFont(QFont("Microsoft YaHei UI",9));
    m_priceChartView->setChart(priceChart);
}

void AdminPage::setupGoodsTab()
{
    m_goodsTable->clear();
    m_goodsTable->setColumnCount(5);
    m_goodsTable->setHorizontalHeaderLabels({"ID", "商品名称", "价格", "发布人", "操作"});
    m_goodsTable->setColumnWidth(0, 60);
    m_goodsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_goodsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_goodsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_goodsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_goodsTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_goodsTable->verticalHeader()->setVisible(false);
    m_goodsTable->verticalHeader()->setDefaultSectionSize(50);
    m_goodsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QList<QVariantMap> goods = DbManager::instance().getAllGoods();
    m_goodsTable->setRowCount(goods.size());

    for(int i = 0; i < goods.size(); ++i) {
        QVariantMap data = goods[i];
        m_goodsTable->setItem(i, 0, new QTableWidgetItem(data["id"].toString()));
        m_goodsTable->setItem(i, 1, new QTableWidgetItem(data["name"].toString()));

        QTableWidgetItem *priceItem = new QTableWidgetItem(data["price"].toString());
        priceItem->setForeground(QColor("#1A1A1A"));
        priceItem->setFont(QFont("Arial", 10, QFont::Bold));
        m_goodsTable->setItem(i, 2, priceItem);

        m_goodsTable->setItem(i, 3, new QTableWidgetItem(data["publisher"].toString()));

        QPushButton *btnDel = new QPushButton("强制下架");
        btnDel->setCursor(Qt::PointingHandCursor);
        btnDel->setStyleSheet("QPushButton { color: #FF4D4F; border: none; font-weight: bold; } QPushButton:hover { text-decoration: underline; }");
        connect(btnDel, &QPushButton::clicked, this, [=](){
            if(QMessageBox::Yes == QMessageBox::question(this, "警告", "确认下架此商品？历史数据将保留。")) {
                DbManager::instance().removeGoods(data["id"].toInt());
                Toast::show(this->window(), "已强制下架商品");
                refreshData();
            }
        });
        m_goodsTable->setCellWidget(i, 4, btnDel);
    }
}

void AdminPage::setupUsersTab()
{
    m_usersTable->clear();
    m_usersTable->setColumnCount(5);
    m_usersTable->setHorizontalHeaderLabels({"ID", "姓名", "联系方式", "状态", "操作"});
    m_usersTable->setColumnWidth(0, 60);
    m_usersTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_usersTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_usersTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_usersTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_usersTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_usersTable->verticalHeader()->setVisible(false);
    m_usersTable->verticalHeader()->setDefaultSectionSize(50);
    m_usersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QList<QVariantMap> users = DbManager::instance().getAllUsers();
    m_usersTable->setRowCount(users.size());

    for(int i = 0; i < users.size(); ++i) {
        QVariantMap data = users[i];
        m_usersTable->setItem(i, 0, new QTableWidgetItem(data["id"].toString()));
        m_usersTable->setItem(i, 1, new QTableWidgetItem(data["name"].toString()));
        m_usersTable->setItem(i, 2, new QTableWidgetItem(data["contact"].toString()));

        // 获取用户状态
        UserStatus status = DbManager::instance().getUserStatus(data["id"].toInt());
        QString statusText = (status == StatusBanned) ? "已封禁" : "正常";
        QTableWidgetItem *statusItem = new QTableWidgetItem(statusText);
        if (status == StatusBanned) {
            statusItem->setForeground(QColor("#FF4D4F"));
            statusItem->setFont(QFont("Arial", 10, QFont::Bold));
        }
        m_usersTable->setItem(i, 3, statusItem);

        UserStatus userStatus = DbManager::instance().getUserStatus(data["id"].toInt());
        QString btnText = (userStatus == StatusBanned) ? "解封用户" : "封禁用户";
        QString btnColor = (userStatus == StatusBanned) ? "#52C41A" : "#FF7875";

        QPushButton *btnAction = new QPushButton(btnText);
        btnAction->setCursor(Qt::PointingHandCursor);
        btnAction->setStyleSheet(QString("QPushButton { color: %1; border: none; font-weight: bold; } QPushButton:hover { text-decoration: underline; }").arg(btnColor));

        connect(btnAction, &QPushButton::clicked, this, [=](){
            int userId = data["id"].toInt();
            UserStatus currentStatus = DbManager::instance().getUserStatus(userId);

            if (currentStatus == StatusBanned) {
                if(QMessageBox::Yes == QMessageBox::question(this, "确认", "确认解封该用户？")) {
                    DbManager::instance().unbanUser(userId);
                    Toast::show(this->window(), "已解封用户");
                    refreshData();
                }
            } else {
                if(QMessageBox::Yes == QMessageBox::question(this, "警告", "确认封禁该用户？")) {
                    DbManager::instance().banUser(userId);
                    Toast::show(this->window(), "已封禁用户");
                    refreshData();
                }
            }
        });
        m_usersTable->setCellWidget(i, 4, btnAction);
    }
}
