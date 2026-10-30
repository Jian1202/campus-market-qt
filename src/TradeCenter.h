#pragma once
#include <QDialog>
#include <QTableWidget>
#include <QTabWidget>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <functional>
class TradeCenter : public QDialog {
public:
    explicit TradeCenter(QWidget *parent=nullptr);
private:
    void refresh();
    void result(bool ok);
    void action(QTableWidget *table,int row,int column,const QString &label,std::function<void()> callback);
    QTabWidget *tabs;
    QTableWidget *requests,*orders,*mine,*reports=nullptr,*stats=nullptr;
    QLabel *summary;
    QComboBox *reportChoice=nullptr;
    QSpinBox *threshold=nullptr;
};
