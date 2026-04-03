# 打砖块游戏 - 第四周重构版

## 项目简介
基于 C++ 和 Raylib 的打砖块游戏，第四周重构：
- 游戏逻辑封装到 `Game` 类
- 状态机管理（PLAYING, PAUSED, GAMEOVER, VICTORY）
- 参数从 `config.json` 读取
- 单元测试（assert）

## 编译与运行

### 依赖
- Raylib 5.0
- nlohmann/json (用于配置文件)


### 编译
```bash
mkdir build && cd build
cmake ..
make
