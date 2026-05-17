#include "raylib.h"
#include "Game.h"
#include <cstring>

int main(int argc, char* argv[]) {
    bool asHost = false;
    const char* serverIp = nullptr;

    if (argc >= 2 && strcmp(argv[1], "--host") == 0) {
        asHost = true;
    } else if (argc >= 3 && strcmp(argv[1], "--client") == 0) {
        serverIp = argv[2];
    } else {
        // 默认以主机模式运行单机游戏（保证物理更新）
        asHost = true;
        printf("Running as single-player (host mode).\n");
        printf("For network: %s --host  OR  %s --client <server_ip>\n", argv[0], argv[0]);
    }

    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Breakout - Async Loading");
    SetTargetFPS(60);

    Game game;
    game.Init(asHost, serverIp);

    while (!WindowShouldClose()) {
        game.Update();
        game.Draw();
    }

    CloseWindow();
    return 0;
}