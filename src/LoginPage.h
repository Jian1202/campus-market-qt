#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QWidget>
#include <QLineEdit>
#include <QLabel> // 👈 补上这个

class LoginPage : public QWidget
{
    Q_OBJECT
public:
    explicit LoginPage(QWidget *parent = nullptr);

signals:
    void loginSuccess(int uid);
    void goRegisterSignal();

private slots:
    void onLogin();

private:
    void setupUi();

    QLineEdit *m_studentIdEdit;  // 学号输入框
    QLineEdit *m_passEdit;
    QLabel *m_msgLabel; // 👈 补上这个成员变量
};

#endif // LOGINPAGE_H
