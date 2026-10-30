#include "HomePage.h"
#include "DbManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QPixmap>
#include <QDir>
#include <QCoreApplication>
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>
#include <QScrollArea>
#include <QResizeEvent>

HomePage::HomePage(QWidget *parent) : QWidget(parent)
{
    this->setAttribute(Qt::WA_StyledBackground);
    this->setStyleSheet("HomePage { background-color: #FFFFFF; }");
    setupUi();
}

void HomePage::refreshGoods(const QString &keyword, const QString &category)
{
    m_currentKeyword = keyword;
    m_currentCategory = category;
    updateTitleBar();
    m_currentGoodsData = DbManager::instance().getAllGoods(keyword, category);
    updateGridLayout();
}

void HomePage::updateGridLayout()
{
    // 清空现有布局
    QLayoutItem *child;
    while ((child = m_gridLayout->takeAt(0)) != 0) {
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }

    if (m_currentGoodsData.isEmpty()) {
        QLabel *empty = new QLabel("暂无相关商品", m_scrollContent);
        empty->setStyleSheet("color: #999; font-size: 18px; border: none; background: transparent;");
        m_gridLayout->addWidget(empty, 0, 0, Qt::AlignCenter);
        return;
    }

    int cardWidth = 260;
    int spacing = 25;
    int leftRightMargin = 40; // 设定的左右边距

    // 计算当前宽度下，减去固定边距后能塞下多少列
    int availableWidth = this->width() - (leftRightMargin * 2);
    int colCount = qMax(1, (availableWidth + spacing) / (cardWidth + spacing));

    // 3. 调整布局对齐：确保整体居中，这样左右空隙就对称了
    m_gridLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    m_gridLayout->setContentsMargins(leftRightMargin, 20, leftRightMargin, 40);
    m_gridLayout->setSpacing(spacing);

    int row = 0; int col = 0;
    for (const auto &data : m_currentGoodsData) {
        QFrame *card = new QFrame(m_scrollContent);
        card->setFixedSize(260, 330);
        // 👇 核心修正：显式确保卡片背景纯白，且无边框
        card->setStyleSheet("QFrame { background-color: #FFFFFF; border-radius: 12px; border: none; }");

        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
        shadow->setBlurRadius(20);
        shadow->setColor(QColor(0, 0, 0, 20));
        shadow->setOffset(0, 4);
        card->setGraphicsEffect(shadow);

        QVBoxLayout *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(0, 0, 0, 0);
        cardLayout->setSpacing(0);

        // 图片区
        QLabel *imgLabel = new QLabel(card);
        imgLabel->setFixedHeight(200);
        // 图片区保持淡灰底色，但去掉可能的边框
        imgLabel->setStyleSheet("border-top-left-radius: 12px; border-top-right-radius: 12px; background-color: #FAFAFA; border: none;");
        imgLabel->setAlignment(Qt::AlignCenter);

        if (!data["imagePath"].toString().isEmpty()) {
            QString image=data["imagePath"].toString();
            QPixmap pix(image.startsWith(":")||QDir::isAbsolutePath(image)?image:QCoreApplication::applicationDirPath()+"/"+image);
            imgLabel->setPixmap(pix.scaled(260, 200, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        } else {
            imgLabel->setText("无图");
            imgLabel->setStyleSheet("color: #CCC; font-size: 14px; background-color: #FAFAFA; border-top-left-radius: 12px; border-top-right-radius: 12px; border: none;");
        }

        // 文字区
        QWidget *infoArea = new QWidget(card);
        // 👇 核心修正：强制文字容器透明，且绝对无边框
        infoArea->setStyleSheet("background: transparent; border: none;");

        QVBoxLayout *infoLayout = new QVBoxLayout(infoArea);
        infoLayout->setContentsMargins(15, 12, 15, 15);
        infoLayout->setSpacing(6);

        QLabel *price = new QLabel(data["price"].toString(), infoArea);
        price->setStyleSheet("color: #1A1A1A; font-size: 18px; font-weight: 900; background: transparent; border: none;");

        QLabel *name = new QLabel(data["name"].toString(), infoArea);
        name->setStyleSheet("color: #333; font-size: 14px; font-weight: bold; background: transparent; border: none;");

        QHBoxLayout *tagLayout = new QHBoxLayout();
        tagLayout->setSpacing(5);
        QLabel *catTag = new QLabel(data["category"].toString(), infoArea);
        catTag->setStyleSheet("color: #999; font-size: 10px; background: #F0F0F0; padding: 2px 6px; border-radius: 4px; border: none;");
        tagLayout->addWidget(catTag);
        tagLayout->addStretch();

        QLabel *user = new QLabel("卖家: " + data["publisher"].toString(), infoArea);
        user->setStyleSheet("color: #999; font-size: 12px; background: transparent; border: none;");

        infoLayout->addWidget(price);
        infoLayout->addWidget(name);
        infoLayout->addLayout(tagLayout);
        infoLayout->addWidget(user);
        infoLayout->addStretch();

        cardLayout->addWidget(imgLabel);
        cardLayout->addWidget(infoArea);

        // 点击遮罩
        QPushButton *maskBtn = new QPushButton(card);
        maskBtn->setGeometry(0, 0, 260, 330);
        maskBtn->setStyleSheet("background: transparent; border: none;");
        maskBtn->setCursor(Qt::PointingHandCursor);
        connect(maskBtn, &QPushButton::clicked, this, [=](){ emit viewProductSignal(data); });

        m_gridLayout->addWidget(card, row, col);

        col++;
        if (col >= colCount) { col = 0; row++; }
    }
}

void HomePage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateGridLayout();
}

void HomePage::updateTitleBar() {
    UserRole role = DbManager::instance().getCurrentUserRole();
    QString exitStyleNormal = "QPushButton { background-color: transparent; color: #999; border: none; font-size: 14px; } QPushButton:hover { color: #FF4D4F; }";
    QString exitStyleLogin  = "QPushButton { background-color: #409EFF; color: white; border-radius: 20px; font-weight: bold; font-size: 14px; border: none; } QPushButton:hover { background-color: #3388DD; }";

    if (role == RoleGuest) {
        m_logoText->setText("中南民族大学二手交易平台 (游客)");
        m_logoText->setStyleSheet("font-size: 20px; font-weight: 900; color: #666; font-family: 'Microsoft YaHei'; border: none;");
        m_btnPublish->setVisible(false);
        m_btnUser->setVisible(false);
        m_btnAdmin->setVisible(false);
        m_btnExit->setText("登录 / 注册");
        m_btnExit->setFixedSize(130, 40);
        m_btnExit->setStyleSheet(exitStyleLogin);
    } else if (role == RoleAdmin) {
        m_logoText->setText("SCUEC 二手交易平台 (管理员模式)");
        m_logoText->setStyleSheet("font-size: 20px; font-weight: 900; color: #FF4D4F; font-family: 'Microsoft YaHei'; border: none;");
        m_btnPublish->setVisible(false);
        m_btnUser->setVisible(false);
        m_btnAdmin->setVisible(true);
        m_btnExit->setText("退出");
        m_btnExit->setFixedSize(60, 40);
        m_btnExit->setStyleSheet(exitStyleNormal);
    } else {
        m_logoText->setText("中南民族大学二手交易平台");
        m_logoText->setStyleSheet("font-size: 20px; font-weight: 900; color: #409EFF; font-family: 'Microsoft YaHei'; border: none;");
        m_btnPublish->setVisible(true);
        m_btnUser->setVisible(true);
        m_btnAdmin->setVisible(false);
        m_btnExit->setText("退出");
        m_btnExit->setFixedSize(60, 40);
        m_btnExit->setStyleSheet(exitStyleNormal);
    }
}

void HomePage::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QWidget *navBar = new QWidget(this);
    navBar->setFixedHeight(70);
    navBar->setStyleSheet("background-color: #FFFFFF; border-bottom: 1px solid #F0F0F0;");

    QHBoxLayout *navLayout = new QHBoxLayout(navBar);
    navLayout->setContentsMargins(30, 0, 30, 0);
    navLayout->setSpacing(15);

    QLabel *logoImg = new QLabel(navBar);
    logoImg->setFixedSize(40, 40);
    logoImg->setScaledContents(true);
    QPixmap pix(":/logo.png");
    if (!pix.isNull()) logoImg->setPixmap(pix);

    m_logoText = new QLabel("中南民族大学二手交易平台", navBar);
    m_logoText->setStyleSheet("font-size: 20px; font-weight: 900; color: #409EFF; border: none;");

    m_searchEdit = new QLineEdit(navBar);
    m_searchEdit->setPlaceholderText("🔍 搜索想要的好物...");
    m_searchEdit->setFixedWidth(320);
    m_searchEdit->setFixedHeight(40);
    m_searchEdit->setStyleSheet("QLineEdit { border: none; border-radius: 20px; padding-left: 15px; background: #F7F8FA; color: #333; } QLineEdit:focus { background: #FFF; border: 1px solid #1A1A1A; }");
    connect(m_searchEdit, &QLineEdit::returnPressed, this, [=](){ refreshGoods(m_searchEdit->text(), m_currentCategory); });

    m_btnPublish = new QPushButton("发布商品", navBar);
    m_btnPublish->setFixedSize(110, 40);
    m_btnPublish->setStyleSheet("QPushButton { background-color: #1A1A1A; color: #FFF; border-radius: 20px; font-weight: bold; border: none; } QPushButton:hover { background-color: #333; }");

    m_btnUser = new QPushButton("个人中心", navBar);
    m_btnUser->setFixedSize(110, 40);
    m_btnUser->setStyleSheet("QPushButton { background-color: transparent; color: #666; border: 1px solid #E0E0E0; border-radius: 20px; } QPushButton:hover { background-color: #F5F5F5; }");

    m_btnAdmin = new QPushButton("进入后台管理", navBar);
    m_btnAdmin->setFixedSize(140, 40);
    m_btnAdmin->setStyleSheet("QPushButton { background-color: #FF4D4F; color: #FFF; border-radius: 20px; font-weight: bold; border: none; } QPushButton:hover { background-color: #D9363E; }");

    m_btnExit = new QPushButton("退出", navBar);
    m_btnExit->setFixedSize(60, 40);

    navLayout->addWidget(logoImg);
    navLayout->addSpacing(8);
    navLayout->addWidget(m_logoText);
    navLayout->addSpacing(20);
    navLayout->addWidget(m_searchEdit);
    navLayout->addStretch();
    navLayout->addWidget(m_btnPublish);
    navLayout->addWidget(m_btnUser);
    navLayout->addWidget(m_btnAdmin);
    navLayout->addWidget(m_btnExit);

    mainLayout->addWidget(navBar);

    // 分类标签栏
    QWidget *categoryBar = new QWidget(this);
    categoryBar->setFixedHeight(50);
    categoryBar->setStyleSheet("background-color: #FFFFFF; border-bottom: 1px solid #F0F0F0;");
    QHBoxLayout *catLayout = new QHBoxLayout(categoryBar);
    catLayout->setContentsMargins(30, 0, 30, 0);
    catLayout->setSpacing(10);

    QStringList categories = {"全部", "数码产品", "学习用品", "生活百货", "代步工具", "运动健身", "其他"};
    for (const QString &cat : categories) {
        QPushButton *btn = new QPushButton(cat, categoryBar);
        btn->setCheckable(true);
        btn->setAutoExclusive(true);
        if (cat == "全部") btn->setChecked(true);

        btn->setStyleSheet(
            "QPushButton { border: none; padding: 5px 15px; border-radius: 15px; color: #666; font-weight: bold; }"
            "QPushButton:hover { background-color: #F5F5F5; }"
            "QPushButton:checked { background-color: #1A1A1A; color: #FFFFFF; }"
        );

        connect(btn, &QPushButton::clicked, this, [=](){
            refreshGoods(m_currentKeyword, cat);
        });
        catLayout->addWidget(btn);
    }
    catLayout->addStretch();
    mainLayout->addWidget(categoryBar);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    // 隐藏横向滚动条，美化纵向滚动条在 MainWindow 已做，这里确保背景一致
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: #F7F8FA; }");

    m_scrollContent = new QWidget();
    m_scrollContent->setStyleSheet("background-color: #F7F8FA; border: none;");
    m_gridLayout = new QGridLayout(m_scrollContent);
    m_gridLayout->setContentsMargins(40, 20, 40, 40);
    m_gridLayout->setSpacing(25);
    m_gridLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    scrollArea->setWidget(m_scrollContent);
    mainLayout->addWidget(scrollArea);

    connect(m_btnPublish, &QPushButton::clicked, this, &HomePage::goPublishSignal);
    connect(m_btnUser, &QPushButton::clicked, this, &HomePage::goUserCenterSignal);
    connect(m_btnAdmin, &QPushButton::clicked, this, &HomePage::goUserCenterSignal);
    connect(m_btnExit, &QPushButton::clicked, this, &HomePage::logoutSignal);
}
