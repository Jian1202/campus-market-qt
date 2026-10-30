#ifndef USERPAGE_H
#define USERPAGE_H

#include <QWidget>
#include <QVariantMap>
#include <QLabel>
#include <QTableWidget>
#include <QStackedWidget>
#include "SideBar.h"

class UserPage : public QWidget
{
    Q_OBJECT
public:
    explicit UserPage(QWidget *parent = nullptr);
    void refreshData();

signals:
    void goBackSignal();
    void viewProductSignal(QVariantMap data);

private:
    void setupUi();
    QWidget* createStatCard(const QString &title, const QString &value, const QString &color);

    QLabel *m_userNameLabel;
    QLabel *m_goodsCountLabel;
    QLabel *m_msgCountLabel;
    QLabel *m_unreadMsgLabel;
    QTableWidget *m_goodsTable;
    QTableWidget *m_msgTable;
    QTableWidget *m_recentViewTable;
    QStackedWidget *m_contentStack;
    SideBar *m_sideBar;
};

#endif // USERPAGE_H
