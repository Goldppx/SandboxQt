//SandboxInput.cpp
#include "SandboxWorld.h"
#include "SandboxInput.h"
#include "SandboxRenderer.h"

SandboxInput::SandboxInput(QWidget *parent) : QWidget(parent) {}
void SandboxInput::setWorld(SandboxWorld *worldPtr) {
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setAttribute(Qt::WA_TranslucentBackground);
    world = worldPtr;
    connect(world,&SandboxWorld::worldChanged,this,[this] {
        this->update();
    });
}
QMenu* brushMenu;

void SandboxInput::keyPressEvent(QKeyEvent *event) {
    if (world) {
        if (event->key() == Qt::Key_C) {
            world->clearWorld();
        }
        if (event->key() == Qt::Key_P || event->key() == Qt::Key_Space) {
            world->setPaused(!world->getPaused());
        }
        if (event->key() == Qt::Key_B) {
            brush.nextType();
        }
        if (event->key() == Qt::Key_Plus || event->key() == Qt::Key_Equal) {
            ++brush;
        }
        if (event->key() == Qt::Key_Minus) {
            --brush;
        }
    }
}
void SandboxInput::mouseMoveEvent(QMouseEvent *event) {
    if (!world) return;
    mousePos = event->pos();
    // 无论是否按下按键，都先更新当前网格坐标，供渲染器绘制预览圈
    gridPos.setX(static_cast<int>(event->position().x() * world->width / width()));
    gridPos.setY(static_cast<int>(event->position().y() * world->height / height()));

    if (event->buttons() & Qt::LeftButton || event->buttons() & Qt::RightButton) {
        // 1. 使用你重构后的 brush.getSize()，而不是写死的 50 分之一
        int brushSize = brush.getSize();
        int radiusSq = brushSize * brushSize; // 预计算半径平方

        for (int i = gridPos.x() - brushSize; i <= gridPos.x() + brushSize; i++) {
            // 2. 修正：将你代码里的 gridY 改为 gridPos.y()
            for (int j = gridPos.y() - brushSize; j <= gridPos.y() + brushSize; j++) {

                if (i >= 0 && i < world->width && j >= 0 && j < world->height) {

                    // 3. 核心修改：增加圆形判定
                    int dx = i - gridPos.x();
                    int dy = j - gridPos.y();

                    if (dx * dx + dy * dy <= radiusSq) {
                        if ((std::rand() % 100) < 30) {
                            world->setParticle(i, j, brush.getType());
                        }
                    }
                }
            }
        }
    }
    // 4. 记得更新界面，否则预览圈会延迟或不显示
    update();
}

void SandboxInput::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.fillRect(this->rect(), Qt::transparent);
    //painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    if (world) {
        SandboxRenderer renderer;
        renderer.render(painter, *world, this->rect(), brush, mousePos);
    }
}

void SandboxInput::contextMenuEvent(QContextMenuEvent *event) {
    QMenu menu(this);
    QMenu* brushMenu = menu.addMenu("Brush...");

    QAction* setAir = brushMenu->addAction("Air");
    QAction* setSand = brushMenu->addAction("Sand");
    QAction* setWater = brushMenu->addAction("Water");
    QAction* setGrass = brushMenu->addAction("Grass");
    QAction* setWall = brushMenu->addAction("Wall");

    menu.addSeparator();

    QAction* clearAction = menu.addAction("Clear World");
    QAction* pauseAction = menu.addAction(world->getPaused() ? "Resume" : "Pause");

    connect(setAir, &QAction::triggered,this,[this](){ brush.setType(ParticleType::Air); });
    connect(setSand, &QAction::triggered, this, [this](){ brush.setType(ParticleType::Sand); });
    connect(setWater, &QAction::triggered, this, [this](){ brush.setType(ParticleType::Water); });
    connect(setGrass, &QAction::triggered, this, [this](){ brush.setType(ParticleType::Grass); });
    connect(setWall, &QAction::triggered, this, [this](){ brush.setType(ParticleType::Wall); });

    connect(clearAction, &QAction::triggered, this, [this](){ world->clearWorld(); });

    connect(pauseAction, &QAction::triggered, this, [this](){ world->setPaused(!world->getPaused()); });

    menu.exec(event->globalPos());
}