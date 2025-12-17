#ifndef SDLWIDGET_H
#define SDLWIDGET_H

#include <QWidget>
#include "SDLContainer.h"

class QTimer;

class SDLWidget : public QWidget
{
    Q_OBJECT

public:
    SDLWidget(QWidget *parent = nullptr);
    ~SDLWidget();
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    SDLContainer sdlcon;
    QTimer* refreshTimer{nullptr};

};

#endif // SDLWIDGET_H