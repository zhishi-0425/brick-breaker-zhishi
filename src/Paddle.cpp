#include "Paddle.h"
#include "Ball.h"

Paddle::Paddle(float x, float y, float w, float h)
    : rect{x, y, w, h},
      originalWidth(w),
      currentWidth(w),
      effectRemaining(0.0f),
      screenWidth(800) {}

void Paddle::Draw() {
    DrawRectangleRec(rect, BLUE);
    DrawRectangleLinesEx(rect, 2, SKYBLUE);
}

void Paddle::MoveLeft(float speed) {
    rect.x -= speed;
    if (rect.x < 5) rect.x = 5;
}

void Paddle::MoveRight(float speed) {
    rect.x += speed;
    if (rect.x + rect.width > screenWidth - 5)
        rect.x = screenWidth - rect.width - 5;
}

bool Paddle::CheckCollision(const Ball& ball) const {
    return CheckCollisionCircleRec(ball.GetPosition(), ball.GetRadius(), rect);
}

void Paddle::OnCollision(Ball& ball) {
    Vector2 speed = ball.GetSpeed();
    speed.y = -speed.y;
    float offset = ball.GetPosition().x - (rect.x + rect.width / 2);
    speed.x += offset * 0.05f;
    ball.SetSpeed(speed);
}

// ========== 新增方法 ==========
void Paddle::Extend(float extraWidth, float duration) {
    currentWidth = originalWidth + extraWidth;
    rect.width = currentWidth;
    effectRemaining = duration;
    // 防止超出右边界
    if (rect.x + rect.width > screenWidth - 5)
        rect.x = screenWidth - rect.width - 5;
}

void Paddle::ResetWidth() {
    rect.width = originalWidth;
    currentWidth = originalWidth;
    effectRemaining = 0.0f;
}

void Paddle::Update(float dt) {
    if (effectRemaining > 0) {
        effectRemaining -= dt;
        if (effectRemaining <= 0) {
            ResetWidth();
        }
    }
}