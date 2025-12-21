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
    // Connect signal to slot for opening tile viewer on main thread
    connect(this, &MainWindow::requestOpenTileViewer, this, &MainWindow::openTileViewer, Qt::QueuedConnection);
    // Removed image conversion action; only loading remains
}

MainWindow::~MainWindow()
{
    // Stop the emulator thread properly
    auto emu = emu_ref.load();
    if (emu) {
        emu->ctx.running = false;
    }
    
    if (emuThread.joinable()) {
        emuThread.join();  // Wait for the thread to finish before destroying the MainWindow, bool will make it stop
    }
    
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
    QString filePath = action->data().toString();
    std::cout << "Recent file action triggered: " << filePath.toStdString() << std::endl;
    setWindowTitle(filePath);
    startEmulator(filePath.toStdString());
}

void MainWindow::startEmulator(const std::string& romPath)
{
    // Stop previous emulator if running
    auto old_emu = emu_ref.load();
    if (old_emu) {
        old_emu->ctx.running = false;
    }
    
    if (emuThread.joinable()) {
        emuThread.join();
    }
    
    // Signal to open tile viewer on main thread
    emit requestOpenTileViewer();
    
    // Create new emulator atomically
    auto new_emu = std::make_shared<Emu>();
    new_emu->set_component_pointers();
    new_emu->get_rom().cart_load(romPath.c_str());
    new_emu->ctx.running = true;
    new_emu->ctx.paused = false;
    new_emu->ctx.ticks = 0;
    new_emu->get_cpu().cpu_init();
    
    emu_ref.store(new_emu);
    ui->centralwidget->emu_ref = new_emu;  // weak_ptr assignment
    
    emuThread = std::thread([this]() {
        auto emu = emu_ref.load(); // Atomic load of shared_ptr
        uint64_t last_tile_update = 0;
        
        while (emu->ctx.running) {
            if (emu->ctx.paused) { SDL_Delay(10); continue; }
            if (!emu->get_cpu().cpu_step()) {
                SDL_Delay(1);
            }
            emu->ctx.ticks++;
            
            // Update tile viewer periodically (every 16ms ~60fps)
            if ((emu->ctx.ticks - last_tile_update) > 70224) { // ~60 times per second
                std::lock_guard<std::mutex> tv_lock(tile_viewer_mutex);
                if (tile_viewer.is_open()) {
                    // Lock VRAM for reading
                    std::lock_guard<std::mutex> vram_lock(emu->get_ppu().get_vram_mutex());
                    tile_viewer.update(emu->get_ppu().get_vram_ptr());
                    tile_viewer.render();
                    last_tile_update = emu->ctx.ticks;
                }
            }
        }
    });
}

void MainWindow::openTileViewer()
{
    // This runs on the main thread (Qt's event loop thread)
    std::lock_guard<std::mutex> lock(tile_viewer_mutex);
    if (!tile_viewer.is_open()) {
        tile_viewer.init();
    }
}
