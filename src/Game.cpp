#include "Game.h"
#include <cstdio>
#include <nlohmann/json.hpp>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>

using json = nlohmann::json;

Game::Game()
    : paddle(350, 550, 100, 20),
      score(0),
      lives(3),
      ballLaunched(false),
      currentState(GameState::PLAYING),
      screenWidth(800),
      screenHeight(600),
      deltaTime(0.0f),
      slowRemaining(0.0f),
      slowFactor(0.7f) {
    srand(time(nullptr));
}

Game::~Game() {}

void Game::Init() {
    balls.clear();
    // 创建一个球，速度为零，并放到板上方
    balls.emplace_back(Vector2{0, 0}, Vector2{0, 0}, 10);
    ballLaunched = false;
    paddle = Paddle(350, 550, 100, 20);
    // 将球放到板的正上方
    balls[0].ResetToPaddle(paddle.GetRect().x + paddle.GetRect().width/2, paddle.GetRect().y);
    InitBricks();
    currentState = GameState::PLAYING;
    powerUps.clear();
    particles.clear();
    slowRemaining = 0.0f;
    slowFactor = 0.7f;
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
    if (IsKeyPressed(KEY_R)) {
        score = 0;
        lives = 3;
        balls.clear();
        balls.emplace_back(Vector2{0, 0}, Vector2{0, 0}, 10);
        ballLaunched = false;
        paddle = Paddle(350, 550, 100, 20);
        balls[0].ResetToPaddle(paddle.GetRect().x + paddle.GetRect().width/2, paddle.GetRect().y);
        InitBricks();
        currentState = GameState::PLAYING;
        powerUps.clear();
        particles.clear();
        slowRemaining = 0.0f;
        return;
    }

    if (currentState == GameState::PLAYING && IsKeyPressed(KEY_SPACE) && !ballLaunched) {
        if (!balls.empty()) {
            balls[0].Launch(paddle.GetRect().x + paddle.GetRect().width/2, paddle.GetRect().y);
            ballLaunched = true;
        }
    }

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
    if (deltaTime > 0.033f) deltaTime = 0.033f;

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

    if (IsKeyDown(KEY_LEFT)) paddle.MoveLeft(5);
    if (IsKeyDown(KEY_RIGHT)) paddle.MoveRight(5);

    for (auto& ball : balls) {
        ball.Move();
        ball.BounceEdge(screenWidth, screenHeight);
    }

    for (auto& ball : balls) {
        if (paddle.CheckCollision(ball)) {
            paddle.OnCollision(ball);
        }
    }

    for (auto& brick : bricks) {
        if (!brick.IsActive()) continue;
        bool hit = false;
        for (auto& ball : balls) {
            if (ball.CheckBrickCollision(brick.GetRect())) {
                hit = true;
                break;
            }
        }
        if (hit) {
            brick.SetActive(false);
            score += 10;

            // 粒子特效
            for (int i = 0; i < 10; i++) {
                Particle p;
                p.position = { brick.GetRect().x + rand() % (int)brick.GetRect().width,
                               brick.GetRect().y + rand() % (int)brick.GetRect().height };
                p.velocity = { (rand()%100 - 50)/10.0f, (rand()%100 - 50)/10.0f };
                p.color = GREEN;
                p.life = 0.5f;
                particles.push_back(p);
            }

            // 30% 概率生成道具
            if ((float)(rand() % 100) / 100.0f < powerUpDropRate) {
                PowerUpType type = static_cast<PowerUpType>(rand() % 3);
                float x = brick.GetRect().x + brick.GetRect().width / 2;
                float y = brick.GetRect().y;
                powerUps.emplace_back(x, y, type);
            }
        }
    }

    // 球掉出屏幕
    bool anyBallActive = false;
    for (auto it = balls.begin(); it != balls.end(); ) {
        if (it->GetPosition().y + it->GetRadius() > screenHeight) {
            it = balls.erase(it);
        } else {
            anyBallActive = true;
            ++it;
        }
    }
    if (!anyBallActive) {
        lives--;
        if (lives <= 0) {
            ChangeState(GameState::GAMEOVER);
        } else {
            balls.clear();
            balls.emplace_back(Vector2{0, 0}, Vector2{0, 0}, 10);
            balls[0].ResetToPaddle(paddle.GetRect().x + paddle.GetRect().width/2, paddle.GetRect().y);
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

    UpdatePowerUps(deltaTime);
    CheckPowerUpCollision();
    UpdateParticles(deltaTime);

    if (slowRemaining > 0) {
    slowRemaining -= deltaTime;
        if (slowRemaining <= 0) {
            // 恢复速度（需要保存原始速度，简化起见不做恢复）
        }
    }
}

void Game::UpdatePaused() {}
void Game::UpdateGameOver() {}
void Game::UpdateVictory() {}

void Game::UpdatePowerUps(float dt) {
    for (auto& p : powerUps) {
        p.Update(dt);
    }
    powerUps.erase(std::remove_if(powerUps.begin(), powerUps.end(),
                  [](const PowerUp& p) { return !p.active; }),
                  powerUps.end());
}

void Game::CheckPowerUpCollision() {
    for (auto& p : powerUps) {
        if (!p.active) continue;
        if (CheckCollisionCircleRec(p.position, 12, paddle.GetRect())) {
            switch (p.type) {
                case PowerUpType::PADDLE_EXTEND:
                    paddle.Extend(paddleExtendWidth, paddleExtendDuration);
                    break;
                case PowerUpType::MULTI_BALL: {
                    // 找出所有运动中的球（速度大小 > 0.5）
                    std::vector<Vector2> movingSpeeds;
                    for (const auto& b : balls) {
                        Vector2 spd = b.GetSpeed();
                        float mag = sqrt(spd.x*spd.x + spd.y*spd.y);
                        if (mag > 0.5f) {
                            movingSpeeds.push_back(spd);
                        }
                    }
                    // 如果没有运动球，则使用默认速度
                    if (movingSpeeds.empty()) {
                        movingSpeeds.push_back({5, -5});
                    }
                    // 为每个运动球生成一个新球（生成在板的上方）
                    std::vector<Ball> newBalls;
                    for (const auto& spd : movingSpeeds) {
                        float x = paddle.GetRect().x + paddle.GetRect().width / 2;
                        float y = paddle.GetRect().y - 15;  // 板的上方
                        float angle = (rand() % 60 - 30) * 3.14159f / 180.0f;
                        float cosA = cos(angle);
                        float sinA = sin(angle);
                        Vector2 newSpd = { spd.x * cosA - spd.y * sinA,
                                           spd.x * sinA + spd.y * cosA };
                        // 确保新球速度不太小
                        if (fabs(newSpd.x) < 1.0f) newSpd.x = (rand() % 2 == 0 ? 4 : -4);
                        if (fabs(newSpd.y) < 1.0f) newSpd.y = (rand() % 2 == 0 ? 4 : -4);
                        Ball newBall(Vector2{x, y}, newSpd, 10);
                        newBall.Activate();   // 关键：设置为已发射状态
                        newBalls.push_back(newBall);
                    }
                    balls.insert(balls.end(), newBalls.begin(), newBalls.end());
                    break;
                }
                case PowerUpType::SLOW_BALL:
                    slowRemaining = slowDuration;
                    slowFactor = slowFactorValue;
                    for (auto& b : balls) {
                        Vector2 spd = b.GetSpeed();
                        b.SetSpeed({spd.x * slowFactor, spd.y * slowFactor});
                    }
                    break;
            }
            p.active = false;
        }
    }
}


void Game::UpdateParticles(float dt) {
    for (auto& p : particles) {
        p.position.x += p.velocity.x * dt;
        p.position.y += p.velocity.y * dt;
        p.life -= dt;
    }
    particles.erase(std::remove_if(particles.begin(), particles.end(),
                  [](const Particle& p) { return p.life <= 0; }),
                  particles.end());
}

void Game::DrawParticles() {
    for (auto& p : particles) {
        DrawCircleV(p.position, 2, p.color);
    }
}

void Game::Draw() {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    DrawRectangle(0, 0, 5, screenHeight, GRAY);
    DrawRectangle(screenWidth - 5, 0, 5, screenHeight, GRAY);
    DrawRectangle(0, 0, screenWidth, 5, GRAY);
    DrawRectangle(0, screenHeight - 5, screenWidth, 5, GRAY);

    for (auto& ball : balls) ball.Draw();
    paddle.Draw();
    for (auto& brick : bricks) brick.Draw();
    for (auto& p : powerUps) p.Draw();
    DrawParticles();

    DrawText(TextFormat("Score: %d", score), 10, 10, 20, DARKGRAY);
    DrawText(TextFormat("Lives: %d", lives), 10, 40, 20, DARKGRAY);

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

void Game::Shutdown() {}

void Game::LoadConfig(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        TraceLog(LOG_WARNING, "Config file not found: %s, using defaults", path.c_str());
        return;
    }
    json config = json::parse(f);

    screenWidth = config["window"]["width"];
    screenHeight = config["window"]["height"];

    float paddleWidth = config["paddle"]["width"];
    float paddleHeight = config["paddle"]["height"];
    float paddleX = config["paddle"]["x"];
    float paddleY = config["paddle"]["y"];
    paddle = Paddle(paddleX, paddleY, paddleWidth, paddleHeight);

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

    lives = config["game"]["initialLives"];

        // 读取道具参数
    if (config.contains("powerups")) {
        auto& p = config["powerups"];
        powerUpDropRate = p["paddle_extend"]["drop_rate"];
        paddleExtendWidth = p["paddle_extend"]["extra_width"];
        paddleExtendDuration = p["paddle_extend"]["duration"];
        slowFactorValue = p["slow_ball"]["speed_factor"];
        slowDuration = p["slow_ball"]["duration"];
        // 多球道具没有额外参数，掉落率可共用
    }

}