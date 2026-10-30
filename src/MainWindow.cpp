#include "MainWindow.h"
#include "LoginPage.h"
#include "RegisterPage.h"
#include "HomePage.h"
#include "PublishPage.h"
#include "ProductDetail.h"
#include "UserPage.h"
#include "AdminPage.h"
#include "DbManager.h"
#include "TradeCenter.h"
#include <QMessageBox>
#include "Toast.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QApplication>
#include <QMouseEvent>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QFont>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    // 1. 无边框 + 透明背景
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground);

    resize(1024, 768);
    setObjectName("AppRoot");

    initUi();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initUi()
{
    // 全局滚动条样式
    this->setStyleSheet(
        "QScrollBar:vertical {"
        "    border: none;"
        "    background: transparent;"
        "    width: 8px;"
        "    margin: 0px 0px 0px 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: rgba(200, 200, 200, 100);"
        "    min-height: 30px;"
        "    border-radius: 4px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background: rgba(150, 150, 150, 150);"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }"
        "QScrollBar:horizontal { height: 0px; }"
        );

    m_rootLayout = new QVBoxLayout(this);
    m_rootLayout->setContentsMargins(10, 10, 10, 10);
    m_rootLayout->setSpacing(0);

    m_mainFrame = new QFrame(this);
    m_mainFrame->setObjectName("MainFrame");
    m_mainFrame->setStyleSheet(
        "QFrame#MainFrame {"
        "    background-color: #FFFFFF;"
        "    border-radius: 12px;"
        "    border: 1px solid #E0E0E0;"
        "}"
        );

    m_shadow = new QGraphicsDropShadowEffect(this);
    m_shadow->setBlurRadius(20);
    m_shadow->setColor(QColor(0, 0, 0, 100));
    m_shadow->setOffset(0, 0);
    m_mainFrame->setGraphicsEffect(m_shadow);

    m_rootLayout->addWidget(m_mainFrame);

    QVBoxLayout *innerLayout = new QVBoxLayout(m_mainFrame);
    innerLayout->setContentsMargins(0, 0, 0, 0);
    innerLayout->setSpacing(0);

    // 绘制标题栏
    setupTitleBar(innerLayout);

    m_stack = new AnimatedStackedWidget(this);

    LoginPage *loginPage = new LoginPage(this);
    RegisterPage *registerPage = new RegisterPage(this);
    HomePage *homePage = new HomePage(this);
    PublishPage *publishPage = new PublishPage(this);
    ProductDetail *detailPage = new ProductDetail(this);
    UserPage *userPage = new UserPage(this);
    AdminPage *adminPage = new AdminPage(this);

    m_stack->addWidget(loginPage);
    m_stack->addWidget(registerPage);
    m_stack->addWidget(homePage);
    m_stack->addWidget(publishPage);
    m_stack->addWidget(detailPage);
    m_stack->addWidget(userPage);
    m_stack->addWidget(adminPage);

    innerLayout->addWidget(m_stack);

    // --- 信号连接 ---
    connect(loginPage, &LoginPage::loginSuccess, this, [=](int uid){
        Q_UNUSED(uid);
        homePage->refreshGoods();
        m_stack->slideInIdx(m_stack->indexOf(homePage));
    });
    connect(loginPage, &LoginPage::goRegisterSignal, this, [=](){
        m_stack->slideInIdx(m_stack->indexOf(registerPage));
    });
    connect(registerPage, &RegisterPage::goLoginSignal, this, [=](){
        m_stack->slideInIdx(m_stack->indexOf(loginPage));
    });
    connect(homePage, &HomePage::logoutSignal, this, [=](){
        DbManager::instance().loginAsGuest();
        m_stack->slideInIdx(m_stack->indexOf(loginPage));
    });
    connect(homePage, &HomePage::goPublishSignal, this, [=](){
        publishPage->clearFields();
        m_stack->slideInIdx(m_stack->indexOf(publishPage));
    });
    connect(publishPage, &PublishPage::goHomeSignal, this, [=](){
        homePage->refreshGoods();
        m_stack->slideInIdx(m_stack->indexOf(homePage));
    });
    connect(homePage, &HomePage::goUserCenterSignal, this, [=](){
        UserRole role = DbManager::instance().getCurrentUserRole();
        if (role == RoleAdmin) {
            adminPage->refreshData();
            m_stack->slideInIdx(m_stack->indexOf(adminPage));
        } else {
            userPage->refreshData();
            m_stack->slideInIdx(m_stack->indexOf(userPage));
        }
    });
    connect(userPage, &UserPage::goBackSignal, this, [=](){
        homePage->refreshGoods();
        m_stack->slideInIdx(m_stack->indexOf(homePage));
    });
    connect(adminPage, &AdminPage::goHomeSignal, this, [=](){
        homePage->refreshGoods();
        m_stack->slideInIdx(m_stack->indexOf(homePage));
    });
    connect(homePage, &HomePage::viewProductSignal, this, [=](QVariantMap data){
        m_lastPageIndex = 2;
        detailPage->setProductData(data);
        detailPage->setBackText("← 返回首页");
        m_stack->slideInIdx(m_stack->indexOf(detailPage));
    });
    connect(userPage, &UserPage::viewProductSignal, this, [=](QVariantMap data){
        m_lastPageIndex = 5;
        detailPage->setProductData(data);
        detailPage->setBackText("← 返回个人主页");
        m_stack->slideInIdx(m_stack->indexOf(detailPage));
    });
    connect(detailPage, &ProductDetail::goBackSignal, this, [=](){
        if (m_lastPageIndex == 5) userPage->refreshData();
        m_stack->setCurrentIndex(m_lastPageIndex);
    });
}

