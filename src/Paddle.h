#ifndef PADDLE_H
#define PADDLE_H

#include "raylib.h"

class Ball;

class Paddle {
private:
    Rectangle rect;
    float originalWidth;      // 原始宽度
    float currentWidth;       // 当前宽度（可能临时变化）
    float effectRemaining;    // 剩余效果时间（秒）
    float screenWidth;        // 屏幕宽度，用于边界限制

public:
    Paddle(float x, float y, float w, float h);
    void Draw();
    void MoveLeft(float speed);
    void MoveRight(float speed);
    Rectangle GetRect() const { return rect; }
    
    bool CheckCollision(const Ball& ball) const;
    void OnCollision(Ball& ball);
    
    // 道具效果
    void Extend(float extraWidth, float duration);  // 加长板
    void ResetWidth();                              // 恢复原始宽度
    void Update(float dt);                          // 每帧更新，减少剩余时间
};

#endif