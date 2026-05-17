#include "NetworkManager.h"
#include <cstdio>

NetworkManager::NetworkManager() : host(nullptr), peer(nullptr), role(NetworkRole::None) {}

NetworkManager::~NetworkManager() { Disconnect(); }

bool NetworkManager::Init() {
    if (enet_initialize() != 0) {
        printf("ENet initialization failed\n");
        return false;
    }
    return true;
}

bool NetworkManager::CreateHost(int port) {
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;
    host = enet_host_create(&address, 1, 2, 0, 0);
    if (!host) {
        printf("Failed to create host on port %d\n", port);
        return false;
    }
    role = NetworkRole::Host;
    printf("Host created on port %d, waiting for client...\n", port);
    return true;
}

bool NetworkManager::ConnectToHost(const char* ip, int port) {
    host = enet_host_create(nullptr, 1, 2, 0, 0);
    if (!host) {
        printf("Failed to create client host\n");
        return false;
    }
    ENetAddress address;
    enet_address_set_host(&address, ip);
    address.port = port;
    peer = enet_host_connect(host, &address, 2, 0);
    if (!peer) {
        printf("Failed to connect to %s:%d\n", ip, port);
        return false;
    }
    role = NetworkRole::Client;
    printf("Connecting to %s:%d...\n", ip, port);
    return true;
}

void NetworkManager::Disconnect() {
    if (peer) enet_peer_disconnect(peer, 0);
    if (host) {
        enet_host_flush(host);
        enet_host_destroy(host);
        host = nullptr;
    }
    peer = nullptr;
    role = NetworkRole::None;
    enet_deinitialize();
}

void NetworkManager::Update() {
    if (!host) return;
    ENetEvent event;
    while (enet_host_service(host, &event, 0) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                printf("Connected to server.\n");
                peer = event.peer;
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                if (onReceive) onReceive(event.packet->data, event.packet->dataLength);
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                printf("Disconnected.\n");
                peer = nullptr;
                break;
        }
    }
}

void NetworkManager::SendToPeer(const void* data, size_t size) {
    if (peer && (role == NetworkRole::Host || role == NetworkRole::Client)) {
        ENetPacket* packet = enet_packet_create(data, size, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, packet);
    }
}

void NetworkManager::SetOnReceiveCallback(std::function<void(const unsigned char*, size_t)> callback) {
    onReceive = callback;
}

bool NetworkManager::IsConnected() const {
    return peer != nullptr;
}