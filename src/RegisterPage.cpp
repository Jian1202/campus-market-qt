#include "RegisterPage.h"
#include "DbManager.h"
#include "Toast.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QRegularExpression>

RegisterPage::RegisterPage(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void RegisterPage::setupUi()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ===========================
    // 左侧：黑色品牌区
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
    // 右侧：白色注册区
    // ===========================
    QWidget *rightPanel = new QWidget(this);
    rightPanel->setStyleSheet("background-color: #FFFFFF;");

    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setAlignment(Qt::AlignCenter);

    QWidget *centerWidget = new QWidget(rightPanel);
    centerWidget->setFixedWidth(320);

    QVBoxLayout *formLayout = new QVBoxLayout(centerWidget);
    formLayout->setSpacing(15);
    formLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *title = new QLabel("创建账号", centerWidget);
    title->setStyleSheet("font-size: 28px; font-weight: bold; color: #333333; margin-bottom: 10px;");

    // 学号输入框
    m_studentIdEdit = new QLineEdit(centerWidget);
    m_studentIdEdit->setPlaceholderText("请输入学号 (如: 202421091200)");
    m_studentIdEdit->setFixedHeight(45);
    m_studentIdEdit->setStyleSheet("QLineEdit { border: 1px solid #E0E0E0; border-radius: 4px; padding-left: 10px; font-size: 14px; } QLineEdit:focus { border: 1px solid #333; }");

    // 姓名输入框
    m_nameEdit = new QLineEdit(centerWidget);
    m_nameEdit->setPlaceholderText("请输入姓名");
    m_nameEdit->setFixedHeight(45);
    m_nameEdit->setStyleSheet("QLineEdit { border: 1px solid #E0E0E0; border-radius: 4px; padding-left: 10px; font-size: 14px; } QLineEdit:focus { border: 1px solid #333; }");

    // 密码输入框
    m_passEdit = new QLineEdit(centerWidget);
    m_passEdit->setPlaceholderText("设置密码 (至少6位)");
    m_passEdit->setEchoMode(QLineEdit::Password);
    m_passEdit->setFixedHeight(45);
    m_passEdit->setStyleSheet("QLineEdit { border: 1px solid #E0E0E0; border-radius: 4px; padding-left: 10px; font-size: 14px; } QLineEdit:focus { border: 1px solid #333; }");

    // 确认密码输入框
    m_passConfirmEdit = new QLineEdit(centerWidget);
    m_passConfirmEdit->setPlaceholderText("确认密码");
    m_passConfirmEdit->setEchoMode(QLineEdit::Password);
    m_passConfirmEdit->setFixedHeight(45);
    m_passConfirmEdit->setStyleSheet("QLineEdit { border: 1px solid #E0E0E0; border-radius: 4px; padding-left: 10px; font-size: 14px; } QLineEdit:focus { border: 1px solid #333; }");

    // 联系方式输入框
    m_contactEdit = new QLineEdit(centerWidget);
    m_contactEdit->setPlaceholderText("联系方式 (手机/微信/QQ)");
    m_contactEdit->setFixedHeight(45);
    m_contactEdit->setStyleSheet("QLineEdit { border: 1px solid #E0E0E0; border-radius: 4px; padding-left: 10px; font-size: 14px; } QLineEdit:focus { border: 1px solid #333; }");

    // 错误提示标签
    m_msgLabel = new QLabel("", centerWidget);
    m_msgLabel->setStyleSheet("color: #FF4D4F; font-size: 12px;");
    m_msgLabel->setAlignment(Qt::AlignCenter);

    // 注册按钮
    QPushButton *btnRegister = new QPushButton("立即注册", centerWidget);
    btnRegister->setFixedHeight(45);
    btnRegister->setCursor(Qt::PointingHandCursor);
    btnRegister->setStyleSheet("QPushButton { background-color: #1A1A1A; color: white; border-radius: 4px; font-size: 15px; font-weight: bold; } QPushButton:hover { background-color: #333333; }");

    // 返回登录按钮
    QPushButton *btnBack = new QPushButton("已有账号？返回登录", centerWidget);
    btnBack->setCursor(Qt::PointingHandCursor);
    btnBack->setStyleSheet(
        "QPushButton {"
        "    background: transparent;"
        "    color: #666;"
        "    border: none;"
        "    font-size: 14px;"
        "    text-decoration: underline;"
        "}"
        "QPushButton:hover { color: #409EFF; }"
        );

    formLayout->addWidget(title);
    formLayout->addWidget(m_studentIdEdit);
    formLayout->addWidget(m_nameEdit);
    formLayout->addWidget(m_passEdit);
    formLayout->addWidget(m_passConfirmEdit);
    formLayout->addWidget(m_contactEdit);
    formLayout->addWidget(m_msgLabel);
    formLayout->addWidget(btnRegister);
    formLayout->addWidget(btnBack, 0, Qt::AlignCenter);

    rightLayout->addWidget(centerWidget);

    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(rightPanel);

    // === 信号连接 ===
    connect(btnRegister, &QPushButton::clicked, this, &RegisterPage::onRegister);

    connect(btnBack, &QPushButton::clicked, this, [=](){
        emit goLoginSignal();
    });
}

void RegisterPage::onRegister()
{
    QString studentId = m_studentIdEdit->text().trimmed();
    QString name = m_nameEdit->text().trimmed();
    QString pass = m_passEdit->text();
    QString passConfirm = m_passConfirmEdit->text();
    QString contact = m_contactEdit->text().trimmed();

    // 清空之前的错误提示
    m_msgLabel->setText("");

    // 验证学号
    if (studentId.isEmpty()) {
        m_msgLabel->setText("❌ 学号不能为空");
        return;
    }

    // 学号格式验证（8位数字）
    QRegularExpression studentIdRegex("^\\d{12}$");
    if (!studentIdRegex.match(studentId).hasMatch()) {
        m_msgLabel->setText("❌ 学号必须为12位数字");
        return;
    }

    // 验证学号唯一性
    if (DbManager::instance().isStudentIdExists(studentId)) {
        m_msgLabel->setText("❌ 该学号已被注册");
        return;
    }

    // 验证姓名
    if (name.isEmpty()) {
        m_msgLabel->setText("❌ 姓名不能为空");
        return;
    }

    if (name.length() > 20) {
        m_msgLabel->setText("❌ 姓名过长（最多20个字符）");
        return;
    }

    // 验证密码
    if (pass.isEmpty()) {
        m_msgLabel->setText("❌ 密码不能为空");
        return;
    }

    if (pass.length() < 6) {
        m_msgLabel->setText("❌ 密码至少6位");
        return;
    }

    // 验证二次密码
    if (pass != passConfirm) {
        m_msgLabel->setText("❌ 两次输入的密码不一致");
        return;
    }

    // 验证联系方式
    if (contact.isEmpty()) {
        m_msgLabel->setText("❌ 联系方式不能为空");
        return;
    }

    // 执行注册
    if (DbManager::instance().registerUser(studentId, name, pass, contact)) {
        Toast::show(this->window(), "🎉 注册成功，请登录");
        emit goLoginSignal();
    } else {
        Toast::show(this->window(), "❌ 注册失败，请重试");
        m_msgLabel->setText("❌ 注册失败");
    }
}
