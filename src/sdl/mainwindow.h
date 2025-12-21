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
    std::atomic<std::shared_ptr<Emu>> emu_ref;
    SDL_TileViewer tile_viewer;

signals:
    void requestOpenTileViewer();

public slots:
    void handleRecentFileAction(QAction* action);
    void openTileViewer();

private:
    Ui::MainWindow *ui;
    std::thread emuThread;
    std::mutex tile_viewer_mutex;  // Protects tile_viewer access
    void startEmulator(const std::string& romPath);
};
#endif // MAINWINDOW_H
