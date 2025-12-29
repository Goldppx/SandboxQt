//
// Created by GoldesqueMoON on 2025/12/26.
//

#ifndef SANDBOXQT_SANDBOXRENDERER_H
#define SANDBOXQT_SANDBOXRENDERER_H

#include <QPainter>
#include <QImage>
#include <QFont>
#include "SandboxWorld.h"

class Brush;

class SandboxRenderer {
    public:
    static void render(QPainter &painter, const SandboxWorld &world, const QRect &targetRect, const Brush &brush, const QPoint &mousePos);
};


#endif //SANDBOXQT_SANDBOXRENDERER_H