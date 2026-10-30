#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QWidget>
#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVariantMap>

class HomePage : public QWidget
{
    Q_OBJECT
public:
    explicit HomePage(QWidget *parent = nullptr);

    // 刷新商品（查数据库）
    void refreshGoods(const QString &keyword = "", const QString &category = "全部");

signals:
    void logoutSignal();
    void goPublishSignal();
    void viewProductSignal(QVariantMap data);
    void goUserCenterSignal();

protected:
    // 监听窗口缩放，实现自适应排版
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupUi();
    void updateTitleBar();
    void updateGridLayout(); // 👈 补齐这个：负责重新排版卡片

    QWidget *m_scrollContent;
    QGridLayout *m_gridLayout;
    QLineEdit *m_searchEdit;
    QLabel *m_logoText;
    QString m_currentCategory = "全部";
    QString m_currentKeyword = "";

    // 按钮组
    QPushButton *m_btnPublish;
    QPushButton *m_btnUser;
    QPushButton *m_btnAdmin;
    QPushButton *m_btnExit;

    // 👈 补齐这个：缓存商品数据，防止缩放窗口时频繁查库
    QList<QVariantMap> m_currentGoodsData;
};

#endif // HOMEPAGE_H
