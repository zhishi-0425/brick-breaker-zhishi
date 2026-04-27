#include "Game.h"
#include <cstdio>
#include <nlohmann/json.hpp>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <thread>

using json = nlohmann::json;

// 模拟耗时加载任务（例如加载纹理）
void SimulateLoading() {
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

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
      slowFactor(0.7f),
      powerUpDropRate(0.3f),
      paddleExtendWidth(40.0f),
      paddleExtendDuration(5.0f),
      slowFactorValue(0.7f),
      slowDuration(5.0f),
      brickRows(5),
      brickCols(8),
      brickWidth(70.0f),
      brickHeight(25.0f),
      brickStartX(-1.0f),
      brickStartY(80.0f),
      brickSpacingX(10.0f),
      brickSpacingY(5.0f),
      isHost(false),
      isClient(false),
      opponentPaddleX(350.0f),
      lastStateTime(0.0),
      currentStateTime(0.0),
      interpBallX(400.0f),
      interpBallY(300.0f),
      interpOpponentPaddleX(350.0f),
      loadState(LoadState::IDLE), 
      loadCompleted(false),
      firstStateReceived(false) {
    srand(time(nullptr));
}

Game::~Game() {}

void Game::Init(bool asHost, const char* serverIp) {
    net.Init();
    isHost = asHost;
    isClient = !asHost;

    if (isHost) {
        net.CreateHost(12345);
        net.SetOnReceiveCallback([this](const unsigned char* data, size_t len) {
            if (len == sizeof(PaddleUpdateMessage)) {
                PaddleUpdateMessage* msg = (PaddleUpdateMessage*)data;
                opponentPaddleX = msg->paddleX;
            }
        });
        // 主机完整初始化
        LoadConfig("config.json");
        balls.clear();
        float startX = paddle.GetRect().x + paddle.GetRect().width / 2;
        float startY = paddle.GetRect().y - 15;   // 板顶部上方15像素
        balls.emplace_back(Vector2{startX, startY}, Vector2{0, 0}, 10);
        ballLaunched = false;
        paddle = Paddle(350, 550, 100, 20);
        InitBricks();
        powerUps.clear();
        particles.clear();
        slowRemaining = 0.0f;
        originalSpeeds.clear();
        currentState = GameState::PLAYING;
    } else {
        net.ConnectToHost(serverIp ? serverIp : "127.0.0.1", 12345);
        net.SetOnReceiveCallback([this](const unsigned char* data, size_t len) {
            if (len == sizeof(GameStateMessage)) {
                GameStateMessage* msg = (GameStateMessage*)data;
                if (firstStateReceived) {
                    lastGameState = currentGameState;
                    lastStateTime = currentStateTime;
                }
                currentGameState = *msg;
                currentStateTime = msg->timestamp;
                firstStateReceived = true;

                // 完全替换客户端数据
                // 球
                balls.clear();
                for (int i = 0; i < currentGameState.ballCount && i < MAX_BALLS; ++i) {
                    Vector2 pos{ currentGameState.ballsX[i], currentGameState.ballsY[i] };
                    Vector2 spd{ currentGameState.ballsSpeedX[i], currentGameState.ballsSpeedY[i] };
                    balls.emplace_back(pos, spd, 10);
                    balls.back().Activate();
                }
                // 分数和生命
                score = currentGameState.scoreLeft;
                lives = currentGameState.livesLeft;
                opponentPaddleX = currentGameState.paddleRightX;
                // 砖块
                int idx = 0;
                for (int row = 0; row < brickRows; ++row) {
                    for (int col = 0; col < brickCols; ++col) {
                        int brickIndex = row * brickCols + col;
                        if (brickIndex < (int)bricks.size()) {
                            bricks[brickIndex].SetActive(currentGameState.bricksActive[idx]);
                        }
                        ++idx;
                    }
                }
                // 道具
                powerUps.clear();
                for (int i = 0; i < currentGameState.powerUpCount && i < MAX_POWERUPS; ++i) {
                    PowerUpType type = (PowerUpType)currentGameState.powerUpType[i];
                    PowerUp p(currentGameState.powerUpPosX[i], currentGameState.powerUpPosY[i], type);
                    p.duration = currentGameState.powerUpDuration[i];
                    powerUps.push_back(p);
                }
                // 游戏状态
                if (currentGameState.gameRunning) {
                    if (currentState != GameState::PLAYING) currentState = GameState::PLAYING;
                } else {
                    if (currentState == GameState::PLAYING) currentState = GameState::GAMEOVER;
                }
            }
        });
        // 客户端初始化本地资源
        LoadConfig("config.json");
        balls.clear();
        float startX = paddle.GetRect().x + paddle.GetRect().width / 2;
        float startY = paddle.GetRect().y - 15;   // 板顶部上方15像素
        balls.emplace_back(Vector2{startX, startY}, Vector2{0, 0}, 10);
        ballLaunched = false;
        paddle = Paddle(350, 550, 100, 20);
        InitBricks();          // 必须调用，使 bricks 有正确的数量
        currentState = GameState::PLAYING;
        powerUps.clear();
        particles.clear();
        slowRemaining = 0.0f;
    }
}

void Game::InitBricks() {
    bricks.clear();
    float totalWidth = brickCols * brickWidth + (brickCols - 1) * brickSpacingX;
    float actualStartX = (brickStartX == -1.0f) ? (screenWidth - totalWidth) / 2 : brickStartX;
    std::vector<Color> rowColors = {RED, ORANGE, YELLOW, GREEN, BLUE};
    for (int row = 0; row < brickRows; ++row) {
        Color color = rowColors[row % rowColors.size()];
        for (int col = 0; col < brickCols; ++col) {
            float x = actualStartX + col * (brickWidth + brickSpacingX);
            float y = brickStartY + row * (brickHeight + brickSpacingY);
            bricks.emplace_back(x, y, brickWidth, brickHeight, color);
        }
    }
}

void Game::ChangeState(GameState newState) {
    currentState = newState;
    TraceLog(LOG_INFO, "Game state changed to %d", (int)newState);
}

void Game::HandleInput() {
    if (IsKeyPressed(KEY_R)) {
        if (isHost) {
            score = 0;
            lives = 3;
            balls.clear();
            balls.emplace_back(Vector2{400, 450}, Vector2{0, 0}, 10);
            ballLaunched = false;
            paddle = Paddle(350, 550, 100, 20);
            InitBricks();
            currentState = GameState::PLAYING;
            powerUps.clear();
            particles.clear();
            slowRemaining = 0.0f;
            originalSpeeds.clear();
        }
        return;
    }

    if (isHost && currentState == GameState::PLAYING && IsKeyPressed(KEY_SPACE) && !ballLaunched) {
        if (!balls.empty()) {
            balls[0].Launch(paddle.GetRect().x + paddle.GetRect().width/2, paddle.GetRect().y);
            ballLaunched = true;
        }
    }

    if (isHost && currentState == GameState::PLAYING && IsKeyPressed(KEY_P)) {
        ChangeState(GameState::PAUSED);
    } else if (isHost && currentState == GameState::PAUSED && IsKeyPressed(KEY_P)) {
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
    // 异步加载处理（按 L 键触发）
    if (IsKeyPressed(KEY_L) && loadState == LoadState::IDLE) {
        {
            std::lock_guard<std::mutex> lock(loadMutex);
            loadState = LoadState::LOADING;
            loadCompleted = false;
        }
        loadFuture = std::async(std::launch::async, SimulateLoading);
    }

    if (loadState == LoadState::LOADING) {
        if (loadFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            loadFuture.get();
            {
                std::lock_guard<std::mutex> lock(loadMutex);
                loadState = LoadState::IDLE;
                loadCompleted = true;
            }
            // 加载完成后改变所有砖块颜色（演示效果）
            for (auto& brick : bricks) {
                brick.SetColor(WHITE);
            }
        }
    }
    HandleInput();
    net.Update();

    if (isHost) {
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
        }
        // 主机发送完整状态
        if (ballLaunched && net.IsConnected()) {
            GameStateMessage msg;
            msg.timestamp = GetTime();

            msg.ballCount = (int)balls.size();
            if (msg.ballCount > MAX_BALLS) msg.ballCount = MAX_BALLS;
            for (int i = 0; i < msg.ballCount; ++i) {
                msg.ballsX[i] = balls[i].GetPosition().x;
                msg.ballsY[i] = balls[i].GetPosition().y;
                msg.ballsSpeedX[i] = balls[i].GetSpeed().x;
                msg.ballsSpeedY[i] = balls[i].GetSpeed().y;
            }

            msg.paddleLeftX = paddle.GetRect().x;
            msg.paddleRightX = opponentPaddleX;
            msg.scoreLeft = score;
            msg.scoreRight = 0;
            msg.livesLeft = lives;
            msg.livesRight = 3;
            msg.gameRunning = (currentState == GameState::PLAYING);

            int idx = 0;
            for (int row = 0; row < brickRows; ++row) {
                for (int col = 0; col < brickCols; ++col) {
                    int brickIndex = row * brickCols + col;
                    if (brickIndex < (int)bricks.size()) {
                        msg.bricksActive[idx] = bricks[brickIndex].IsActive();
                    } else {
                        msg.bricksActive[idx] = false;
                    }
                    ++idx;
                }
            }

            msg.powerUpCount = (int)powerUps.size();
            if (msg.powerUpCount > MAX_POWERUPS) msg.powerUpCount = MAX_POWERUPS;
            for (int i = 0; i < msg.powerUpCount; ++i) {
                msg.powerUpPosX[i] = powerUps[i].position.x;
                msg.powerUpPosY[i] = powerUps[i].position.y;
                msg.powerUpType[i] = (int)powerUps[i].type;
                msg.powerUpDuration[i] = powerUps[i].duration;
            }

            net.SendToPeer(&msg, sizeof(msg));
        }
    } else {
        UpdateClient();
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
            for (int i = 0; i < 10; i++) {
                Particle p;
                p.position = { brick.GetRect().x + rand() % (int)brick.GetRect().width,
                               brick.GetRect().y + rand() % (int)brick.GetRect().height };
                p.velocity = { (rand()%100 - 50)/10.0f, (rand()%100 - 50)/10.0f };
                p.color = brick.GetColor();
                p.life = 0.5f;
                particles.push_back(p);
            }
            if ((float)(rand() % 100) / 100.0f < powerUpDropRate) {
                PowerUpType type = static_cast<PowerUpType>(rand() % 3);
                float x = brick.GetRect().x + brick.GetRect().width / 2;
                float y = brick.GetRect().y;
                powerUps.emplace_back(x, y, type);
            }
        }
    }

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
            balls.emplace_back(Vector2{400, 450}, Vector2{0, 0}, 10);
            balls[0].ResetToPaddle(paddle.GetRect().x + paddle.GetRect().width/2, paddle.GetRect().y);
            ballLaunched = false;
        }
    }

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
            if (originalSpeeds.size() == balls.size()) {
                for (size_t i = 0; i < balls.size(); ++i) {
                    balls[i].SetSpeed(originalSpeeds[i]);
                }
            }
            originalSpeeds.clear();
            slowRemaining = 0.0f;
        }
    }
}

