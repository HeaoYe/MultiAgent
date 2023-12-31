#include "commons.hpp"

void Network::update_step(float dt) {
}

void Network::send(AgentEventType type, uuid_t dst) {
}

void Network::send_recv(AgentEventType type, uuid_t dst, Callback callback) {
}

std::unique_ptr<Network> Network::_instance = nullptr;

Network::Network(World &world) : world(world) {}

