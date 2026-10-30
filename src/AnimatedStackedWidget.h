#ifndef ANIMATEDSTACKEDWIDGET_H
#define ANIMATEDSTACKEDWIDGET_H

#include <QStackedWidget>
#include <QPropertyAnimation>
#include <QEasingCurve>

class AnimatedStackedWidget : public QStackedWidget {
    Q_OBJECT
public:
    explicit AnimatedStackedWidget(QWidget *parent = nullptr) : QStackedWidget(parent) {
        m_duration = 400;
        m_easing = QEasingCurve::OutCubic;
    }

    void slideInIdx(int idx) {
        if (currentIndex() == idx) return;

        QWidget *newWidget = widget(idx);
        newWidget->setGeometry(0, 0, width(), height());

        QPropertyAnimation *animNew = new QPropertyAnimation(newWidget, "windowOpacity");
        animNew->setDuration(m_duration);
        animNew->setStartValue(0.0);
        animNew->setEndValue(1.0);
        animNew->setEasingCurve(m_easing);

        setCurrentIndex(idx);
        animNew->start(QAbstractAnimation::DeleteWhenStopped);
    }

private:
    int m_duration;
    QEasingCurve::Type m_easing;
};

#endif // ANIMATEDSTACKEDWIDGET_H
