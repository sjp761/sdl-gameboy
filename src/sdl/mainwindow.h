#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <thread>
#include <set>
#include <string>
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


public slots:
    void handleRecentFileAction(QAction* action);

private:
    Ui::MainWindow *ui;
    std::thread emuThread;
    void startEmulator(const std::string& romPath);
};
#endif // MAINWINDOW_H
