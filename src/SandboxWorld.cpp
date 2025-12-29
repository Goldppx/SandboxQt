#include "SandboxWorld.h"
#include <algorithm>
#include <cmath>
#include <ctime>

SandboxWorld::SandboxWorld(int w, int h) : width(w), height(h) {
    grid.resize(width * height);
    buffer.resize(width * height);

    // 初始化为空气
    for (int i = 0; i < width * height; ++i) {
        grid[i] = {ParticleType::Air, 0, false, 0, 0xFFFFFFFF};
        buffer[i] = 0xFFFFFFFF;
    }

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &SandboxWorld::step);
}

void SandboxWorld::clearWorld() {
    for (int i = 0; i < width * height; ++i) {
        grid[i] = {ParticleType::Air,0,false,0,0xFFFFFFFF};
        buffer[i] = 0xFFFFFFFF;
    }
    emit worldChanged();
}

void SandboxWorld::setParticle(int x, int y, ParticleType type) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        const int idx = y * width + x;
        const auto variation = static_cast<int8_t>((fast_rand() % 25) - 12);

        grid[idx].type = type;
        grid[idx].pressure = 0;
        grid[idx].updated = false;
        grid[idx].colorVariation = variation;
        grid[idx].color = calculateColor(grid[idx]);

        buffer[idx] = grid[idx].color;
    }
}

uint32_t SandboxWorld::calculateColor(const Particle &p) {
    // 快速处理空气
    if (p.type == ParticleType::Air) return 0xFFFFFFFF;

    const int var = static_cast<unsigned char>(p.colorVariation);
    const int pres = p.pressure;

    switch (p.type) {
        case ParticleType::Sand: {
            // 预乘常数，减少计算开销
            const int r = std::clamp(std::max(120, 255 - pres * 6) + var, 0, 255);
            const int g = std::clamp(std::max(90, 204 - pres * 6) + var, 0, 255);
            const int b = std::clamp(var >> 1, 0, 255); // 用位移代替除以2
            return (0xFF000000) | (r << 16) | (g << 8) | b;
        }
        case ParticleType::Water: {
            const int r = std::clamp(std::max(10, 50 - pres * 3) + (var >> 1), 0, 255);
            const int g = std::clamp(std::max(30, 140 - pres * 6) + (var >> 1), 0, 255);
            const int b = std::clamp(std::max(100, 255 - pres * 8) + (var >> 1), 0, 255);
            return (0xFF000000) | (r << 16) | (g << 8) | b;
        }
        case ParticleType::Grass:
            return (0xFF000000) | (std::clamp(34 + var, 0, 255) << 16) |
                                 (std::clamp(139 + var, 0, 255) << 8) |
                                  std::clamp(34 + (var >> 1), 0, 255);
        case ParticleType::Wall:
            return (0xFF000000) | (std::clamp(140 + var, 0, 255) << 16) |
                                 (std::clamp(54 + var, 0, 255) << 8) |
                                  std::clamp(54 + var, 0, 255);
        default: return 0xFFFFFFFF;
    }
}

