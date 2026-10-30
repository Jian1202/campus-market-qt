#include "src/MainWindow.h"
#include "src/DbManager.h"
#include <QApplication>
#include <QFile>
#include <QMessageBox>

// main.cpp
int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Qt 6 以下才需要这一句，Qt 6 会报警告
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication a(argc, argv);
    // ... 后面的不变

    // 2. 加载 QSS 样式表
    QFile styleFile(":/style.qss");
    if (styleFile.open(QIODevice::ReadOnly)) {
        a.setStyleSheet(styleFile.readAll());
        styleFile.close();
    }

    // 3. 初始化数据库
    if(!DbManager::instance().isOpen()) {QMessageBox::critical(nullptr,"数据库连接失败",DbManager::instance().lastError());return 1;}

    // 4. 显示主窗口
    MainWindow w;
    w.show();

    return a.exec();
}
