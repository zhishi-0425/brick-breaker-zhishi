#ifndef BALL_H
#define BALL_H

#include "raylib.h"

class Ball {
private:
    Vector2 position;
    Vector2 speed;
    float radius;
    bool launched;               // 新增：是否已发射
public:
    Ball(Vector2 pos, Vector2 sp, float r);
    void Move();
    void Draw();
    void BounceEdge(int screenWidth, int screenHeight);

    // 新增：球与砖块碰撞检测
    bool CheckBrickCollision(Rectangle brickRect);

    // 新增方法
    void Launch(float paddleX, float paddleY);      // 发射
    void ResetToPaddle(float paddleX, float paddleY); // 复位到板上方
    bool IsLaunched() const { return launched; }    // 获取发射状态
    
    
    // 新增 getter/setter
    Vector2 GetPosition() const { return position; }
    float GetRadius() const { return radius; }
    Vector2 GetSpeed() const { return speed; }
    void SetSpeed(Vector2 sp) { speed = sp; }
};

#endif