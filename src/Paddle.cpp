#include "Paddle.h"
#include "Ball.h"

Paddle::Paddle(float x, float y, float w, float h) {
    rect = { x, y, w, h };
}

void Paddle::Draw() {
    DrawRectangleRec(rect, BLUE);
}

void Paddle::MoveLeft(float speed) {
    rect.x -= speed;
    if (rect.x < 0) rect.x = 0;
}

void Paddle::MoveRight(float speed) {
    rect.x += speed;
    if (rect.x + rect.width > GetScreenWidth())
        rect.x = GetScreenWidth() - rect.width;
}

bool Paddle::CheckCollision(const Ball& ball) const {
    return CheckCollisionCircleRec(ball.GetPosition(), ball.GetRadius(), rect);
}

void Paddle::OnCollision(Ball& ball) {
    Vector2 speed = ball.GetSpeed();
    speed.y = -speed.y;                           // 垂直反弹
    float offset = ball.GetPosition().x - (rect.x + rect.width / 2);
    speed.x += offset * 0.05f;                   // 水平偏移
    ball.SetSpeed(speed);
}