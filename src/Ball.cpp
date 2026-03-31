#include "Ball.h"
#include <algorithm>

Ball::Ball(Vector2 pos, Vector2 sp, float r) {
    position = pos;
    speed = sp;
    radius = r;
    launched = false;   // 初始未发射
}

void Ball::Move() {
    if (launched) {
        position.x += speed.x;
        position.y += speed.y;
    }
}

void Ball::Draw() {
    DrawCircleV(position, radius, RED);
}

// void Ball::BounceEdge(int screenWidth, int screenHeight) {
//     // 左右边界
//     if (position.x - radius <= 0 || position.x + radius >= screenWidth) {
//         speed.x *= -1;
//     }
//     // 上边界
//     if (position.y - radius <= 0) {
//         speed.y *= -1;
//     }
//     // 下边界暂时不处理
// }

void Ball::BounceEdge(int screenWidth, int screenHeight) {
    // 左右边界
    if (position.x - radius <= 0 || position.x + radius >= screenWidth) {
        speed.x *= -1;
    }
    // 上边界
    if (position.y - radius <= 0) {
        speed.y *= -1;
    }
    
}

bool Ball::CheckBrickCollision(Rectangle brickRect) {
    // 1. 检测是否碰撞（使用 Raylib 函数）
    if (!CheckCollisionCircleRec(position, radius, brickRect))
        return false;

    // 2. 计算重叠区域，决定反弹方向
    float overlapLeft   = position.x + radius - brickRect.x;
    float overlapRight  = brickRect.x + brickRect.width - (position.x - radius);
    float overlapTop    = position.y + radius - brickRect.y;
    float overlapBottom = brickRect.y + brickRect.height - (position.y - radius);

    float minOverlapX = std::min(overlapLeft, overlapRight);
    float minOverlapY = std::min(overlapTop, overlapBottom);

    if (minOverlapX < minOverlapY) {
        // 水平方向碰撞
        speed.x = -speed.x;
    } else {
        // 垂直方向碰撞
        speed.y = -speed.y;
    }

    return true;
}

void Ball::Launch(float paddleX, float paddleY) {
    if (launched) return;
    // 将球放在板中央上方
    position.x = paddleX;
    position.y = paddleY - radius - 5;
    speed = {2, -2};   // 固定初速度，可调整
    launched = true;
}

void Ball::ResetToPaddle(float paddleX, float paddleY) {
    position.x = paddleX;
    position.y = paddleY - radius - 5;
    speed = {0, 0};
    launched = false;
}