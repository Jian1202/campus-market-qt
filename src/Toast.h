#ifndef TOAST_H
#define TOAST_H

#include <QWidget>
#include <QLabel>
#include <QPropertyAnimation>
#include <QTimer>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>

class Toast : public QWidget {
    Q_OBJECT
public:
    static void show(QWidget *parent, const QString &message, int duration = 2000) {
        Toast *toast = new Toast(parent, message);
        toast->start(duration);
    }

private:
    Toast(QWidget *parent, const QString &message) : QWidget(parent) {
        setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip);
        setAttribute(Qt::WA_TranslucentBackground);

        m_label = new QLabel(message, this);
        m_label->setStyleSheet(
            "QLabel {"
            "   background-color: #1A1A1A;"
            "   color: #FFFFFF;"
            "   padding: 10px 20px;"
            "   border-radius: 20px;"
            "   font-size: 14px;"
            "   font-weight: bold;"
            "}"
        );

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(m_label);
        layout->setContentsMargins(0, 0, 0, 0);

        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
        shadow->setBlurRadius(15);
        shadow->setColor(QColor(0, 0, 0, 80));
        shadow->setOffset(0, 4);
        m_label->setGraphicsEffect(shadow);

        adjustSize();

        if (parent) {
            int x = (parent->width() - width()) / 2;
            int y = 60;
            move(parent->mapToGlobal(QPoint(x, y)));
        }
    }

    void start(int duration) {
        QPropertyAnimation *fadeIn = new QPropertyAnimation(this, "windowOpacity");
        fadeIn->setDuration(300);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);

        QPropertyAnimation *fadeOut = new QPropertyAnimation(this, "windowOpacity");
        fadeOut->setDuration(300);
        fadeOut->setStartValue(1.0);
        fadeOut->setEndValue(0.0);
        // QPropertyAnimation 没有 setStartDelay，使用 QTimer 延迟启动

        connect(fadeIn, &QPropertyAnimation::finished, [=]() {
            QTimer::singleShot(duration, [=]() {
                fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
            });
        });

        connect(fadeOut, &QPropertyAnimation::finished, this, &Toast::deleteLater);

        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
        QWidget::show(); // 调用基类的 show()
    }

    QLabel *m_label;
};

#endif // TOAST_H
