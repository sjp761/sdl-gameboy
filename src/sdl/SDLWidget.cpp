#include "SDLWidget.h"
#include "SDLContainer.h"
#include "ppu.h"
#include "SDL_TileViewer.h"
#include "SDL_TileMapViewer.h"
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QTimer>
#include <cstdint>

SDLWidget::SDLWidget(QWidget *parent)
    : QWidget(parent)
{
    setLayout(new QVBoxLayout(this));
    sdlcon.initSDL();
    SDL_PumpEvents();
    sdlcon.createNativeWindow();
    if (sdlcon.embedded) {
        layout()->addWidget(QWidget::createWindowContainer(sdlcon.embedded, this));
    }

    refreshTimer = new QTimer(this);
    refreshTimer->setInterval(16); // ~60 FPS
    connect(refreshTimer, &QTimer::timeout, this, &SDLWidget::renderFrame);
    refreshTimer->start();
}

SDLWidget::~SDLWidget()
{
    if (refreshTimer) {
        refreshTimer->stop();
    }
}

void SDLWidget::renderFrame()
{
    auto emu = emu_ref.lock();
    if (emu) {
        uint8_t screenBuffer[SCREEN_BUFFER_SIZE];
        emu->get_ppu().copy_screen_buffer(screenBuffer);
        sdlcon.render(screenBuffer);

        // Update tile viewer if active
        if (tile_viewer_ptr) {
            if (tile_viewer_ptr->is_open()) {
                std::lock_guard<std::mutex> vram_lock(emu->get_ppu().get_vram_mutex());
                tile_viewer_ptr->update(emu->get_ppu().get_vram_ptr());
                tile_viewer_ptr->render();
            }
        }
        // Update tile map viewer if active
        if (tile_map_viewer_ptr) {
            if (tile_map_viewer_ptr->is_open()) {
                std::lock_guard<std::mutex> vram_lock(emu->get_ppu().get_vram_mutex());
                tile_map_viewer_ptr->update(emu->get_ppu().get_tilemap_ptr());
                tile_map_viewer_ptr->render();
            }
        }
    } else {
        sdlcon.render(nullptr);
    }
}

void SDLWidget::paintEvent(QPaintEvent *event)
{
    renderFrame();
}

void SDLWidget::resizeEvent(QResizeEvent *event)
{
    sdlcon.resize(width(), height());
}
