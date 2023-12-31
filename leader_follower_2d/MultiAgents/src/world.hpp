#pragma once

#include "agent.hpp"
#include <map>
#include <MangoEngine/MangoEngine.hpp>

class World {
    friend Agent;
    // static constexpr float neighbor_dist = 150;
    static constexpr float neighbor_dist = 100;
    // static constexpr float max_step_dt = 0.001;
    static constexpr float max_step_dt = 0.001;
public:
    void create_agent(std::string name, const T& x, const T& v);

    void startup();
    void reset();
    void update(float dt);
    void draw_agents();
    void draw_ui();
    void destroy();
private:
    Agent &get_agent(std::string name);

    template<typename ReplyType>
    ReplyType *send_event(AgentEventType type, std::string dst, void *data = nullptr);
    std::vector<std::string> query_neighbors(std::string querier);
private:
    std::vector<Agent> agents;
    std::map<std::string, int> agent_map;
    int selector = 0;
    bool is_paused = true, is_follow_mouse = false, is_mouse_downed = false, is_mouse_drag = true, is_highlight=true;
    glm::vec2 dp;
    bool is_select_rect = false;
    glm::vec2 s_dp;
    std::set<int> selected_agents;
    std::map<int, glm::vec2> selected_agents_rp;

    std::shared_ptr<MangoEngine::OrthographicCamera> camera;
public:
    std::shared_ptr<MangoEngine::OrthographicCamera> get_camera() { return camera; }
    std::map<std::string, std::shared_ptr<MangoEngine::Texture>> textures;
};


template<typename ReplyType>
ReplyType *World::send_event(AgentEventType type, std::string dst, void *data) {
    auto &dst_agent = get_agent(dst);
    return reinterpret_cast<ReplyType *>(dst_agent.handle_event(*this, type, data));
}
