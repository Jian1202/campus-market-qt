QT += core sql
CONFIG += console c++17
CONFIG -= app_bundle
TEMPLATE = app
TARGET = DriverProbe
SOURCES = driver_probe.cpp
CAMPUS_SOURCE_ROOT = $$clean_path($$PWD/..)
include(../tools/runtime.pri)
