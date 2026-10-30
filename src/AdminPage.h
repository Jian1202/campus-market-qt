#ifndef ADMINPAGE_H
#define ADMINPAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QChartView>
#include <QPieSeries>
#include <QBarSeries>
#include <QBarSet>
#include "SideBar.h"


class AdminPage : public QWidget
{
    Q_OBJECT
public:
    explicit AdminPage(QWidget *parent = nullptr);
    void refreshData();
    void updateCharts();

signals:
    void goHomeSignal();

private:
    void setupUi();
    void setupGoodsTab();
    void setupUsersTab();
    QWidget* createStatCard(const QString &title, const QString &value, const QString &color);

    QTableWidget *m_goodsTable;
    QTableWidget *m_usersTable;
    QStackedWidget *m_contentStack;
    SideBar *m_sideBar;

    QLabel *m_totalGoodsLabel;
    QLabel *m_totalUsersLabel;

    QChartView *m_categoryChartView;  // 商品分类饼图
    QChartView *m_goodsChartView;     // 商品数量柱状图
    QChartView *m_priceChartView;     // 价格分布柱状图
};

#endif // ADMINPAGE_H
