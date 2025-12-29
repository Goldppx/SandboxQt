#include <iostream>
#include <QApplication>
#include <QPainter>
#include "SandboxInput.h"
#include "SandboxWorld.h"
#include "SandboxRenderer.h"

int main(int argc,char** argv) {
    QApplication app(argc, argv);
    constexpr int worldW = 300;
    constexpr int worldH = 200;
    SandboxWorld world(worldW,worldH);
    SandboxInput input;
    input.setFixedSize(1200,800);
    input.setWorld(&world);
    input.show();
    QTimer gameLoop;
    QObject::connect(&gameLoop,&QTimer::timeout,[&] {
        world.step();
    });
    gameLoop.start(16);
    return QApplication::exec();
}
