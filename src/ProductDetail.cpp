#include "ProductDetail.h"
#include "DbManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>
#include <QClipboard>
#include <QGuiApplication>
#include <QLineEdit>
#include <QInputDialog>
#include <QDir>
#include <QCoreApplication>
#include "Toast.h"

ProductDetail::ProductDetail(QWidget *parent) : QWidget(parent)
{
    initUi();
}

void ProductDetail::initUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 1. 顶部导航栏
    QWidget *navBar = new QWidget(this);
    navBar->setFixedHeight(60);
    navBar->setStyleSheet("background-color: white; border-bottom: 1px solid #F0F0F0;");
    QHBoxLayout *navLayout = new QHBoxLayout(navBar);
    navLayout->setContentsMargins(20, 0, 20, 0);

    m_btnBack = new QPushButton("← 返回", navBar);
    m_btnBack->setCursor(Qt::PointingHandCursor);
    m_btnBack->setStyleSheet("QPushButton { border: none; font-size: 16px; color: #666; font-weight: bold; background: transparent; } QPushButton:hover { color: #333; }");

    m_btnAdminDelete = new QPushButton("🗑️ 违规下架", navBar);
    m_btnAdminDelete->setVisible(false);
    m_btnAdminDelete->setCursor(Qt::PointingHandCursor);
    m_btnAdminDelete->setMinimumWidth(100); // 👈 确保管理员按钮也足够宽
    m_btnAdminDelete->setStyleSheet("QPushButton { background-color: #FFF0F0; color: #FF4D4F; border: 1px solid #FFCCC7; border-radius: 4px; padding: 5px 10px; font-weight: bold; }");

    navLayout->addWidget(m_btnBack);
    navLayout->addStretch();
    navLayout->addWidget(m_btnAdminDelete);
    auto buy=new QPushButton("发起购买请求");buy->setObjectName("buyButton");
    auto reportGoods=new QPushButton("举报商品");auto reportUser=new QPushButton("举报卖家");
    for(auto button:{buy,reportGoods,reportUser}) navLayout->addWidget(button);
    connect(buy,&QPushButton::clicked,this,[this](){
        auto &db=DbManager::instance();
        if(db.requestPurchase(m_currentGoodsId)) QMessageBox::information(this,"请求已发送","等待卖家在交易中心确认；确认后才生成订单");
        else QMessageBox::warning(this,"请求失败",db.lastError());
    });
    auto report=[this](const QString &type,int id){
        bool ok;QString reason=QInputDialog::getMultiLineText(this,"举报","请填写具体理由（最多500字）","",&ok);if(!ok)return;
        auto &db=DbManager::instance();if(db.submitReport(type,id,reason)) QMessageBox::information(this,"已提交","可在交易中心查看处理结果");
        else QMessageBox::warning(this,"举报失败",db.lastError());
    };
    connect(reportGoods,&QPushButton::clicked,this,[this,report](){report("goods",m_currentGoodsId);});
    connect(reportUser,&QPushButton::clicked,this,[this,report](){report("user",m_sellerId);});
    mainLayout->addWidget(navBar);

    // 2. 滚动区
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: #F7F8FA; }");

    QWidget *contentWidget = new QWidget();
    contentWidget->setStyleSheet("background-color: #F7F8FA;");
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(40, 20, 40, 40);
    contentLayout->setSpacing(25);

    // === A. 商品主信息卡片 ===
    QWidget *mainCard = new QWidget();
    mainCard->setStyleSheet("background-color: white; border-radius: 16px; border: none;");

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(mainCard);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 15));
    shadow->setOffset(0, 4);
    mainCard->setGraphicsEffect(shadow);

    QVBoxLayout *cardLayout = new QVBoxLayout(mainCard);
    cardLayout->setContentsMargins(35, 35, 35, 35);
    cardLayout->setSpacing(20);

    m_nameLabel = new QLabel(mainCard);
    m_nameLabel->setTextFormat(Qt::PlainText);
    m_nameLabel->setStyleSheet("font-size: 28px; font-weight: 900; color: #1A1A1A; border: none;");

    m_priceLabel = new QLabel(mainCard);
    m_priceLabel->setStyleSheet("font-size: 32px; font-weight: 900; color: #1A1A1A; border: none;");

    // 卖家与面交信息栏
    QWidget *infoBar = new QWidget();
    infoBar->setStyleSheet("background: #F9F9F9; border-radius: 12px; border: none;");
    QGridLayout *infoGrid = new QGridLayout(infoBar);
    infoGrid->setContentsMargins(25, 25, 25, 25);
    infoGrid->setVerticalSpacing(15);
    infoGrid->setHorizontalSpacing(15);

    QLabel *l1 = new QLabel("发布人:"); l1->setStyleSheet("color: #999; border: none; font-size: 14px;");
    m_publisherVal = new QLabel("---"); m_publisherVal->setStyleSheet("font-weight: bold; color: #333; border: none; font-size: 15px;");

    QLabel *l2 = new QLabel("联系方式:"); l2->setStyleSheet("color: #999; border: none; font-size: 14px;");
    m_contactVal = new QLabel("---"); m_contactVal->setStyleSheet("font-weight: bold; color: #409EFF; border: none; font-size: 15px;");

    // 👈 核心修复：“一键复制”按钮设置
    m_btnCopy = new QPushButton("一键复制");
    m_btnCopy->setCursor(Qt::PointingHandCursor);
    m_btnCopy->setMinimumWidth(85); // 👈 增加最小宽度，确保文字不挤压
    m_btnCopy->setFixedHeight(28);   // 稍微增高
    m_btnCopy->setStyleSheet(
        "QPushButton {"
        "    background: #409EFF; color: white; border-radius: 4px; "
        "    font-size: 12px; font-weight: bold; border: none; padding: 0 10px;"
        "}"
        "QPushButton:hover { background: #3388DD; }"
        );

    QLabel *l3 = new QLabel("面交建议:"); l3->setStyleSheet("color: #999; border: none; font-size: 14px;");
    m_locationVal = new QLabel("---"); m_locationVal->setStyleSheet("font-weight: bold; color: #333; border: none; font-size: 15px;");

    infoGrid->addWidget(l1, 0, 0);
    infoGrid->addWidget(m_publisherVal, 0, 1);
    infoGrid->addWidget(l2, 1, 0);

    QHBoxLayout *contactLay = new QHBoxLayout();
    contactLay->addWidget(m_contactVal);
    contactLay->addSpacing(15); // 👈 增加间距
    contactLay->addWidget(m_btnCopy);
    contactLay->addStretch();   // 👈 将按钮推向左侧靠拢文字
    infoGrid->addLayout(contactLay, 1, 1);

    infoGrid->addWidget(l3, 2, 0);
    infoGrid->addWidget(m_locationVal, 2, 1);

    m_imgLabel = new QLabel();
    m_imgLabel->setFixedSize(500, 380);
    m_imgLabel->setStyleSheet("background: #FAFAFA; border-radius: 12px; border: none;");
    m_imgLabel->setAlignment(Qt::AlignCenter);

    m_descLabel = new QLabel();
    m_descLabel->setTextFormat(Qt::PlainText);
    m_descLabel->setWordWrap(true);
    m_descLabel->setStyleSheet("font-size: 16px; color: #555; line-height: 160%; border: none;");

    cardLayout->addWidget(m_nameLabel);
    cardLayout->addWidget(m_priceLabel);
    cardLayout->addWidget(infoBar);
    cardLayout->addWidget(m_imgLabel, 0, Qt::AlignCenter);
    QLabel *detailTitle = new QLabel("<b>宝贝详情</b>"); detailTitle->setStyleSheet("font-size: 18px; margin-top: 10px; border: none;");
    cardLayout->addWidget(detailTitle);
    cardLayout->addWidget(m_descLabel);

    contentLayout->addWidget(mainCard);

    // === B. 留言板卡片 ===
    QWidget *msgCard = new QWidget();
    msgCard->setStyleSheet("background-color: white; border-radius: 16px; border: none;");

    QGraphicsDropShadowEffect *msgShadow = new QGraphicsDropShadowEffect(msgCard);
    msgShadow->setBlurRadius(20);
    msgShadow->setColor(QColor(0, 0, 0, 15));
    msgShadow->setOffset(0, 4);
    msgCard->setGraphicsEffect(msgShadow);

    QVBoxLayout *msgLayout = new QVBoxLayout(msgCard);
    msgLayout->setContentsMargins(35, 30, 35, 35);
    msgLayout->setSpacing(15);

    QLabel *msgTitle = new QLabel("<b>留言互动</b>"); msgTitle->setStyleSheet("font-size: 18px; border: none;");
    msgLayout->addWidget(msgTitle);

    m_msgDisplay = new QVBoxLayout();
    msgLayout->addLayout(m_msgDisplay);

    QHBoxLayout *inputLay = new QHBoxLayout();
    m_msgInput = new QLineEdit();
    m_msgInput->setPlaceholderText("问问卖家更多细节...");
    m_msgInput->setFixedHeight(45);
    m_msgInput->setStyleSheet("QLineEdit { background: #F7F8FA; border: none; border-radius: 8px; padding-left: 15px; font-size: 14px; } QLineEdit:focus { background: white; border: 1px solid #409EFF; }");

    QPushButton *btnSend = new QPushButton("发送留言");
    btnSend->setFixedSize(100, 45); // 👈 统一高度，加大宽度
    btnSend->setCursor(Qt::PointingHandCursor);
    btnSend->setStyleSheet("QPushButton { background: #1A1A1A; color: white; border-radius: 8px; font-weight: bold; border: none; } QPushButton:hover { background: #333; }");

    inputLay->addWidget(m_msgInput);
    inputLay->addSpacing(10);
    inputLay->addWidget(btnSend);
    msgLayout->addLayout(inputLay);

    contentLayout->addWidget(msgCard);

    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea);

    // --- 信号连接 ---
    connect(m_btnCopy, &QPushButton::clicked, this, [=](){
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(m_contactVal->text());
        Toast::show(this->window(), "📋 联系方式已复制");
    });

    connect(btnSend, &QPushButton::clicked, this, &ProductDetail::onSendMsg);
    connect(m_btnBack, &QPushButton::clicked, this, &ProductDetail::goBackSignal);

    connect(m_btnAdminDelete, &QPushButton::clicked, this, [=](){
        if(QMessageBox::Yes == QMessageBox::question(this, "警告", "确认下架商品并取消未完成订单？商品和历史交易记录将保留。")) {
            if(DbManager::instance().removeGoods(m_currentGoodsId)) emit goBackSignal();
        }
    });
}

