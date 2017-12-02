#-------------------------------------------------
#
# Project created by QtCreator 2014-01-06T21:12:42
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = dupfile
TEMPLATE = app
#-fwide-exec-charset=utf-32也会让宽字符串带上BOM（Linux）
unix:QMAKE_CXXFLAGS += -std=gnu++14 -finput-charset=gb18030 -fexec-charset=utf-8  #-municode -DUNICODE -D_UNICODE
unix:QMAKE_LFLAGS += -Wl,-rpath,/home/whg/soft/gcc/lib64
win32: {
QMAKE_CXXFLAGS += -DUNICODE -D_UNICODE
QMAKE_LFLAGS +=/ENTRY:"wmainCRTStartup"
QMAKE_CFLAGS +=-Zc:wchar_t
QMAKE_LIBS += user32.lib gdi32.lib
}

CONFIG += debug

SOURCES += main.cpp\
        mainwindow.cpp \
    comparethread.cpp

HEADERS  += mainwindow.h \
    comparethread.h

FORMS    += mainwindow.ui

RESOURCES += \
    dupfile.qrc
