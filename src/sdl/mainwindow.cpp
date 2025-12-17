#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "SDLWidget.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QWindow>
#include <QLayout>
#include <QTimer>
#include <iostream>
#include <fstream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "emu.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    menuRecent = new QMenu(tr("Recent Files"), this);
    ui->setupUi(this);
    ui->menuFile->addMenu(menuRecent);
    connect(menuRecent, &QMenu::triggered, this, &MainWindow::handleRecentFileAction); // Anytime an action is triggered in the recent files menu (click one of the recent files), call handleRecentFileAction
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    // Removed image conversion action; only loading remains
}

MainWindow::~MainWindow()
{
    delete ui;
    saveRecentSet(); // Save recent files before closing
}

void MainWindow::openFile()
{
    // Load a Game Boy ROM and start emulator
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open ROM"), "", tr("Game Boy ROMs (*.gb *.gbc);;All Files (*)"));
    if (!fileName.isEmpty()) {
        const std::string path = fileName.toStdString();
        MainWindow::setWindowTitle(fileName);
        recentFiles.insert(path);
        updateRecentFileMenu();
        startEmulator(path);
    }
}


// Removed image conversion/save functionality

void MainWindow::updateRecentFileMenu()
{
    menuRecent->clear(); // Clear the existing actions in the menu
    for (const std::string &file : recentFiles) 
    {
        QAction *action = new QAction(QString::fromStdString(file), this);
        action->setData(QString::fromStdString(file));
        menuRecent->addAction(action);
    }
}

void MainWindow::fillRecentSet()
{
    std::ifstream file("recent_files.txt");
    std::string line;
    while (std::getline(file, line)) {
        recentFiles.insert(line);
    }
}

void MainWindow::saveRecentSet()
{
    std::ofstream file("recent_files.txt");
    for (const auto &string : recentFiles) {
        file << string << std::endl;
    }
}

SDLWidget *MainWindow::getSDLWidget()
{
    return ui->centralwidget;
}

void MainWindow::handleRecentFileAction(QAction *action)
{
    std::cout << "Recent file action triggered: " << action->data().toString().toStdString() << std::endl;
    const std::string path = action->data().toString().toStdString();
    MainWindow::setWindowTitle(action->data().toString());
    startEmulator(path);
}

void MainWindow::startEmulator(const std::string& romPath)
{
    if (emuThread.joinable()) {
        // In a fuller implementation, signal emulator to stop, then join.
        emuThread.detach();
    }
    emuThread = std::thread([romPath]() {
        Emu emu;
        emu.set_component_pointers();
        emu.get_rom().cart_load(romPath.c_str());
        emu.ctx.running = true;
        emu.ctx.paused = false;
        emu.ctx.ticks = 0;
        emu.get_cpu().cpu_init();

        // Basic run loop; rendering hookup will be added later
        while (emu.ctx.running) {
            if (emu.ctx.paused) { SDL_Delay(10); continue; }
            if (!emu.get_cpu().cpu_step()) {
                SDL_Delay(1);
            }
            emu.ctx.ticks++;
        }
    });
}
