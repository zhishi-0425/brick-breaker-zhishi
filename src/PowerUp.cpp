#include "PowerUp.h"

PowerUp::PowerUp(float x, float y, PowerUpType t)
    : position({x, y}), type(t), active(true), speed(100.0f) {
    // 根据类型设置持续时间
    switch (t) {
        case PowerUpType::PADDLE_EXTEND: duration = 5.0f; break;
        case PowerUpType::MULTI_BALL:    duration = 0.0f; break;
        case PowerUpType::SLOW_BALL:     duration = 5.0f; break;
        default: duration = 0.0f; break;
    }
}

void PowerUp::Update(float dt) {
    position.y += speed * dt;
    // 超出屏幕底部则失效
    if (position.y > GetScreenHeight() + 50) {
        active = false;
    }
}

void PowerUp::Draw() const {
    Color color;
    switch (type) {
        case PowerUpType::PADDLE_EXTEND: color = GREEN; break;
        case PowerUpType::MULTI_BALL:    color = YELLOW; break;
        case PowerUpType::SLOW_BALL:     color = BLUE; break;
        default: color = WHITE; break;
    }
    DrawCircleV(position, 12, color);
    DrawCircleLines(position.x, position.y, 12, DARKGRAY);
}

Rectangle PowerUp::GetRect() const {
    return {position.x - 12, position.y - 12, 24, 24};
}
