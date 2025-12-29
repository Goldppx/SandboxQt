//
// Created by GoldesqueMoON on 2025/12/26.
//

#include "SandboxRenderer.h"

#include "SandboxInput.h"

void SandboxRenderer::render(QPainter &painter, const SandboxWorld &world, const QRect &targetRect, const Brush &brush, const QPoint &mousePos) {
    const QImage image (
        reinterpret_cast<const uchar *>(world.getBufferData()),
        world.width,
        world.height,
        QImage::Format_ARGB32
    );
    painter.save();
    painter.setRenderHint(QPainter::SmoothPixmapTransform,false);
    painter.drawImage(targetRect, image);
    painter.restore();
    if (!mousePos.isNull()) {
        const int radius = targetRect.width() / world.width * brush.getSize();
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing);
        constexpr QColor cursorColor(255, 255, 255, 120);
        const QPen pen(cursorColor, 5, Qt::DashLine);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(mousePos, radius, radius);
        painter.restore();
    }
    QString text;
    QFont font("JetBrains Mono",24);
    font.setBold(true);
    font.setStyleStrategy(QFont::PreferAntialias);
    switch (brush.getType()) {
        case ParticleType::Sand: text += "Sand"; break;
        case ParticleType::Wall: text += "Wall"; break;
        case ParticleType::Air:  text += "Air"; break;
        case ParticleType::Water: text += "Water"; break;
        case ParticleType::Grass: text += "Grass"; break;
    }
    painter.setPen(Qt::black);
    painter.setFont(font);
    const QString info = QString("FPS: %1 | Brush: %2").arg(world.getFPS()).arg(text);
    painter.drawText(20,40, info);
}
