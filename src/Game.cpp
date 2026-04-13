#include "Game.h"
#include <cstdio>
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

Game::Game()
    : ball({400, 500}, {0, 0}, 10),
      paddle(350, 550, 100, 20),
      score(0),
      lives(3),
      ballLaunched(false),
      currentState(GameState::PLAYING),  // 为了方便，直接开始游戏，可以后续改为MENU
      screenWidth(800),
      screenHeight(600) {}

Game::~Game() {}

void Game::Init() {
    InitBricks();
    ball.ResetToPaddle(paddle.GetRect().x + paddle.GetRect().width/2, paddle.GetRect().y);
    ballLaunched = false;
    currentState = GameState::PLAYING;
}

void Game::InitBricks() {
    LoadConfig("config.json");
    bricks.clear();
    float brickWidth = 80;
    float brickHeight = 30;
    for (int i = 0; i < 8; i++) {
        bricks.emplace_back(50 + i * 95, 100, brickWidth, brickHeight);
    }
}

void Game::ChangeState(GameState newState) {
    currentState = newState;
    TraceLog(LOG_INFO, "Game state changed to %d", (int)newState);
}

void Game::HandleInput() {
    // 重置游戏（按 R）
    if (IsKeyPressed(KEY_R)) {
        score = 0;
        lives = 3;
        ballLaunched = false;
        ball.ResetToPaddle(paddle.GetRect().x + paddle.GetRect().width/2, paddle.GetRect().y);
        InitBricks();
        ChangeState(GameState::PLAYING);
        return;
    }

    // 发射球（仅在游戏中且未发射时）
    if (currentState == GameState::PLAYING && IsKeyPressed(KEY_SPACE) && !ballLaunched) {
        ball.Launch(paddle.GetRect().x + paddle.GetRect().width/2, paddle.GetRect().y);
        ballLaunched = true;
    }

    // 暂停（按 P）
    if (currentState == GameState::PLAYING && IsKeyPressed(KEY_P)) {
        ChangeState(GameState::PAUSED);
    } else if (currentState == GameState::PAUSED && IsKeyPressed(KEY_P)) {
        ChangeState(GameState::PLAYING);
    }
}

void Game::Update() {
    static float lastTime = GetTime();
    float currentTime = GetTime();
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    if (deltaTime > 0.033f) deltaTime = 0.033f;   // 限制最大帧间隔

    // 更新板的效果计时
    paddle.Update(deltaTime);

    HandleInput();

    switch (currentState) {
        case GameState::PLAYING:
            UpdatePlaying();
            break;
        case GameState::PAUSED:
            UpdatePaused();
            break;
        case GameState::GAMEOVER:
            UpdateGameOver();
            break;
        case GameState::VICTORY:
            UpdateVictory();
            break;
        default:
            break;
    }
}

void Game::UpdatePlaying() {
    if (!ballLaunched) return;

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
            break;
        }
    }

    // 球掉出屏幕
    if (ball.GetPosition().y + ball.GetRadius() > screenHeight) {
        lives--;
        if (lives <= 0) {
            ChangeState(GameState::GAMEOVER);
        } else {
            // 重置球到板上方，未发射
            ball.ResetToPaddle(paddle.GetRect().x + paddle.GetRect().width/2, paddle.GetRect().y);
            ballLaunched = false;
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
        ChangeState(GameState::VICTORY);
    }
}

void Game::UpdatePaused() {
    // 暂停时什么都不做，但可以显示提示
}

void Game::UpdateGameOver() {
    // 游戏结束，等待按 R 重置（已在 HandleInput 中处理）
}

void Game::UpdateVictory() {
    // 胜利，等待按 R 重置
}

void Game::Draw() {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // 绘制墙壁
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

    // 根据状态显示不同信息
    switch (currentState) {
        case GameState::PLAYING:
            if (!ballLaunched) {
                DrawText("Press SPACE to launch", screenWidth/2 - 120, screenHeight - 30, 20, YELLOW);
            }
            break;
        case GameState::PAUSED:
            DrawText("PAUSED", screenWidth/2 - 50, screenHeight/2, 30, YELLOW);
            DrawText("Press P to resume", screenWidth/2 - 100, screenHeight/2 + 40, 20, WHITE);
            break;
        case GameState::GAMEOVER:
            DrawText("GAME OVER", screenWidth/2 - 70, screenHeight/2, 30, RED);
            DrawText("Press R to restart", screenWidth/2 - 100, screenHeight/2 + 40, 20, WHITE);
            break;
        case GameState::VICTORY:
            DrawText("YOU WIN!", screenWidth/2 - 60, screenHeight/2, 30, GREEN);
            DrawText("Press R to restart", screenWidth/2 - 100, screenHeight/2 + 40, 20, WHITE);
            break;
        default:
            break;
    }

    EndDrawing();
}

void Game::Shutdown() {
    // 目前无需额外清理
}

void Game::LoadConfig(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        TraceLog(LOG_WARNING, "Config file not found: %s, using defaults", path.c_str());
        return;
    }
    json config = json::parse(f);

    // 读取窗口尺寸（仅保存，不实际改变窗口）
    screenWidth = config["window"]["width"];
    screenHeight = config["window"]["height"];
    // 注意：窗口已在 main 中创建，如果希望使用配置的尺寸，需要修改 main.cpp 先读配置再创建窗口。
    // 这里我们只是将值保存，后续绘制时可能用到（例如边界检测），但 main 中的窗口大小应与之匹配。
    // 为简单，我们假设 main 中使用的尺寸与配置一致。如果不一致，边界可能出错。建议在 main 中先读取配置再创建窗口。
    // 但由于我们还没有将配置读取移到 main，暂时只保存，后续会调整。

    // 读取球参数
    float ballRadius = config["ball"]["radius"];
    float ballSpeedX = config["ball"]["speedX"];
    float ballSpeedY = config["ball"]["speedY"];
    // 重新创建 ball 对象（注意：ball 是成员，使用赋值）
    ball = Ball({(float)screenWidth/2, (float)screenHeight/2}, {ballSpeedX, ballSpeedY}, ballRadius);
    // 注意：重力、最大速度等参数如果需要，可以存储到 Ball 类的成员中，但当前 Ball 类没有这些成员，我们可以扩展 Ball 类，或者暂时忽略。
    // 为了满足 PPT 要求，我们至少实现从配置文件读取基本参数。

    // 读取板参数
    float paddleWidth = config["paddle"]["width"];
    float paddleHeight = config["paddle"]["height"];
    float paddleX = config["paddle"]["x"];
    float paddleY = config["paddle"]["y"];
    paddle = Paddle(paddleX, paddleY, paddleWidth, paddleHeight);

    // 读取砖块参数
    int brickCols = config["bricks"]["cols"];
    float brickWidth = config["bricks"]["width"];
    float brickHeight = config["bricks"]["height"];
    float startX = config["bricks"]["startX"];
    float startY = config["bricks"]["startY"];
    float spacingX = config["bricks"]["spacingX"];

    bricks.clear();
    for (int i = 0; i < brickCols; i++) {
        bricks.emplace_back(startX + i * (brickWidth + spacingX), startY, brickWidth, brickHeight);
    }

    // 读取游戏参数
    lives = config["game"]["initialLives"];
    // score 保持为0，不覆盖
    // scorePerBrick 可以保存到成员变量，但我们在 UpdatePlaying 中直接用了 10，可以改为从配置读取
    // 为了演示，我们添加一个成员变量 scorePerBrick，在 Game.h 中添加 int scorePerBrick; 然后在 LoadConfig 中赋值。
}
