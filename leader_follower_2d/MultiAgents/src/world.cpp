#include "world.hpp"
#include <glm/gtx/norm.hpp>
#include <sstream>

glm::vec2 screen2view(glm::vec2 pos, std::weak_ptr<MangoEngine::OrthographicCamera> camera) {
    auto extent = camera.lock()->get_extent();
    pos.x -= extent.x / 2.0f;
    pos.y = extent.y / 2.0f - pos.y;
    return pos;
}

void World::create_agent(std::string name, const T& x, const T& v) {
    agent_map.insert(std::make_pair(name, agents.size()));
    auto &agent = agents.emplace_back(name, Agent::State { x, T {}, T {} });
    static constexpr int n = 200;
    agent.color = { rand() % n + 55, rand() % n + 55, rand() % n + 55, 220 };
    agent.color /= 255;
    agent.uuid = agents.size();
}

void World::startup() {
    camera = MangoEngine::camera_system->create_orthographic_camera({ 0, 0, 1 }, { MangoEngine::engine_config->window_width, MangoEngine::engine_config->window_height }, 2);
    // MangoEngine::render_system->set_bg_color(0.84, 0.86, 0.82, 1);
    MangoEngine::render_system->set_bg_color(1 - 0.84, 1 - 0.86, 1 - 0.82, 1);
    textures["agent"] = MangoEngine::Texture::load_from_file("assets/textures/agent.png");
    textures["rect"] = MangoEngine::Texture::load_from_file("assets/textures/rect.png");

    MangoEngine::event_system->add_event_callback<MangoEngine::WindowResizedEvent>([this] (auto &event) {
        this->camera->set_extent({ event.width, event.height });
        this->camera->update();
    });

    MangoEngine::event_system->add_event_callback<MangoEngine::MousePressedEvent>([this] (auto &event) {
        if (event.button == MangoEngine::MouseButton::eLeft) {
            static constexpr float l2 = Agent::display_size * Agent::display_size / 4.0f;
            auto pos = screen2view(MangoEngine::input_system->get_mouse_pos(), camera);
            int i = 0;
            is_mouse_downed = true;
            for (const auto &agent : agents) {
                if (glm::length2(pos - agent.state.x) < l2) {
                    selector = i;
                    bool found = false;
                    for (int s_i : selected_agents) {
                        if (s_i == i) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        selected_agents.clear();
                        selected_agents.insert(i);
                    }
                    break;
                }
                i ++;
            }
            dp = MangoEngine::input_system->get_mouse_pos();
            for (int i : selected_agents) {
                selected_agents_rp[i] = agents[i].state.x;
            }
        } else if (event.button == MangoEngine::MouseButton::eRight) {
            is_select_rect = true;
            s_dp = screen2view(MangoEngine::input_system->get_mouse_pos(), camera);
        }
    });
    
    MangoEngine::event_system->add_event_callback<MangoEngine::MouseReleasedEvent>([this] (auto &event) {
        if (event.button == MangoEngine::MouseButton::eLeft) {
            is_mouse_downed = false;
        } else if (event.button == MangoEngine::MouseButton::eRight) {
            is_select_rect = false;
            int i = 0;
            selected_agents.clear();
            for (const auto &agent : agents) {
                auto p = screen2view(MangoEngine::input_system->get_mouse_pos(), camera);
                glm::vec2 p1 = glm::min(s_dp, p), p2 = glm::max(s_dp, p);
                if (p1.x < agent.state.x.x && agent.state.x.x < p2.x && p1.y < agent.state.x.y && agent.state.x.y < p2.y) {
                    selected_agents.insert(i);
                }
                i ++;
            }
        }
    });

    MangoEngine::event_system->add_event_callback<MangoEngine::MouseMovedEvent>([this] (auto &event) {
        if (is_mouse_downed && is_mouse_drag) {
            for (int i : selected_agents) {
                agents[i].state.x = selected_agents_rp[i] + glm::vec2 { 1, -1 } * (MangoEngine::input_system->get_mouse_pos() - dp);
            }
        }
    });

    MangoEngine::event_system->add_event_callback<MangoEngine::KeyPressedEvent>([this] (auto &event) {
        switch (event.key) {
        case MangoEngine::Key::eSpace:
            is_paused = !is_paused;
            break;
        case MangoEngine::Key::eF:
            is_follow_mouse = !is_follow_mouse;
            break;
        case MangoEngine::Key::eW:
            selector -= 1;
            if (selector < 0) {
                selector += agents.size();
            }
            break;
        case MangoEngine::Key::eS:
            selector += 1;
            if (selector >= agents.size()) {
                selector = 0;
            }
            break;
        case MangoEngine::Key::eQ:
            is_select_rect = true;
            s_dp = screen2view(MangoEngine::input_system->get_mouse_pos(), camera);
            break;
        default:
            break;
        }
    });

    MangoEngine::event_system->add_event_callback<MangoEngine::KeyReleasedEvent>([this] (auto &event) {
        if (event.key == MangoEngine::Key::eQ) {
            is_select_rect = false;
            int i = 0;
            selected_agents.clear();
            for (const auto &agent : agents) {
                auto p = screen2view(MangoEngine::input_system->get_mouse_pos(), camera);
                glm::vec2 p1 = glm::min(s_dp, p), p2 = glm::max(s_dp, p);
                if (p1.x < agent.state.x.x && agent.state.x.x < p2.x && p1.y < agent.state.x.y && agent.state.x.y < p2.y) {
                    selected_agents.insert(i);
                }
                i ++;
            }
        }
    });

    // create_agent("Test1", { 0, 0 }, { 0, 0 });
    // create_agent("Test2", { 150, 150 }, { 0, 0 });
    // create_agent("Test3", { -150, 150 }, { 0, 0 });
    // create_agent("Test4", { -150, -150 }, { 0, 0 });
    // create_agent("Test5", { 150, -150 }, { 0, 0 });

    for (int i = 0; i < 150; i ++) {
        std::stringstream ss;
        ss << "Agent-" << i + 1;
        create_agent(ss.str(), { 0, 0 }, { 0, 0 });
    }

    reset();
}

