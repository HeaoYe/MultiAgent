#include "commons.hpp"
#include <glm/gtx/norm.hpp>

struct World::NeighborQuery {
    static constexpr float neighbor_dist2 = Agent::neighbor_dist * Agent::neighbor_dist;
    std::set<uuid_t> neighbors;
    Agent *agent = nullptr;
    
    void set_agent(Agent &agent) {
        this->agent = &agent;
        agent.deltas.clear();
    }

    void operator()(Agent &agent) {
        auto delta = agent.state.x - this->agent->state.x;
        if (glm::length2(delta) < neighbor_dist2) {
            this->agent->deltas.insert(std::make_pair(agent.uuid, delta));
            neighbors.insert(agent.uuid);
        }
    }

    operator std::set<uuid_t>() {
        return std::move(neighbors);
    }
} query;
AgentController controller;

void World::update_step(float dt) {
    Network::GetInstance().update_step(dt);

    std::for_each(agents.begin(), agents.end(), [this] (auto &agent) {
        query.set_agent(agent);
        agent.last_neighbors = std::for_each(agents.begin(), agents.end(), query);
        std::swap(agent.neighbors, agent.last_neighbors);
        std::swap(agent.state, agent.back_state);
    });

    std::for_each(agents.begin(), agents.end(), controller);

    std::for_each(agents.begin(), agents.end(), [dt] (auto &agent) {
        agent.state.v += dt * agent.back_state.u;
        agent.state.x += dt * agent.back_state.v;
    });
}

void World::startup() {
    camera = MangoEngine::camera_system->create_orthographic_camera({ 0, 0, 1 }, { MangoEngine::engine_config->window_width, MangoEngine::engine_config->window_height }, 2);
    MangoEngine::render_system->set_bg_color(1 - 0.84, 1 - 0.86, 1 - 0.82, 1);
    textures["agent"] = MangoEngine::Texture::load_from_file("assets/textures/agent.png");
    textures["rect"] = MangoEngine::Texture::load_from_file("assets/textures/rect.png");
    agents.reserve(agent_reserve_num);
    Network::Initiallize(*this);

    for (int i = 0; i < agent_reserve_num; i++) {
        create_agent();
    }


    reset();
}

void World::destroy() {
    Network::Destroy();
    agents.clear();
    textures.clear();
    camera.reset();
}

void World::update(float dt_sum) {
    dt_sum = std::min(dt_sum * time_scale, max_step_num * dt_step);
    float dt = dt_step;
    while (dt_sum > 0) {
        if (dt_sum < dt_step) {
            dt = dt_sum;
        }
        update_step(dt);
        dt_sum -= dt_step;
    }
}

Agent &World::create_agent() {
    static uuid_t uuid_counter = 1;
    auto &agent = agents.emplace_back();
    agent.color = random_color(100, 255, 0.8f);
    agent.uuid = uuid_counter;
    uuid_counter ++;
    agent.alpha = 10.0f;
    agent.beta = 15.0f;
    auto extent = camera->get_extent();
    return agent;
}

void World::reset() {
    std::for_each(agents.begin(), agents.end(), [this] (auto &agent) {
        auto extent = camera->get_extent();
        agent.state.x = random_pos(0, extent.x, 0, extent.y) - extent * 0.5f;
        agent.state.v = { 0, 0 };
        agent.state.u = { 0, 0 };
        agent.back_state = agent.state;
        agent.neighbors.clear();
        agent.last_neighbors.clear();
        agent.deltas.clear();
    });
}
