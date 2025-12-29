//SandboxWorld.h

#ifndef SANDBOXQT_SANDBOXWORLD_H
#define SANDBOXQT_SANDBOXWORLD_H
#pragma once
#include <QTimer>
#include <QPainter>
#include <QElapsedTimer>

QT_BEGIN_NAMESPACE

QT_END_NAMESPACE

enum class ParticleType {
    Air = 0,
    Sand = 1,
    Wall = 2,
    Water = 3,
    Grass = 4
};

struct Particle {
    ParticleType type = ParticleType::Air;
    int pressure = 0;
    bool updated = false;
    int8_t colorVariation = 0;
    uint32_t color = 0x80FFFFFF;
};

class SandboxWorld final : public QObject {
    Q_OBJECT
public:
    int width, height;
    QTimer *timer;
    void stop() const { timer->stop(); }
    void start() const { timer->start(); }
    SandboxWorld(int w, int h);

    void setParticle(int x, int y, ParticleType type);

    static uint32_t calculateColor(const Particle &p);

    void clearWorld();
    void update();
    void step();

    [[nodiscard]] QTimer* getTimer() const { return timer; }
    [[nodiscard]] const uint32_t* getBufferData() const { return buffer.data(); }
    [[nodiscard]] std::vector<Particle> getGrid() const { return grid; }
    [[nodiscard]] int getFPS() const { return currentFPS; }
    [[nodiscard]] bool getPaused() const { return isPaused; }
    void setPaused(bool p) { isPaused = p; }

    signals:
    void worldChanged();
private:
    bool isPaused = false;
    std::vector<Particle> grid;
    std::vector<uint32_t> buffer;
    int frameCount = 0;
    int currentFPS = 0;
    QElapsedTimer fpsTimer;
};

static uint32_t fast_rand() {
    static uint32_t x = 123456789, y = 362436069, z = 521288629;
    x ^= x << 16;
    x ^= x >> 5;
    x ^= x << 1;
    const uint32_t t = x;
    x = y;
    y = z;
    z = t ^ x ^ y;
    return z;
}

#endif //SANDBOXQT_SANDBOXWORLD_H