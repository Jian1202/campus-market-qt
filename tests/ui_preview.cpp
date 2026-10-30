#include "DbManager.h"
#include "HomePage.h"
#include "PublishPage.h"
#include "ProductDetail.h"
#include "AdminPage.h"
#include "TradeCenter.h"
#include <QApplication>
#include <QTimer>
#include <QDir>
#include <QFile>
#include <QTabWidget>
#include <QComboBox>

// 仅供开发验证的独立预览入口，不编入主程序。
int main(int argc,char **argv) {
    QApplication app(argc,argv);
    QFile style(":/style.qss");if(style.open(QIODevice::ReadOnly))app.setStyleSheet(style.readAll());
    auto &db=DbManager::instance();
    if(!db.isOpen()||!db.loginUser("admin","admin123")) return 1;
    QString page=app.arguments().value(1,"center");QWidget *window=nullptr;
    if(page=="home"){auto w=new HomePage;w->refreshGoods();window=w;}
    else if(page=="publish"){auto w=new PublishPage;w->clearFields();window=w;}
    else if(page=="detail"){auto w=new ProductDetail;auto data=db.getAllGoods();if(data.isEmpty())return 1;w->setProductData(data.first());window=w;}
    else if(page=="admin"){auto w=new AdminPage;w->refreshData();window=w;}
    else {
        window=new TradeCenter;
        if(page=="reports") {auto t=window->findChild<QTabWidget*>();t->setCurrentIndex(4);}
        if(page=="moderation") {auto t=window->findChild<QTabWidget*>();t->setCurrentIndex(3);}
    }
    window->setWindowTitle("校园二手交易平台界面验证 - "+page);
    window->resize(1200,800);window->show();
    QString dir=app.arguments().value(2);
    if(!dir.isEmpty()) QTimer::singleShot(1500,window,[window,dir,page](){QDir().mkpath(dir);window->grab().save(dir+"/"+page+".png");QApplication::quit();});
    return app.exec();
}
