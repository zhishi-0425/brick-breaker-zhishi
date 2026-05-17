cmake_minimum_required(VERSION 3.10)
project(breakout_week2)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(ENET REQUIRED libenet)

add_executable(breakout_week2
    src/main.cpp
    src/Ball.cpp
    src/Paddle.cpp
    src/Brick.cpp
    src/Game.cpp
    src/PowerUp.cpp
    src/NetworkManager.cpp   # 稍后创建
)

target_include_directories(breakout_week2 PRIVATE ${ENET_INCLUDE_DIRS})
target_link_libraries(breakout_week2 ${ENET_LIBRARIES} raylib m pthread dl rt X11)