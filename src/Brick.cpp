#include "Brick.h"

Brick::Brick(float x, float y, float w, float h, Color c)
    : rect{x, y, w, h}, active(true), color(c) {}

void Brick::Draw() {
    if (active) {
        DrawRectangleRec(rect, color);
        DrawRectangleLinesEx(rect, 1, WHITE);
    }
}