#ifndef PRODUCTDETAIL_H
#define PRODUCTDETAIL_H

#include <QWidget>
#include <QVariantMap>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLineEdit>

class ProductDetail : public QWidget
{
    Q_OBJECT
public:
    explicit ProductDetail(QWidget *parent = nullptr);

    void setProductData(const QVariantMap &data);
    void setBackText(const QString &text);

signals:
    void goBackSignal();

private slots:
    void onSendMsg();

private:
    void initUi();
    void refreshMessages();

    // UI 组件
    QLabel *m_imgLabel;
    QLabel *m_priceLabel;
    QLabel *m_nameLabel;
    QLabel *m_publisherVal;
    QLabel *m_contactVal;
    QLabel *m_locationVal;
    QLabel *m_descLabel;

    QPushButton *m_btnBack;
    QPushButton *m_btnAdminDelete;
    QPushButton *m_btnCopy; // 👈 明确声明复制按钮

    // 留言板组件
    QVBoxLayout *m_msgDisplay;
    QLineEdit *m_msgInput;

    int m_currentGoodsId = -1;
    int m_sellerId=-1;
};

#endif // PRODUCTDETAIL_H
