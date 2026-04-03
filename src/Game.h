#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "Ball.h"
#include "Paddle.h"
#include "Brick.h"
#include <vector>
#include <string>

// 游戏状态枚举
enum class GameState {
    MENU,       // 主菜单（可选）
    PLAYING,    // 游戏中
    PAUSED,     // 暂停
    GAMEOVER,   // 游戏结束（失败）
    VICTORY     // 胜利
};

class Game {
private:
    // 游戏对象
    Ball ball;
    Paddle paddle;
    std::vector<Brick> bricks;

    // 游戏状态
    GameState currentState;
    bool ballLaunched;      // 球是否已发射（仅在PLAYING状态有效）
    int score;
    int lives;

    int scorePerBrick;
    float paddleSpeed;
    float ballGravity;
    float ballMaxSpeed;
    float ballBounceForce;

    // 窗口尺寸
    int screenWidth;
    int screenHeight;

    // 私有方法
    void InitBricks();                      // 初始化砖块
    void ChangeState(GameState newState);   // 切换状态
    void HandleInput();                     // 处理输入（按键）
    void UpdatePlaying();                   // 更新游戏逻辑（PLAYING状态）
    void UpdatePaused();                    // 暂停状态更新（空）
    void UpdateGameOver();                  // 游戏结束状态更新
    void UpdateVictory();                   // 胜利状态更新
    void DrawPlaying();                     // 绘制游戏界面（PLAYING）
    void DrawPaused();                      // 绘制暂停界面
    void DrawGameOver();                    // 绘制游戏结束界面
    void DrawVictory();                     // 绘制胜利界面

    void LoadConfig(const std::string& path);

public:
    Game();
    ~Game();

    void Init();                // 初始化游戏（加载配置、重置状态）
    void Update();              // 每帧更新（根据当前状态调用对应更新函数）
    void Draw();                // 每帧绘制（根据当前状态调用对应绘制函数）
    void Shutdown();            // 清理资源（如果有）
};

#endif