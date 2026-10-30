#ifndef REGISTERPAGE_H
#define REGISTERPAGE_H

#include <QWidget>
#include <QLineEdit>
#include <QLabel>

class RegisterPage : public QWidget
{
    Q_OBJECT
public:
    explicit RegisterPage(QWidget *parent = nullptr);

signals:
    void goLoginSignal(); // 返回登录页信号

private slots:
    void onRegister();    // 注册槽函数

private:
    void setupUi();       // UI 初始化函数

    // UI 控件成员变量
    QLineEdit *m_studentIdEdit;    // 学号输入框
    QLineEdit *m_nameEdit;         // 姓名输入框
    QLineEdit *m_passEdit;         // 密码输入框
    QLineEdit *m_passConfirmEdit;  // 确认密码输入框
    QLineEdit *m_contactEdit;      // 联系方式输入框
    QLabel *m_msgLabel;            // 错误提示标签
};

#endif // REGISTERPAGE_H
