#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include "AnimatedStackedWidget.h"
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QFrame>
#include <QGraphicsDropShadowEffect>

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // 只保留鼠标事件，用于移动窗口
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    // 删除了 changeEvent，不再监听最大化/还原变化

private:
    void initUi();
    void setupTitleBar(QVBoxLayout *layout);

    AnimatedStackedWidget *m_stack;
    QFrame *m_mainFrame;
    QVBoxLayout *m_rootLayout;
    QGraphicsDropShadowEffect *m_shadow;

    QPoint m_dragPosition;
    bool m_isDragging = false;

    int m_lastPageIndex = 2;
};

#endif // MAINWINDOW_H
