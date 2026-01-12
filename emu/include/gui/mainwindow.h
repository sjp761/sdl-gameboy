#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <thread>
#include <atomic>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include "emu.h"
#ifdef ENABLE_DEBUG_VIEWERS
#include "SDL_TileViewer.h"
#include "SDL_TileMapViewer.h"
#endif
class SDLWidget;
class Emu;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void openFile();
    QMenu* menuRecent;
    Ui::MainWindow *getUi() const { return ui; }
    void updateRecentFileMenu();
    std::set<std::string> recentFiles;
    void fillRecentSet();
    void saveRecentSet();
    SDLWidget* getSDLWidget();
    std::shared_ptr<Emu> emu_ref; // Current emulator instance
    std::mutex emu_ref_mutex; // Protects emu_ref access during thread start/stop (gets assigned on different thread than the main one)

    #ifdef ENABLE_DEBUG_VIEWERS
        SDL_TileViewer tile_viewer;
        SDL_TileMapViewer tile_map_viewer;
    #endif

    signals:

    #ifdef ENABLE_DEBUG_VIEWERS
        void requestOpenTileViewer();
        void requestOpenTileMapViewer();
    #endif

public slots:
    void handleRecentFileAction(QAction* action);

#ifdef ENABLE_DEBUG_VIEWERS
    void openTileViewer();
    void openTileMapViewer();
#endif

private:
    Ui::MainWindow *ui;
    std::thread emuThread;
    void startEmulator(const std::string& romPath, const std::string& bootromPath = "roms/dmg_boot.gb");
};
#endif // MAINWINDOW_H
