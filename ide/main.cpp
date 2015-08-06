#include "mainwindow.h"
#include <QApplication>

QApplication * g_ide;

int main(int argc, char *argv[])
{
    QApplication ide(argc, argv);
    g_ide = &ide;

    MainWindow w;
    w.showMaximized();
    w.show();

    return ide.exec();
}
