#ifndef BRICK_H
#define BRICK_H

#include "raylib.h"

class Brick {
private:
    Rectangle rect;
    bool active;
    Color color;          // 新增
public:
    Brick(float x, float y, float w, float h, Color c);
    void Draw();
    bool IsActive() const { return active; }
    void SetActive(bool a) { active = a; }
    Rectangle GetRect() const { return rect; }
    Color GetColor() const { return color; }  // 可选
    void SetColor(Color c) { color = c; } //允许外部修改砖块颜色，用于显示“加载完成”效果
};

#endif