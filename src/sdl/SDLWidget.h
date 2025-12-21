#ifndef SDLWIDGET_H
#define SDLWIDGET_H

#include <QWidget>
#include <memory>
#include "SDLContainer.h"
#include <emu.h>

class QTimer;
class SDL_TileViewer;

class SDL_TileMapViewer;

class SDLWidget : public QWidget
{
    Q_OBJECT

public:
    SDLWidget(QWidget *parent = nullptr);
    ~SDLWidget();
    std::weak_ptr<Emu> emu_ref; // Weak pointer to emulator instance, set by MainWindow so it owns it
    SDL_TileViewer* tile_viewer_ptr{nullptr};
    SDL_TileMapViewer* tile_map_viewer_ptr{nullptr};
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    SDLContainer sdlcon;
    QTimer* refreshTimer{nullptr};
    void renderFrame();

};

#endif // SDLWIDGET_H