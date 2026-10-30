#include "LoginPage.h"
#include "DbManager.h"
#include "Toast.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPainterPath>
#include <QSqlQuery>

LoginPage::LoginPage(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void LoginPage::setupUi()
{
    // 背景：左右分栏
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ===========================
    // 左侧：黑色品牌区 (固定宽度，防止切换时跳动)
    // ===========================
    QWidget *leftPanel = new QWidget(this);
    leftPanel->setStyleSheet("background-color: #1A1A1A;");
    leftPanel->setFixedWidth(500);

    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setAlignment(Qt::AlignCenter);
    leftLayout->setSpacing(30);

    QLabel *logoImg = new QLabel(leftPanel);
    logoImg->setFixedSize(300, 300);
    logoImg->setScaledContents(true);

    QPixmap pix(":/logo.png");
    if (!pix.isNull()) {
        logoImg->setPixmap(pix);
    } else {
        logoImg->setText("Logo");
        logoImg->setStyleSheet("color: white; font-weight: bold; background-color: #333; border-radius: 20px;");
        logoImg->setAlignment(Qt::AlignCenter);
    }

    QLabel *appName = new QLabel("SCUEC\nMarket", leftPanel);
    appName->setStyleSheet("color: #FFFFFF; font-size: 48px; font-weight: 900; font-family: 'Impact';");
    appName->setAlignment(Qt::AlignCenter);

    QLabel *slogan = new QLabel("中南民族大学校园二手交易平台", leftPanel);
    slogan->setStyleSheet("color: #AAAAAA; font-size: 16px; margin-top: 10px;");
    slogan->setAlignment(Qt::AlignCenter);

    leftLayout->addStretch();
    leftLayout->addWidget(logoImg, 0, Qt::AlignCenter);
    leftLayout->addWidget(appName, 0, Qt::AlignCenter);
    leftLayout->addWidget(slogan, 0, Qt::AlignCenter);
    leftLayout->addStretch();

    // ===========================
    // 右侧：白色登录区
    // ===========================
    QWidget *rightPanel = new QWidget(this);
    rightPanel->setStyleSheet("background-color: #FFFFFF;");

    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setAlignment(Qt::AlignCenter);

    QWidget *centerWidget = new QWidget(rightPanel);
    centerWidget->setFixedWidth(320);

    QVBoxLayout *formLayout = new QVBoxLayout(centerWidget);
    formLayout->setSpacing(20);
    formLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *welcome = new QLabel("欢迎回来", centerWidget);
    welcome->setStyleSheet("font-size: 28px; font-weight: bold; color: #333333; margin-bottom: 20px;");

    m_studentIdEdit = new QLineEdit(centerWidget);
    m_studentIdEdit->setPlaceholderText("请输入学号");
    m_studentIdEdit->setFixedHeight(45);
    m_studentIdEdit->setStyleSheet("QLineEdit { border: 1px solid #E0E0E0; border-radius: 4px; padding-left: 10px; font-size: 14px; } QLineEdit:focus { border: 1px solid #333; }");

    m_passEdit = new QLineEdit(centerWidget);
    m_passEdit->setPlaceholderText("请输入密码");
    m_passEdit->setEchoMode(QLineEdit::Password);
    m_passEdit->setFixedHeight(45);
    m_passEdit->setStyleSheet("QLineEdit { border: 1px solid #E0E0E0; border-radius: 4px; padding-left: 10px; font-size: 14px; } QLineEdit:focus { border: 1px solid #333; }");

    QPushButton *btnLogin = new QPushButton("立即登录", centerWidget);
    btnLogin->setFixedHeight(45);
    btnLogin->setCursor(Qt::PointingHandCursor);
    btnLogin->setStyleSheet("QPushButton { background-color: #1A1A1A; color: white; border-radius: 4px; font-size: 15px; font-weight: bold; } QPushButton:hover { background-color: #333333; }");

    QPushButton *btnRegister = new QPushButton("注册账号", centerWidget);
    btnRegister->setCursor(Qt::PointingHandCursor);
    btnRegister->setStyleSheet("QPushButton { background: transparent; color: #666; border: none; font-size: 14px; text-decoration: underline; } QPushButton:hover { color: #409EFF; }");

    m_msgLabel = new QLabel("", centerWidget);
    m_msgLabel->setStyleSheet("color: #FF4D4F; font-size: 12px;");
    m_msgLabel->setAlignment(Qt::AlignCenter);

    QPushButton *btnGuest = new QPushButton("随便看看 >", centerWidget);
    btnGuest->setCursor(Qt::PointingHandCursor);
    btnGuest->setStyleSheet("QPushButton { background: transparent; color: #999; border: none; font-size: 14px; margin-top: 10px; } QPushButton:hover { color: #666; }");

    formLayout->addWidget(welcome);
    formLayout->addWidget(m_studentIdEdit);
    formLayout->addWidget(m_passEdit);
    formLayout->addWidget(btnLogin);
    formLayout->addWidget(btnRegister, 0, Qt::AlignCenter);
    formLayout->addWidget(m_msgLabel, 0, Qt::AlignCenter);
    formLayout->addWidget(btnGuest, 0, Qt::AlignCenter);

    rightLayout->addWidget(centerWidget);

    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(rightPanel);

    connect(btnLogin, &QPushButton::clicked, this, &LoginPage::onLogin);
    connect(btnRegister, &QPushButton::clicked, this, [=](){ emit goRegisterSignal(); });
    connect(btnGuest, &QPushButton::clicked, this, [=](){ DbManager::instance().loginAsGuest(); emit loginSuccess(-1); });
}

void LoginPage::onLogin()
{
    QString studentId = m_studentIdEdit->text().trimmed();
    QString pass = m_passEdit->text();

    m_msgLabel->setText("");

    if (studentId.isEmpty() || pass.isEmpty()) {
        m_msgLabel->setText("学号或密码不能为空");
        return;
    }

    if (!DbManager::instance().isStudentIdExists(studentId)) {
        Toast::show(this->window(), "学号或密码错误");
        m_msgLabel->setText("学号或密码错误");
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT id, status FROM User WHERE student_id = ?");
    query.addBindValue(studentId);
    if (query.exec() && query.next()) {
        int statusInt = query.value(1).toInt();
        if (statusInt == StatusBanned) {
            Toast::show(this->window(), "账户已被封禁");
            m_msgLabel->setText("账户已被封禁，如有疑问请联系管理员");
            return;
        }
    }

    if (DbManager::instance().loginUser(studentId, pass)) {
        int uid = DbManager::instance().getCurrentUserId();
        Toast::show(this->window(), "欢迎回来");
        emit loginSuccess(uid);
    } else {
        Toast::show(this->window(), "学号或密码错误");
        m_msgLabel->setText("学号或密码错误");
    }
}
