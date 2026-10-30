#include "PublishPage.h"
#include "DbManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollArea>
#include <QPixmap>

PublishPage::PublishPage(QWidget *parent):QWidget(parent) {
    setStyleSheet("PublishPage {background:#F5F5F7;}");
    auto root=new QVBoxLayout(this);root->setContentsMargins(24,24,24,24);auto scroll=new QScrollArea;scroll->setWidgetResizable(true);scroll->setFrameShape(QFrame::NoFrame);root->addWidget(scroll);
    auto outer=new QWidget;auto outerLayout=new QHBoxLayout(outer);outerLayout->setContentsMargins(0,0,0,0);
    auto page=new QWidget;page->setMaximumWidth(860);auto layout=new QVBoxLayout(page);layout->setContentsMargins(36,28,36,30);layout->setSpacing(20);
    outerLayout->addStretch();outerLayout->addWidget(page,1);outerLayout->addStretch();
    auto title=new QLabel("发布校园二手商品");title->setStyleSheet("font-size:26px;font-weight:bold;");layout->addWidget(title);
    auto hint=new QLabel("填写商品信息，让同学更快了解你的闲置物品");hint->setStyleSheet("color:#858B94;font-size:13px;");layout->addWidget(hint);
    auto form=new QFormLayout;form->setSpacing(18);layout->addLayout(form);
    m_nameEdit=new QLineEdit;m_nameEdit->setObjectName("productTitle");m_nameEdit->setMaxLength(160);
    m_maxPrice=new QLineEdit;m_maxPrice->setObjectName("productPrice");m_maxPrice->setPlaceholderText("元，最多两位小数；允许赠送 0 元");
    m_condition=new QLineEdit;m_condition->setObjectName("productCondition");m_condition->setMaxLength(200);m_condition->setPlaceholderText("例如：九成新，屏幕无划痕，电池正常");
    m_categoryCombo=new QComboBox;
    m_descEdit=new QTextEdit;m_descEdit->setObjectName("productDescription");m_descEdit->setMinimumHeight(120);m_descEdit->setMaximumHeight(180);
    m_imageBtn=new QPushButton("选择商品图片（可选）");
    form->addRow("商品标题",m_nameEdit);form->addRow("售价",m_maxPrice);form->addRow("分类",m_categoryCombo);form->addRow("成色描述",m_condition);form->addRow("详细描述",m_descEdit);form->addRow("图片",m_imageBtn);
    auto buttons=new QHBoxLayout;auto cancel=new QPushButton("返回首页");auto submit=new QPushButton("发布商品");cancel->setObjectName("secondaryButton");submit->setObjectName("primaryButton");
    m_imageBtn->setObjectName("secondaryButton");buttons->addStretch();buttons->addWidget(cancel);buttons->addWidget(submit);layout->addLayout(buttons);layout->addStretch();scroll->setWidget(outer);
    page->setStyleSheet("QLineEdit,QTextEdit,QComboBox {background:white;color:#222;border:1px solid #ccc;padding:10px;border-radius:5px;} QPushButton {padding:10px 24px;background:#222;color:white;border-radius:6px;min-height:20px;} QPushButton#secondaryButton {background:#F4F5F7;color:#555;border:1px solid #E2E5E9;} QPushButton:hover {background:#3C3C3C;} QPushButton#secondaryButton:hover {background:#E9ECF0;} QLabel {color:#222;}");
    connect(cancel,&QPushButton::clicked,this,&PublishPage::goHomeSignal);connect(submit,&QPushButton::clicked,this,&PublishPage::onPublish);connect(m_imageBtn,&QPushButton::clicked,this,&PublishPage::onUploadImage);
}
void PublishPage::clearFields() {
    m_nameEdit->clear();m_maxPrice->clear();m_condition->clear();m_descEdit->clear();m_selectedPath.clear();m_imageBtn->setText("选择商品图片（可选）");
    m_categoryCombo->clear();for(const auto &r:DbManager::instance().rows("SELECT * FROM Category ORDER BY id"))m_categoryCombo->addItem(r["name"].toString(),r["id"]);
}
void PublishPage::onUploadImage() {
    QString file=QFileDialog::getOpenFileName(this,"选择图片",{},"图片 (*.png *.jpg *.jpeg *.bmp)");
    if(!file.isEmpty()){m_selectedPath=file;m_imageBtn->setText("已选择图片，点击重新选择");}
}
void PublishPage::onPublish() {
    bool ok;QString value=m_maxPrice->text().trimmed();double price=value.toDouble(&ok);
    if(!ok||price<0||price>99999999.99||(value.contains('.')&&value.section('.',1).size()>2)) {QMessageBox::warning(this,"格式错误","请输入有效售价，最多两位小数");return;}
    QString image;
    if(!m_selectedPath.isEmpty()) {
        image=DbManager::saveUploadedImage(QPixmap(m_selectedPath));
        if(image.isEmpty()){QMessageBox::warning(this,"图片保存失败","请重新选择有效图片");return;}
    }
    auto &db=DbManager::instance();
    if(!db.addProduct(m_nameEdit->text(),price,m_descEdit->toPlainText(),m_condition->text(),m_categoryCombo->currentData().toInt(),image)){QMessageBox::warning(this,"发布失败",db.lastError());return;}
    QMessageBox::information(this,"发布成功","商品已发布");clearFields();emit goHomeSignal();
}