void World::reset() {
    auto extent = camera->get_extent();
    for (auto &agent : agents) {
        agent.state.x = glm::vec2 { rand() % static_cast<int>(extent.x), rand() % static_cast<int>(extent.y) } - extent / 2.0f;
        agent.state.v = { 0, 0 };
        agent.state.u = { 0, 0 };
        agent.back_state = agent.state;
        agent.last_neighbors.clear();
        agent.neighbors.clear();
        agent.deltas.clear();
        agent.ctrl = {};
    }
    is_follow_mouse = false;
    is_mouse_drag = true;
    is_paused = true;
    is_highlight = true;
}

void World::update(float dt) {
    if (is_paused) {
        return;
    }

    dt = glm::min(dt, 0.03f);
    float step_dt = max_step_dt;
    while (dt > 0) {
        if (dt < max_step_dt) {
            step_dt = dt;
        }
        dt -= step_dt;
        std::for_each(agents.begin(), agents.end(), [this] (Agent &agent) {
            std::swap(agent.state, agent.back_state);
            agent.last_neighbors = query_neighbors(agent.name);
            std::swap(agent.last_neighbors, agent.neighbors);
            agent.deltas.clear();
            for (const auto &neighbor_name : agent.neighbors) {
                agent.deltas[neighbor_name] = get_agent(neighbor_name).back_state.x - agent.back_state.x;
            }
        });

        std::for_each(agents.begin(), agents.end(), [this, step_dt] (Agent &agent) {
            agent.update(*this, step_dt);
        });

        auto &agent = agents[selector];
        if (is_follow_mouse) {
            auto pos = screen2view(MangoEngine::input_system->get_mouse_pos(), camera);
            agent.state.u += agent.alpha * (pos - agent.back_state.x) - agent.beta * agent.back_state.v;
        }

        std::for_each(agents.begin(), agents.end(), [this, step_dt] (Agent &agent) {
            agent.state.v = agent.back_state.v + step_dt * agent.back_state.u;
            agent.state.x = agent.back_state.x + step_dt * agent.back_state.v;
        });
    }
}

void World::draw_agents() {
    auto &command = MangoEngine::render_system->get_render_command();

    if (is_highlight) {
        for (int i : selected_agents) {
            command.draw_quad(agents[i].state.x, Agent::display_size + 10, { 0.1, 0.4, 0.66, 0.7 }, textures["rect"]);
        }
        command.draw_quad(agents[selector].state.x, Agent::display_size + 10, { 0.1, 0.66, 0.4, 0.7 }, textures["rect"]);
    }
    std::for_each(agents.begin(), agents.end(), [&command, this] (Agent &agent) {
        
        command.draw_quad(agent.state.x, Agent::display_size, agent.color, textures["agent"]);
        if (agent.ctrl.has_team) {
            command.draw_quad(agent.state.x, Agent::display_size / 3, agent.ctrl.team.color, textures["agent"]);
        }
    });
    if (is_select_rect) {
        auto p2 = screen2view(MangoEngine::input_system->get_mouse_pos(), camera);
        command.draw_quad((p2 + s_dp) / 2.0f, glm::abs(p2 - s_dp), { 0.4, 0.5, 0.8, 0.5 });
    }
}

void World::destroy() {
    agents.clear();
    agent_map.clear();
}

Agent &World::get_agent(std::string name) {
    return agents[agent_map[name]];
}

std::vector<std::string> World::query_neighbors(std::string querier) {
    static constexpr float neighbor_dist2 = neighbor_dist * neighbor_dist;
    std::vector<std::string> neighbors;
    auto &querier_agent = get_agent(querier);
    std::for_each(agents.begin(), agents.end(), [&neighbors, &querier, &querier_agent] (Agent &agent) {
        if (querier == agent.name) {
            return;
        }
        if (glm::length2(agent.state.x - querier_agent.state.x) <= neighbor_dist2) {
            neighbors.push_back(agent.name);
        }
    });
    return std::move(neighbors);
}
