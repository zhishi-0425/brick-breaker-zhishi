#include "raylib.h"
#include "Ball.h"
#include "Paddle.h"
#include "Brick.h"
#include "GameState.h"
#include <vector>

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "打砖块2D - 第三周");

    Ball ball({400, 500}, {0, 0}, 10);
    Paddle paddle(350, 550, 100, 20);
    GameState gameState;

    ball.ResetToPaddle(paddle.GetRect().x + paddle.GetRect().width/2, paddle.GetRect().y);

    std::vector<Brick> bricks;
    float brickWidth = 80;
    float brickHeight = 30;
    for (int i = 0; i < 8; i++) {
        bricks.emplace_back(50 + i * 95, 100, brickWidth, brickHeight);
    }

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // 重置
        if (IsKeyPressed(KEY_R)) {
            gameState.Reset();
            paddle = Paddle(350, 550, 100, 20);
            ball.ResetToPaddle(paddle.GetRect().x + paddle.GetRect().width/2, paddle.GetRect().y);
            bricks.clear();
            for (int i = 0; i < 8; i++) {
                bricks.emplace_back(50 + i * 95, 100, brickWidth, brickHeight);
            }
        }

        // 发射
        if (IsKeyPressed(KEY_SPACE) && !ball.IsLaunched() && !gameState.IsGameOver()) {
            ball.Launch(paddle.GetRect().x + paddle.GetRect().width/2, paddle.GetRect().y);
        }

        // 更新
        if (!gameState.IsGameOver() && ball.IsLaunched()) {
            if (IsKeyDown(KEY_LEFT)) paddle.MoveLeft(5);
            if (IsKeyDown(KEY_RIGHT)) paddle.MoveRight(5);

            ball.Move();
            ball.BounceEdge(screenWidth, screenHeight);

            if (paddle.CheckCollision(ball)) {
                paddle.OnCollision(ball);
            }

            for (auto& brick : bricks) {
                if (brick.IsActive() && ball.CheckBrickCollision(brick.GetRect())) {
                    brick.SetActive(false);
                    gameState.AddScore(10);
                    break;
                }
            }

            if (ball.GetPosition().y + ball.GetRadius() > screenHeight) {
                gameState.LoseLife();
                if (!gameState.IsGameOver()) {
                    ball.ResetToPaddle(paddle.GetRect().x + paddle.GetRect().width/2, paddle.GetRect().y);
                }
            }

            bool allBricksDestroyed = true;
            for (auto& brick : bricks) {
                if (brick.IsActive()) {
                    allBricksDestroyed = false;
                    break;
                }
            }
            if (allBricksDestroyed && !gameState.IsGameOver()) {
                gameState.SetVictory(true);
                gameState.SetGameOver(true);
            }
        }

        // 绘制
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawRectangle(0, 0, 5, screenHeight, GRAY);
        DrawRectangle(screenWidth - 5, 0, 5, screenHeight, GRAY);
        DrawRectangle(0, 0, screenWidth, 5, GRAY);
        DrawRectangle(0, screenHeight - 5, screenWidth, 5, GRAY);

        ball.Draw();
        paddle.Draw();
        for (auto& brick : bricks) brick.Draw();

        DrawText(TextFormat("Score: %d", gameState.GetScore()), 10, 10, 20, DARKGRAY);
        DrawText(TextFormat("Lives: %d", gameState.GetLives()), 10, 40, 20, DARKGRAY);

        if (!ball.IsLaunched() && !gameState.IsGameOver()) {
            DrawText("Press SPACE to launch", screenWidth/2 - 120, screenHeight - 30, 20, YELLOW);
        }

        if (gameState.IsGameOver()) {
            const char* msg = gameState.IsVictory() ? "YOU WIN!" : "GAME OVER";
            DrawText(msg, screenWidth/2 - MeasureText(msg, 30)/2, screenHeight/2, 30, RED);
            DrawText("Press R to restart", screenWidth/2 - 100, screenHeight/2 + 40, 20, DARKGRAY);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}