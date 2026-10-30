QT += core gui sql widgets charts
TARGET = CampusMarket
TEMPLATE = app
CONFIG += c++17
INCLUDEPATH += src
SOURCES += main.cpp $$files(src/*.cpp)
HEADERS += $$files(src/*.h)
RESOURCES += resources.qrc

CAMPUS_SOURCE_ROOT = $$PWD
include(tools/runtime.pri)
