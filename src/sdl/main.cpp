#include "mainwindow.h"
#include "SDLWidget.h"
#include <QApplication>
#include <SDL3/SDL.h>
#include <iostream>
#include <SDL3_image/SDL_image.h>

int main(int argc, char *argv[])
{
    setenv("QT_QPA_PLATFORM", "xcb", 1);
    setenv("SDL_VIDEODRIVER", "x11", 1);
    QApplication a(argc, argv);
    MainWindow w;
    w.fillRecentSet();
    w.updateRecentFileMenu();
    w.show();
    return a.exec();
}