void SandboxWorld::update() {
    if (isPaused) return;

    // 1. 重置更新状态
    for (auto &p : grid) p.updated = false;

    // 2. 自底向上扫描 (Bottom-up scan)
    for (int y = height - 1; y >= 0; y--) {
        // 使用快速随机数决定左右扫描方向
        bool leftToRight = (fast_rand() & 1);
        for (int i = 0; i < width; i++) {
            const int x = leftToRight ? i : (width - 1 - i);
            const int currIdx = y * width + x;
            Particle &curr = grid[currIdx];

            // 忽略空气、墙和本帧已更新过的粒子
            if (curr.updated || curr.type == ParticleType::Air || curr.type == ParticleType::Wall) {
                continue;
            }

            // 3. 压力传递优化 (O(1) Pressure Propagation)
            if (curr.type == ParticleType::Sand || curr.type == ParticleType::Water) {
                const int prevPressure = curr.pressure;
                if (y > 0) {
                    const Particle &above = grid[currIdx - width];
                    curr.pressure = (above.type == curr.type) ? std::min(20, above.pressure + 1) : 0;
                } else {
                    curr.pressure = 0;
                }

                // 压力变化时同步更新颜色和渲染缓冲
                if (prevPressure != curr.pressure) {
                    curr.color = calculateColor(curr);
                    buffer[currIdx] = curr.color;
                }
            }

            const int downIdx = currIdx + width;
            const bool canGoDown = (y < height - 1);

            // 4. 核心物理逻辑
            switch (curr.type) {
                case ParticleType::Sand: {
                    // 下落逻辑 (Gravity)
                    if (canGoDown && (grid[downIdx].type == ParticleType::Air || grid[downIdx].type == ParticleType::Water)) {
                        std::swap(grid[currIdx], grid[downIdx]);
                        grid[downIdx].updated = true;
                        buffer[currIdx] = grid[currIdx].color;
                        buffer[downIdx] = grid[downIdx].color;
                    }
                    // 滑动逻辑 (Sliding)
                    else {
                        const int dir = (fast_rand() & 1) ? 1 : -1;
                        const int sideDown = downIdx + dir;
                        if (canGoDown && (x + dir >= 0 && x + dir < width) &&
                            (grid[sideDown].type == ParticleType::Air || grid[sideDown].type == ParticleType::Water)) {
                            std::swap(grid[currIdx], grid[sideDown]);
                            grid[sideDown].updated = true;
                            buffer[currIdx] = grid[currIdx].color;
                            buffer[sideDown] = grid[sideDown].color;
                        }
                        else if (y > 0 && grid[currIdx - width].type == ParticleType::Air && (fast_rand() % 1000 < 1)) {
                            bool foundWater = false;
                            for (int dy = -1; dy <= 1 && !foundWater; dy++) {
                                for (int dx = -1; dx <= 1; dx++) {
                                    const int nx = x + dx, ny = y + dy;
                                    if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                                        if (grid[ny * width + nx].type == ParticleType::Water) {
                                            foundWater = true; break;
                                        }
                                    }
                                }
                            }
                            if (foundWater) {
                                int upIdx = currIdx - width;
                                grid[upIdx].type = ParticleType::Grass;
                                grid[upIdx].colorVariation = static_cast<int8_t>((fast_rand() % 25) - 12);
                                grid[upIdx].updated = true;
                                grid[upIdx].color = calculateColor(grid[upIdx]);
                                buffer[upIdx] = grid[upIdx].color;
                            }
                        }
                    }
                    break;
                }

                case ParticleType::Water: {
                    // 1. 下落
                    if (canGoDown && grid[downIdx].type == ParticleType::Air) {
                        std::swap(grid[currIdx], grid[downIdx]);
                        grid[downIdx].updated = true;
                        buffer[currIdx] = grid[currIdx].color;
                        buffer[downIdx] = grid[downIdx].color;
                    }
                    // 2. 斜下平滑
                    else {
                        int dir = (fast_rand() & 1) ? 1 : -1;
                        const int sideDown = downIdx + dir;
                        if (canGoDown && (x + dir >= 0 && x + dir < width) && grid[sideDown].type == ParticleType::Air) {
                            std::swap(grid[currIdx], grid[sideDown]);
                            grid[sideDown].updated = true;
                            buffer[currIdx] = grid[currIdx].color;
                            buffer[sideDown] = grid[sideDown].color;
                        }
                        // 3. 穿透式水平探测 (Leveling Logic)
                        else {
                            const int dispersionRate = 8 + curr.pressure;
                            // 尝试向两个方向快速寻找空位
                            for (const int currentDir : {dir, -dir}) {
                                int bestX = -1;
                                for (int r = 1; r <= dispersionRate; ++r) {
                                    const int tx = x + (currentDir * r);
                                    if (tx < 0 || tx >= width) break;
                                    const int tidx = y * width + tx;

                                    if (grid[tidx].type == ParticleType::Air) {
                                        bestX = tx; break;
                                    }
                                    // 只有遇到固体（墙/沙）才彻底阻断探测
                                    if (grid[tidx].type == ParticleType::Wall || grid[tidx].type == ParticleType::Sand) break;
                                }
                                if (bestX != -1) {
                                    const int targetIdx = y * width + bestX;
                                    std::swap(grid[currIdx], grid[targetIdx]);
                                    grid[targetIdx].updated = true;
                                    buffer[currIdx] = grid[currIdx].color;
                                    buffer[targetIdx] = grid[targetIdx].color;
                                }
                            }
                        }
                    }
                    break;
                }

                case ParticleType::Grass: {
                    // 草地繁殖
                    if ((fast_rand() % 100) < 1) {
                        int count = 0;
                        constexpr int radius = 5;
                        // 局部密度检查
                        for (int dy = -radius; dy <= radius; dy++) {
                            for (int dx = -radius; dx <= radius; dx++) {
                                const int nx = x + dx;
                                if (const  int ny = y + dy; nx >= 0 && nx < width && ny >= 0 && ny < height) {
                                    if (grid[ny * width + nx].type == ParticleType::Grass) count++;
                                }
                            }
                        }
                        if (count < 20) {
                            int upIdx = currIdx - width;
                            if (upIdx >= 0 && grid[upIdx].type == ParticleType::Air) {
                                grid[upIdx].type = ParticleType::Grass;
                                grid[upIdx].colorVariation = static_cast<int8_t>((fast_rand() % 25) - 12);
                                grid[upIdx].updated = true;
                                grid[upIdx].color = calculateColor(grid[upIdx]);
                                buffer[upIdx] = grid[upIdx].color;
                            }
                        }
                    }
                    break;
                }
                default: break;
            }
        }
    }
}

void SandboxWorld::step() {
    if (!fpsTimer.isValid()) fpsTimer.start();
    update();
    update();
    frameCount++;
    if (fpsTimer.elapsed() >= 1000) {
        currentFPS = frameCount;
        frameCount = 0;
        fpsTimer.restart();
    }
    emit worldChanged();
}