void Game::UpdateClient() {
    float speed = 5;
    if (IsKeyDown(KEY_LEFT)) paddle.MoveLeft(speed);
    if (IsKeyDown(KEY_RIGHT)) paddle.MoveRight(speed);

    PaddleUpdateMessage msg{ paddle.GetRect().x };
    net.SendToPeer(&msg, sizeof(msg));
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
                    std::vector<Vector2> movingSpeeds;
                    for (const auto& b : balls) {
                        Vector2 spd = b.GetSpeed();
                        float mag = sqrt(spd.x*spd.x + spd.y*spd.y);
                        if (mag > 0.5f) movingSpeeds.push_back(spd);
                    }
                    if (movingSpeeds.empty()) movingSpeeds.push_back({5, -5});
                    std::vector<Ball> newBalls;
                    for (const auto& spd : movingSpeeds) {
                        float x = paddle.GetRect().x + paddle.GetRect().width / 2;
                        float y = paddle.GetRect().y - 15;
                        float angle = (rand() % 60 - 30) * 3.14159f / 180.0f;
                        float cosA = cos(angle);
                        float sinA = sin(angle);
                        Vector2 newSpd = { spd.x * cosA - spd.y * sinA,
                                           spd.x * sinA + spd.y * cosA };
                        if (fabs(newSpd.x) < 1.0f) newSpd.x = (rand() % 2 == 0 ? 4 : -4);
                        if (fabs(newSpd.y) < 1.0f) newSpd.y = (rand() % 2 == 0 ? 4 : -4);
                        Ball newBall(Vector2{x, y}, newSpd, 10);
                        newBall.Activate();
                        newBalls.push_back(newBall);
                    }
                    balls.insert(balls.end(), newBalls.begin(), newBalls.end());
                    break;
                }
                case PowerUpType::SLOW_BALL:
                    originalSpeeds.clear();
                    for (const auto& b : balls) {
                        originalSpeeds.push_back(b.GetSpeed());
                    }
                    slowRemaining = slowDuration;
                    for (auto& b : balls) {
                        Vector2 spd = b.GetSpeed();
                        b.SetSpeed({spd.x * slowFactorValue, spd.y * slowFactorValue});
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

    // 砖块
    for (auto& brick : bricks) brick.Draw();

    if (isHost) {
        for (auto& ball : balls) ball.Draw();
        paddle.Draw();
        // 仅在双人模式（有客户端连接）时才绘制对手板，否则不绘制
        if (net.IsConnected()) {
            DrawRectangle(opponentPaddleX, paddle.GetRect().y, paddle.GetRect().width, paddle.GetRect().height, SKYBLUE);
        }
    } else {
        // 客户端：使用接收到的状态绘制所有球
        if (firstStateReceived) {
            for (int i = 0; i < currentGameState.ballCount && i < MAX_BALLS; ++i) {
                DrawCircleV(Vector2{currentGameState.ballsX[i], currentGameState.ballsY[i]}, 10, RED);
            }
        } else {
            // 未收到任何状态时，显示一个默认球
            DrawCircleV(Vector2{400, 300}, 10, RED);
        }
        paddle.Draw();
        DrawRectangle(opponentPaddleX, paddle.GetRect().y, paddle.GetRect().width, paddle.GetRect().height, SKYBLUE);
    }

    for (auto& p : powerUps) p.Draw();
    DrawParticles();

    DrawText(TextFormat("Score: %d", score), 10, 10, 20, DARKGRAY);
    DrawText(TextFormat("Lives: %d", lives), 10, 40, 20, DARKGRAY);

    if (isHost && currentState == GameState::PLAYING && !ballLaunched) {
        DrawText("Press SPACE to launch", screenWidth/2 - 120, screenHeight - 30, 20, YELLOW);
    }
    if (currentState == GameState::PAUSED) {
        DrawText("PAUSED", screenWidth/2 - 50, screenHeight/2, 30, YELLOW);
        DrawText("Press P to resume", screenWidth/2 - 100, screenHeight/2 + 40, 20, WHITE);
    } else if (currentState == GameState::GAMEOVER) {
        DrawText("GAME OVER", screenWidth/2 - 70, screenHeight/2, 30, RED);
        DrawText("Press R to restart", screenWidth/2 - 100, screenHeight/2 + 40, 20, WHITE);
    } else if (currentState == GameState::VICTORY) {
        DrawText("YOU WIN!", screenWidth/2 - 60, screenHeight/2, 30, GREEN);
        DrawText("Press R to restart", screenWidth/2 - 100, screenHeight/2 + 40, 20, WHITE);
    }

    if (loadState == LoadState::LOADING) {
        DrawText("Loading...", screenWidth/2 - 60, screenHeight/2, 30, YELLOW);
    } else if (loadCompleted) {
        DrawText("Load Complete! Press L again to reload", screenWidth/2 - 200, screenHeight/2 + 50, 20, GREEN);
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
    if (config.contains("bricks")) {
        auto& b = config["bricks"];
        brickRows = b.value("rows", 5);
        brickCols = b.value("cols", 8);
        brickWidth = b.value("width", 70.0f);
        brickHeight = b.value("height", 25.0f);
        brickStartX = b.value("startX", -1.0f);
        brickStartY = b.value("startY", 80.0f);
        brickSpacingX = b.value("spacingX", 10.0f);
        brickSpacingY = b.value("spacingY", 5.0f);
    }
    lives = config["game"]["initialLives"];
    if (config.contains("powerups")) {
        auto& p = config["powerups"];
        powerUpDropRate = p["paddle_extend"]["drop_rate"];
        paddleExtendWidth = p["paddle_extend"]["extra_width"];
        paddleExtendDuration = p["paddle_extend"]["duration"];
        slowFactorValue = p["slow_ball"]["speed_factor"];
        slowDuration = p["slow_ball"]["duration"];
    }
}