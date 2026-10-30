#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QButtonGroup>

class SideBar : public QWidget {
    Q_OBJECT
public:
    explicit SideBar(QWidget *parent = nullptr) : QWidget(parent) {
        setFixedWidth(200);
        setStyleSheet("SideBar { background-color: #1A1A1A; border-top-left-radius: 12px; border-bottom-left-radius: 12px; }");

        m_layout = new QVBoxLayout(this);
        m_layout->setContentsMargins(0, 20, 0, 20);
        m_layout->setSpacing(5);

        m_btnGroup = new QButtonGroup(this);
        m_btnGroup->setExclusive(true);
    }

    void addAction(const QString &text, int id, bool isBottom = false) {
        QPushButton *btn = new QPushButton(text, this);
        btn->setCheckable(true);
        btn->setFixedHeight(50);
        btn->setCursor(Qt::PointingHandCursor);

        // 返回首页按钮特殊样式
        if (isBottom) {
            btn->setStyleSheet(
                "QPushButton {"
                "   background-color: #FF7875;"
                "   color: white;"
                "   border: none;"
                "   border-radius: 8px;"
                "   text-align: center;"
                "   padding: 0px;"
                "   margin: 0px 15px;"
                "   font-size: 14px;"
                "   font-weight: bold;"
                "   letter-spacing: 1px;"
                "}"
                "QPushButton:hover { background-color: #FF4D4F; }"
                "QPushButton:pressed { background-color: #D9363E; }"
            );
        } else {
            btn->setStyleSheet(
                "QPushButton {"
                "   background-color: transparent;"
                "   color: #999;"
                "   border: none;"
                "   text-align: left;"
                "   padding-left: 30px;"
                "   font-size: 14px;"
                "   font-weight: bold;"
                "}"
                "QPushButton:hover { color: #FFF; background-color: #333; }"
                "QPushButton:checked { color: #FFF; background-color: #333; border-left: 4px solid #FFF; }"
            );
        }

        m_btnGroup->addButton(btn, id);

        if (isBottom) {
            m_layout->addStretch();
            m_layout->addWidget(btn);
        } else {
            m_layout->addWidget(btn);
        }

        connect(btn, &QPushButton::clicked, [this, id](){
            emit actionTriggered(id);
        });
    }

    void setChecked(int id) {
        if (m_btnGroup->button(id)) {
            m_btnGroup->button(id)->setChecked(true);
        }
    }

signals:
    void actionTriggered(int id);

private:
    QVBoxLayout *m_layout;
    QButtonGroup *m_btnGroup;
};

#endif // SIDEBAR_H
