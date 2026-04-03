#include <cassert>
#include <cstdio>
#include "../src/Ball.h"
#include "../src/Paddle.h"

void TestBallPaddleCollision() {
    Ball ball({100, 100}, {0, 5}, 10);
    Paddle paddle({90, 105, 20, 5});

    bool collided = paddle.CheckCollision(ball);
    assert(collided == true);

    paddle.OnCollision(ball);
    assert(ball.GetSpeed().y < 0);

    printf("TestBallPaddleCollision passed.\n");
}

int main() {
    TestBallPaddleCollision();
    return 0;
}