void MainWindow::setupTitleBar(QVBoxLayout *layout)
{
    QWidget *titleBar = new QWidget(this);
    titleBar->setFixedHeight(40);
    titleBar->setStyleSheet("background-color: #202020; border-top-left-radius: 12px; border-top-right-radius: 12px;");

    QHBoxLayout *hLayout = new QHBoxLayout(titleBar);
    hLayout->setContentsMargins(15, 0, 10, 0);
    hLayout->setSpacing(0);

    QLabel *titleLabel = new QLabel("中南民族大学二手交易平台", titleBar);
    titleLabel->setStyleSheet("font-weight: bold; color: #FFFFFF; font-size: 13px; background: transparent;");

    QString btnBase = "QPushButton { background: transparent; border: none; color: #FFFFFF; font-family: 'Marlett'; font-size: 14px; } QPushButton:hover { background-color: #444444; }";
    QString closeStyle = "QPushButton { background: transparent; border: none; color: #FFFFFF; font-family: 'Marlett'; font-size: 14px; } QPushButton:hover { background-color: #E81123; }";

    QPushButton *btnMin = new QPushButton("0", titleBar);
    QPushButton *btnMax = new QPushButton("1", titleBar);
    QPushButton *btnClose = new QPushButton("r", titleBar);

    btnMin->setFixedSize(46, 40);
    btnMax->setFixedSize(46, 40);
    btnClose->setFixedSize(46, 40);

    btnMin->setStyleSheet(btnBase);
    btnMax->setStyleSheet(btnBase);
    btnClose->setStyleSheet(closeStyle);

    hLayout->addWidget(titleLabel);
    auto tradeButton = new QPushButton("交易 / 信用 / 举报 / 报表", titleBar);
    tradeButton->setObjectName("tradeCenterButton");
    tradeButton->setStyleSheet("QPushButton {color:white;background:#333;padding:6px 16px;border:1px solid #666;border-radius:4px;}");
    hLayout->addWidget(tradeButton);
    connect(tradeButton, &QPushButton::clicked, this, [this](){
        if(!DbManager::instance().isActive()) {QMessageBox::information(this,"提示","请先登录正常账户");return;}
        TradeCenter dialog(this); dialog.exec();
        if(auto home=qobject_cast<HomePage*>(m_stack->widget(2))) home->refreshGoods();
        if(auto user=qobject_cast<UserPage*>(m_stack->widget(5))) user->refreshData();
    });
    hLayout->addStretch();
    hLayout->addWidget(btnMin);
    hLayout->addWidget(btnMax);
    hLayout->addWidget(btnClose);

    layout->addWidget(titleBar);

    connect(btnMin, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(btnMax, &QPushButton::clicked, this, [this](){
        if (this->isMaximized()) this->showNormal();
        else this->showMaximized();
    });
    connect(btnClose, &QPushButton::clicked, this, &QWidget::close);
}

// ==========================================
// 👇👇👇 拖拽窗口核心逻辑 👇👇👇
// ==========================================

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (this->isMaximized()) return; // 最大化时不处理

    // 判断鼠标按下的位置是否在自定义标题栏区域 (Y轴 0~50 像素)
    // 包含标题栏容器以及周围一点冗余
    if (event->button() == Qt::LeftButton && event->position().y() < 50) {
        // 检查是否点中了标题栏内的子控件（如按钮）
        // 如果点的是按钮，就不触发窗口移动
        if (childAt(event->position().toPoint()) != nullptr &&
            childAt(event->position().toPoint())->inherits("QPushButton")) {
            return;
        }

        m_isDragging = true;
        // 记录鼠标相对窗口左上角的偏移
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (this->isMaximized()) return;

    if (event->buttons() & Qt::LeftButton && m_isDragging) {
        // 计算新位置并移动
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_isDragging = false;
    QWidget::mouseReleaseEvent(event);
}
