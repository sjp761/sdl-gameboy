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
#include "SDL_TileViewer.h"
#include "SDL_TileMapViewer.h"
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
    std::shared_ptr<Emu> emu_ref;
    std::mutex emu_ref_mutex; // Protects emu_ref access
    SDL_TileViewer tile_viewer;
    SDL_TileMapViewer tile_map_viewer;

signals:
    void requestOpenTileViewer();
    void requestOpenTileMapViewer();

public slots:
    void handleRecentFileAction(QAction* action);
    void openTileViewer();
    void openTileMapViewer();

private:
    Ui::MainWindow *ui;
    std::thread emuThread;
    void startEmulator(const std::string& romPath);
};
#endif // MAINWINDOW_H
