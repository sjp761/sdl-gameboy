#include "SDLWidget.h"
#include "SDLContainer.h"
#include "ppu.h"
#ifdef ENABLE_DEBUG_VIEWERS
#include "SDL_TileViewer.h"
#include "SDL_TileMapViewer.h"
#endif
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
        // Swap PPU buffers so rendering thread can safely read front buffer
        emu->get_ppu().swap_buffers();
        
        // Render main screen using front buffer
        sdlcon.render(emu->get_ppu().get_screen_buffer());

#ifdef ENABLE_DEBUG_VIEWERS
        // Lock VRAM mutex while accessing buffers for debug viewers
        std::lock_guard<std::mutex> lock(emu->get_ppu().get_vram_mutex());
        
        // Update tile viewer if active
        if (tile_viewer_ptr) {
            if (tile_viewer_ptr->is_open()) {
                tile_viewer_ptr->update(emu->get_ppu().get_vram_buffer());
                tile_viewer_ptr->render();
            }
        }
        // Update tile map viewer if active
        if (tile_map_viewer_ptr) {
            if (tile_map_viewer_ptr->is_open()) {
                tile_map_viewer_ptr->update(emu->get_ppu().get_tilemap_buffer());
                tile_map_viewer_ptr->render();
            }
        }
#endif
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
