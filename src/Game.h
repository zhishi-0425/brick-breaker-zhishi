#ifndef GAME_H
#define GAME_H

#include <string>
#include <vector>
#include "raylib.h"
#include "Ball.h"
#include "Paddle.h"
#include "Brick.h"
#include "PowerUp.h"
#include "NetworkManager.h"
#include "NetworkMessages.h"
#include <future>
#include <atomic>

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

    // 配置文件参数
    float powerUpDropRate;
    float paddleExtendWidth;
    float paddleExtendDuration;
    float slowFactorValue;
    float slowDuration;

    int brickRows;
    int brickCols;
    float brickWidth;
    float brickHeight;
    float brickStartX;
    float brickStartY;
    float brickSpacingX;
    float brickSpacingY;

    std::vector<Vector2> originalSpeeds;

    // 网络相关
    NetworkManager net;
    bool isHost;
    bool isClient;
    float opponentPaddleX;
    GameStateMessage currentGameState;
    GameStateMessage lastGameState;
    double lastStateTime;
    double currentStateTime;
    float interpBallX, interpBallY;
    float interpOpponentPaddleX;
    bool firstStateReceived;

    // 异步加载相关
    enum class LoadState { IDLE, LOADING, DONE };
    LoadState loadState;
    std::future<void> loadFuture;
    std::mutex loadMutex;
    bool loadCompleted;

    void InitBricks();
    void ChangeState(GameState newState);
    void HandleInput();
    void UpdatePlaying();
    void UpdateClient();
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
    void Init(bool asHost = false, const char* serverIp = nullptr);
    void Update();
    void Draw();
    void Shutdown();
    void LoadConfig(const std::string& path);
};

#endif