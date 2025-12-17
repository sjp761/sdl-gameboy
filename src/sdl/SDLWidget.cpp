#include "SDLWidget.h"
#include "SDLContainer.h"
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QTimer>

SDLWidget::SDLWidget(QWidget *parent)
    : QWidget(parent)
{
    this->setLayout(new QVBoxLayout(this));
    sdlcon.initSDL();
    SDL_PumpEvents();
    sdlcon.createNativeWindow();
    if (sdlcon.embedded) {
        this->layout()->addWidget(QWidget::createWindowContainer(sdlcon.embedded, this));
    }

    // Periodically refresh the SDL renderer to update the embedded display.
    refreshTimer = new QTimer(this);
    refreshTimer->setInterval(16); // ~60 FPS
    connect(refreshTimer, &QTimer::timeout, this, [this]() { sdlcon.render(); });
    refreshTimer->start();
}

SDLWidget::~SDLWidget()
{
    //blank (for now at least)
}

void SDLWidget::paintEvent(QPaintEvent *event)
{
    sdlcon.render();
}

void SDLWidget::resizeEvent(QResizeEvent *event)
{
    sdlcon.resize(width(), height());
}