void ProductDetail::setProductData(const QVariantMap &data)
{
    m_currentGoodsId = data["id"].toInt();
    m_sellerId=data["publisherId"].toInt();
    m_nameLabel->setText(data["name"].toString());
    m_priceLabel->setText(data["price"].toString());
    m_descLabel->setText("成色："+data["condition_desc"].toString()+"\n\n"+data["description"].toString());
    m_publisherVal->setText(data["publisher"].toString()+QString("（信用分 %1）").arg(DbManager::instance().credit(m_sellerId),0,'f',2));

    QString loc = data["location"].toString();
    m_locationVal->setText(loc.isEmpty() ? "卖家未提供" : loc);

    UserRole role = DbManager::instance().getCurrentUserRole();
    if (role == RoleGuest) {
        m_contactVal->setText("🔒 登录可见");
        m_btnCopy->setVisible(false);
    } else {
        m_contactVal->setText(data["contact"].toString());
        m_btnCopy->setVisible(true);
    }

    m_btnAdminDelete->setVisible(role == RoleAdmin);

    QString image=data["imagePath"].toString();
    QPixmap pix(image.startsWith(":")||QDir::isAbsolutePath(image)?image:QCoreApplication::applicationDirPath()+"/"+image);
    if (!pix.isNull()) {
        m_imgLabel->setPixmap(pix.scaled(500, 380, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        m_imgLabel->setText("暂无图片");
    }

    refreshMessages();
}

void ProductDetail::refreshMessages()
{
    QLayoutItem *child;
    while ((child = m_msgDisplay->takeAt(0)) != nullptr) {
        if(child->widget()) delete child->widget();
        delete child;
    }

    auto msgs = DbManager::instance().getMessagesForGoods(m_currentGoodsId);
    if(msgs.isEmpty()) {
        QLabel *empty = new QLabel("暂无留言，快来抢沙发吧~");
        empty->setStyleSheet("color: #CCC; font-style: italic; border: none; background: transparent;");
        m_msgDisplay->addWidget(empty);
    } else {
        for (const auto &m : msgs) {
            QLabel *label = new QLabel(QString("<font color='#409EFF'><b>%1</b></font>: %2 <font color='#BBB' size='2'>[%3]</font>")
                                           .arg(m["username"].toString().toHtmlEscaped(), m["content"].toString().toHtmlEscaped(), m["time"].toString()));
            label->setWordWrap(true);
            label->setStyleSheet("padding: 10px; border-bottom: 1px solid #F5F5F5; background: transparent;");
            m_msgDisplay->addWidget(label);
        }
    }
}

void ProductDetail::onSendMsg()
{
    if (DbManager::instance().getCurrentUserRole() == RoleGuest) {
        QMessageBox::warning(this, "提示", "请先登录后再留言");
        return;
    }
    QString content = m_msgInput->text().trimmed();
    if (content.isEmpty()) return;

    if (DbManager::instance().addMessage(m_currentGoodsId, DbManager::instance().getCurrentUserId(), content)) {
        m_msgInput->clear();
        Toast::show(this->window(), "💬 留言已发送");
        refreshMessages();
    }
}

void ProductDetail::setBackText(const QString &text)
{
    if (m_btnBack) m_btnBack->setText(text);
}
