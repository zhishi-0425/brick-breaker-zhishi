#pragma once
#include <cstddef>

#pragma pack(push, 1)

const int MAX_BALLS = 8;
const int MAX_POWERUPS = 20;
const int MAX_BRICKS = 40;   // 5行×8列 = 40

struct GameStateMessage {
    double timestamp;

    int ballCount;
    float ballsX[MAX_BALLS];
    float ballsY[MAX_BALLS];
    float ballsSpeedX[MAX_BALLS];
    float ballsSpeedY[MAX_BALLS];

    float paddleLeftX;
    float paddleRightX;

    int scoreLeft, scoreRight;
    int livesLeft, livesRight;
    bool gameRunning;

    bool bricksActive[MAX_BRICKS];

    int powerUpCount;
    float powerUpPosX[MAX_POWERUPS];
    float powerUpPosY[MAX_POWERUPS];
    int powerUpType[MAX_POWERUPS];
    float powerUpDuration[MAX_POWERUPS];
};

struct PaddleUpdateMessage {
    float paddleX;
};

#pragma pack(pop)