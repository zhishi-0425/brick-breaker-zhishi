#pragma once
#include <enet/enet.h>
#include <functional>
#include <vector>

enum class NetworkRole { None, Host, Client };

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();
    bool Init();
    bool CreateHost(int port);
    bool ConnectToHost(const char* ip, int port);
    void Disconnect();
    void Update();  // 每帧调用，处理网络事件
    void SendToPeer(const void* data, size_t size);
    void SetOnReceiveCallback(std::function<void(const unsigned char*, size_t)> callback);
    NetworkRole GetRole() const { return role; }
    bool IsConnected() const;
private:
    ENetHost* host;
    ENetPeer* peer;
    NetworkRole role;
    std::function<void(const unsigned char*, size_t)> onReceive;
};