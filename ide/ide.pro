#-------------------------------------------------
#
# Project created by QtCreator 2015-02-22T10:59:38
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = fakeide
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

RESOURCES += \
    Images/appres.qrc

DISTFILES += \
    ide.rc
	
RC_FILE = \
  ide.rc
  