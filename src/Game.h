#ifndef GAME_H
#define GAME_H

#include <string>
#include <vector>
#include "raylib.h"
#include "Ball.h"
#include "Paddle.h"
#include "Brick.h"
#include "PowerUp.h"

enum class GameState {
    PLAYING,
    PAUSED,
    GAMEOVER,
    VICTORY
};

struct Particle {
    Vector2 position;
    Vector2 velocity;
    Color color;
    float life;
};

class Game {
private:
    std::vector<Ball> balls;
    Paddle paddle;
    std::vector<Brick> bricks;
    std::vector<PowerUp> powerUps;
    std::vector<Particle> particles;

    GameState currentState;
    bool ballLaunched;
    int score;
    int lives;
    int screenWidth;
    int screenHeight;
    float deltaTime;
    float slowRemaining;
    float slowFactor;

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