#include "GameState.h"

GameState::GameState() : lives(3), score(0), gameOver(false), victory(false) {}

void GameState::Reset() {
    lives = 3;
    score = 0;
    gameOver = false;
    victory = false;
}

void GameState::AddScore(int points) {
    score += points;
    if (score < 0) score = 0;
}

void GameState::LoseLife() {
    if (lives > 0) lives--;
    if (lives <= 0) {
        gameOver = true;
        victory = false;
    }
}

bool GameState::IsGameOver() const { return gameOver; }
bool GameState::IsVictory() const { return victory; }
int GameState::GetLives() const { return lives; }
int GameState::GetScore() const { return score; }
void GameState::SetGameOver(bool over) { gameOver = over; }
void GameState::SetVictory(bool win) { victory = win; }