#ifndef GAMESTATE_H
#define GAMESTATE_H

class GameState {
private:
    int lives;
    int score;
    bool gameOver;
    bool victory;
public:
    GameState();
    void Reset();
    void AddScore(int points);
    void LoseLife();
    bool IsGameOver() const;
    bool IsVictory() const;
    int GetLives() const;
    int GetScore() const;
    void SetGameOver(bool over);
    void SetVictory(bool win);
};

#endif