#include "raylib.h"
#include "Ball.h"
#include "Paddle.h"
#include "Brick.h"
#include <vector>

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "打砖块2D - 第三周");

    // 游戏对象
    Ball ball({400, 500}, {0, 0}, 10);
    Paddle paddle(350, 550, 100, 20);
    
    // 游戏状态
    int score = 0;
    int lives = 3;
    bool gameRunning = true;
    
    // 重置球到板上方（未发射）
    ball.ResetToPaddle(paddle.GetRect().x + paddle.GetRect().width/2, paddle.GetRect().y);

    // 创建砖块（8个一排）
    std::vector<Brick> bricks;
    float brickWidth = 80;
    float brickHeight = 30;
    for (int i = 0; i < 8; i++) {
        bricks.emplace_back(50 + i * 95, 100, brickWidth, brickHeight);
    }

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // ---------- 重置游戏（按 R）----------
        if (IsKeyPressed(KEY_R)) {
            score = 0;
            lives = 3;
            gameRunning = true;
            // 重置板位置
            paddle = Paddle(350, 550, 100, 20);
            // 重置球（未发射）
            ball.ResetToPaddle(paddle.GetRect().x + paddle.GetRect().width/2, paddle.GetRect().y);
            // 重置砖块
            bricks.clear();
            for (int i = 0; i < 8; i++) {
                bricks.emplace_back(50 + i * 95, 100, brickWidth, brickHeight);
            }
        }

        // ---------- 发射球（按空格，仅在游戏进行且未发射时）----------
        if (IsKeyPressed(KEY_SPACE) && !ball.IsLaunched() && gameRunning) {
            ball.Launch(paddle.GetRect().x + paddle.GetRect().width/2, paddle.GetRect().y);
        }

        // ---------- 游戏更新（仅在游戏进行且球已发射时）----------
        if (gameRunning && ball.IsLaunched()) {
            // 板移动
            if (IsKeyDown(KEY_LEFT)) paddle.MoveLeft(5);
            if (IsKeyDown(KEY_RIGHT)) paddle.MoveRight(5);

            // 球移动
            ball.Move();
            ball.BounceEdge(screenWidth, screenHeight);

            // 球与板碰撞
            if (paddle.CheckCollision(ball)) {
                paddle.OnCollision(ball);
            }

            // 球与砖块碰撞
            for (auto& brick : bricks) {
                if (brick.IsActive() && ball.CheckBrickCollision(brick.GetRect())) {
                    brick.SetActive(false);
                    score += 10;
                    break;   // 一次只处理一个碰撞
                }
            }

            // 球掉出屏幕（减命、重置）
            if (ball.GetPosition().y + ball.GetRadius() > screenHeight) {
                lives--;
                if (lives <= 0) {
                    gameRunning = false;   // 游戏结束
                } else {
                    // 球复位到板上方，等待发射
                    ball.ResetToPaddle(paddle.GetRect().x + paddle.GetRect().width/2, paddle.GetRect().y);
                }
            }

            // 胜利检测
            bool allBricksDestroyed = true;
            for (auto& brick : bricks) {
                if (brick.IsActive()) {
                    allBricksDestroyed = false;
                    break;
                }
            }
            if (allBricksDestroyed) {
                gameRunning = false;
            }
        }

        // ---------- 绘制（始终执行）----------
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // 绘制墙壁（四边）
        DrawRectangle(0, 0, 5, screenHeight, GRAY);
        DrawRectangle(screenWidth - 5, 0, 5, screenHeight, GRAY);
        DrawRectangle(0, 0, screenWidth, 5, GRAY);
        DrawRectangle(0, screenHeight - 5, screenWidth, 5, GRAY);

        // 绘制游戏元素
        ball.Draw();
        paddle.Draw();
        for (auto& brick : bricks) brick.Draw();

        // 显示分数和生命
        DrawText(TextFormat("Score: %d", score), 10, 10, 20, DARKGRAY);
        DrawText(TextFormat("Lives: %d", lives), 10, 40, 20, DARKGRAY);

        // 提示发射（仅在未发射且游戏运行时）
        if (!ball.IsLaunched() && gameRunning) {
            DrawText("Press SPACE to launch", screenWidth/2 - 120, screenHeight - 30, 20, YELLOW);
        }

        // 游戏结束或胜利信息
        if (!gameRunning) {
            DrawText("GAME OVER", screenWidth/2 - 70, screenHeight/2, 30, RED);
            DrawText("Press R to restart", screenWidth/2 - 100, screenHeight/2 + 40, 20, DARKGRAY);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}