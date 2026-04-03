#include "raylib.h"
#include "Game.h"

int main() {


    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "打砖块 - 第四周重构");

    Game game;
    game.Init();

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        game.Update();
        game.Draw();
    }

    game.Shutdown();
    CloseWindow();
    return 0;
}