#include <cstdio>
#include <cassert>
#include "../src/Ball.h"
#include "../src/Paddle.h"

void TestBallPaddleCollision() {
    // 创建一个球，位于板的正上方（即将碰撞）
    Ball ball({100, 100}, {0, 5}, 10);
    Paddle paddle({90, 105, 20, 5});

    // 检测碰撞（应返回 true）
    bool collided = paddle.CheckCollision(ball);
    assert(collided == true);

    // 处理碰撞，速度 y 方向应变为负（向上）
    paddle.OnCollision(ball);
    assert(ball.GetSpeed().y < 0);

    printf("TestBallPaddleCollision passed.\n");
}

int main() {
    TestBallPaddleCollision();
    return 0;
}
