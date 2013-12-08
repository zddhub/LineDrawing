/*
#-------------------------------------------------
#
# LineDrawingWidget created by QtCreator
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
*/
#include <QtGui/QApplication>
#include "linedrawingwidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LineDrawingWidget w;

    w.readMesh("./data/horse.obj");
    w.show();

    return a.exec();
}
