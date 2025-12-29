//SandBoxInput.h

#ifndef SANDBOXQT_SANDBOXINPUT_H
#define SANDBOXQT_SANDBOXINPUT_H
#pragma once
#include <QWidget>
#include <QMouseEvent>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include "SandboxWorld.h"
class SandboxWorld;

class Brush {
public:
    Brush& operator++() {
        if (size < maxSize) {
            size++;
        }
        return *this;
    }
    Brush operator++(int) {
        const Brush temp = *this;
        ++(*this);
        return temp;
    }
    Brush& operator--() {
        if (size > minSize) {
            size--;
        }
        return *this;
    }
    Brush operator--(int) {
        const Brush temp = *this;
        --(*this);
        return temp;
    }
    void nextType() {
        switch (type) {
            case ParticleType::Sand:  type = ParticleType::Water; break;
            case ParticleType::Water: type = ParticleType::Grass; break;
            case ParticleType::Grass: type = ParticleType::Wall;  break;
            case ParticleType::Wall:  type = ParticleType::Air;  break;
            case ParticleType::Air:  type = ParticleType::Sand;  break; // NOLINT(*-branch-clone)
            default: type = ParticleType::Sand; break;
        }
    }
    [[nodiscard]] int getSize() const { return size; }
    [[nodiscard]] ParticleType getType() const { return type; }
    void setType(const ParticleType t) {
        this->type = t;
    }
private:
    int size = 10;
    const int maxSize = 50;
    const int minSize = 1;
    ParticleType type = ParticleType::Sand;
};

class SandboxInput final : public QWidget{
    Q_OBJECT
    public:
    Brush brush;
    explicit SandboxInput(QWidget *parent = nullptr);
    void setWorld(SandboxWorld *worldPtr);
    private:
    SandboxWorld* world = nullptr;
    QPoint gridPos;
    QPoint mousePos;
    protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
};


#endif //SANDBOXQT_SANDBOXINPUT_H