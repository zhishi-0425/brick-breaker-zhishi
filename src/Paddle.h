#ifndef PADDLE_H
#define PADDLE_H

#include "raylib.h"

class Ball;  // 前向声明

class Paddle {
private:
    Rectangle rect;
public:
    Paddle(float x, float y, float w, float h);
    void Draw();
    void MoveLeft(float speed);
    void MoveRight(float speed);
    Rectangle GetRect() const { return rect; }   // 新增

    // 新增碰撞检测与处理
    bool CheckCollision(const Ball& ball) const;
    void OnCollision(Ball& ball);
};

#endif