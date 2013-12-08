#-------------------------------------------------
#
# Project created by QtCreator
#
# @Author: zdd
# @Email: zddhub@gmail.com
#
# rstc.cc in Qt
#
# Thanks:
#    Original by Tilke Judd
#    Tweaks by Szymon Rusinkiewicz
#
#    apparentridge.h
#    Compute apparent ridges.
#
#    Implements method of
#      Judd, T., Durand, F, and Adelson, E.
#      Apparent Ridges for Line Drawing,
#      ACM Trans. Graphics (Proc. SIGGRAPH), vol. 26, no. 3, 2007.
#-------------------------------------------------

QT       += core gui \
            opengl

TARGET = LineDrawing
TEMPLATE = app


SOURCES += main.cpp \
    linedrawingwidget.cpp \
    rtsc.cpp \
    apparentridge.cpp

HEADERS  += \
    linedrawingwidget.h

INCLUDEPATH += .\include


LIBS += .\lib\trimeshd.lib














