#ifndef GAME_H
#define GAME_H

#include <string> 
#include "raylib.h"
#include "Ball.h"
#include "Paddle.h"
#include "Brick.h"
#include "PowerUp.h"
#include <vector>

enum class GameState {
    PLAYING,
    PAUSED,
    GAMEOVER,
    VICTORY
};

// 粒子结构
struct Particle {
    Vector2 position;
    Vector2 velocity;
    Color color;
    float life;
};

class Game {
private:
    std::vector<Ball> balls;          // 多球管理
    Paddle paddle;
    std::vector<Brick> bricks;
    std::vector<PowerUp> powerUps;    // 道具列表
    std::vector<Particle> particles;  // 粒子列表

    GameState currentState;
    bool ballLaunched;       // 是否已发射（主球）
    int score;
    int lives;
    int screenWidth;
    int screenHeight;
    float deltaTime;         // 帧时间
    float slowRemaining;     // 减速剩余时间
    float slowFactor;        // 减速因子

    void InitBricks();
    void ChangeState(GameState newState);
    void HandleInput();
    void UpdatePlaying();
    void UpdatePaused();
    void UpdateGameOver();
    void UpdateVictory();
    void UpdatePowerUps(float dt);
    void CheckPowerUpCollision();
    void UpdateParticles(float dt);
    void DrawParticles();

public:
    Game();
    ~Game();
    void Init();
    void Update();
    void Draw();
    void Shutdown();
    void LoadConfig(const std::string& path);
};

#endif