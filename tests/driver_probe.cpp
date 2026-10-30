#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSettings>
#include <QTextStream>
int main(int argc,char **argv){
 QCoreApplication a(argc,argv);auto d=QSqlDatabase::addDatabase("QMYSQL");
 QTextStream(stdout)<<"QMYSQL="<<d.isValid()<<Qt::endl;if(!d.isValid())return 1;
 if(a.arguments().contains("--connect")){
  QSettings c(QCoreApplication::applicationDirPath()+"/database.ini",QSettings::IniFormat);
  d.setHostName(c.value("mysql/host").toString());d.setPort(c.value("mysql/port").toInt());d.setDatabaseName(c.value("mysql/database").toString());d.setUserName(c.value("mysql/user").toString());d.setPassword(c.value("mysql/password").toString());
  if(!d.open()){QTextStream(stderr)<<d.lastError().text()<<Qt::endl;return 2;}
  QTextStream(stdout)<<"MYSQL_CONNECTED=1"<<Qt::endl;
 }return 0;
}
