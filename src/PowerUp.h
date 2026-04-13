#ifndef POWERUP_H
#define POWERUP_H

#include "raylib.h"

// 道具类型枚举
enum class PowerUpType {
    PADDLE_EXTEND,   // 加长板
    MULTI_BALL,      // 多球
    SLOW_BALL        // 减速球
};

// 道具类
class PowerUp {
public:
    Vector2 position;
    PowerUpType type;
    bool active;
    float duration;   // 效果持续时间（秒），0表示永久
    float speed;      // 下落速度（像素/秒）

    PowerUp(float x, float y, PowerUpType t);
    void Update(float dt);
    void Draw() const;
    Rectangle GetRect() const;
};

#endif
