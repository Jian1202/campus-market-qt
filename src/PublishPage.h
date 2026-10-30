#ifndef PUBLISHPAGE_H
#define PUBLISHPAGE_H

#include <QWidget>

class PublishPage : public QWidget
{
    Q_OBJECT
public:
    explicit PublishPage(QWidget *parent = nullptr);
    void clearFields();

signals:
    void goHomeSignal();

private:
    void onUploadImage();
    void onPublish();

    // UI 控件 (匹配你的高级版代码)
    class QPushButton *m_imageBtn; // 图片上传按钮
    class QLineEdit *m_nameEdit;
    class QLineEdit *m_condition;
    class QLineEdit *m_minPrice;   // 最低价
    class QLineEdit *m_maxPrice;   // 期望价
    class QTextEdit *m_descEdit;
    class QComboBox *m_categoryCombo;

    QString m_selectedPath; // 记录选中的图片路径
};

#endif // PUBLISHPAGE_H
