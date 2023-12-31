#include <MangoEngine/MangoEngine.hpp>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include "agent.hpp"

glm::vec2 random_vec2(unsigned int x_range, unsigned int y_range) {
    return glm::vec2 { rand() % x_range, rand() % y_range };
}

float get_vec_rotate(const glm::vec2 &x) {
    return glm::acos(glm::dot(glm::normalize(x), { 1, 0 })) * (x.y > 0 ? 1 : -1);
}

class Sandbox final : public MangoEngine::Application {
private:
    std::shared_ptr<MangoEngine::OrthographicCamera> camera;
    std::shared_ptr<MangoEngine::Texture> agent_texture, arrow_texture, dashed_line_texture;
    std::vector<Agent<glm::vec2>> agents;
    std::map<std::string, int> agent_map;
    float time_scaler = 1.0f;
    static constexpr double max_dt = 0.00005;
    // static constexpr double max_dt = 0.001;
    bool is_paused = true, is_follow_mouse = false;
    int selected_idx = 0;

    void create_agent(std::string name, const std::set<std::string> &leaders) {
        agent_map.insert(std::make_pair(name, agents.size()));
        agents.push_back(Agent<glm::vec2> { name, leaders });
    }

    void setup_agent(std::string name, glm::vec2 x, glm::vec2 v, glm::vec2 u = { 0, 0 }) {
        auto &agent = get_agent(name);
        agent.state.x = x;
        agent.state.v = v;
        agent.state.u = u;
        agent.back_state = agent.state;
    }

    Agent<glm::vec2> &get_agent(std::string name) {
        return agents[agent_map[name]];
    }

    void reset() {
        is_paused = true;
        is_follow_mouse = false;
        for (auto &agent : agents) {
            auto extent = camera->get_extent();
            agent.state.x = random_vec2(extent.x, extent.y) - extent / 2.0f;
            agent.state.v = random_vec2(1000, 1000) - glm::vec2 { 500, 500 };
            agent.state.u = { 0, 0 };
            agent.back_state = agent.state;
        }
    }

public:    
    void generate_engine_config(MangoEngine::EngineConfig *engine_config) override {
        engine_config->api = MangoEngine::RenderAPI::eVulkan;
        engine_config->title = "Multi Agent";
        engine_config->window_width = 1280;
        engine_config->window_height = 720;
        engine_config->window_x = 300;
        engine_config->window_y = 60;
    }

    MangoEngine::Result initialize() override {
        srand(std::chrono::system_clock::now().time_since_epoch().count());

        // create_agent("leader1", {});
        // std::vector<std::string> names = { "leader1" };
        // for (int i = 0; i < 20; i ++) {
        //     std::stringstream ss;
        //     ss << 'a' << i;
        //     std::set<std::string> leaders;
        //     for (int j = 0; j < 2; j ++) {
        //         leaders.insert(names[rand() % names.size()]);
        //     }
        //     create_agent(ss.str(), leaders);
        //     names.push_back(ss.str());
        // }

        create_agent("a1", { "a2", "a3", "a4"});
        create_agent("a2", { "a1", "a3" });
        create_agent("a3", { "a1", "a2", "a4" });
        create_agent("a4", { "a1", "a3" });

        camera = MangoEngine::camera_system->create_orthographic_camera({ 0, 0, 1 }, { 1280, 720 }, 2);
        agent_texture = MangoEngine::Texture::load_from_file("assets/textures/agent.png");
        arrow_texture = MangoEngine::Texture::load_from_file("assets/textures/arrow.png");
        dashed_line_texture = MangoEngine::Texture::load_from_file("assets/textures/dashed_line.png");

        // MangoEngine::render_system->set_bg_color(0.84, 0.86, 0.82, 1);
        MangoEngine::render_system->set_bg_color(1 - 0.84, 1 - 0.86, 1 - 0.82, 1);

        MangoEngine::event_system->add_event_callback<MangoEngine::WindowResizedEvent>([this] (auto &event) {
            this->camera->set_extent({ event.width, event.height });
            this->camera->update();
        });

        MangoEngine::event_system->add_event_callback<MangoEngine::KeyPressedEvent>([this] (auto &event) {
            switch (event.key) {
            case MangoEngine::Key::eSpace:
                this->is_paused = !this->is_paused;
                break;
            case MangoEngine::Key::eF:
                this->is_follow_mouse = !this->is_follow_mouse;
                break;
            case MangoEngine::Key::eW:
                this->selected_idx -= 1;
                if (this->selected_idx < 0) {
                    this->selected_idx += agents.size();
                }
                break;
            case MangoEngine::Key::eS:
                this->selected_idx += 1;
                if (this->selected_idx >= agents.size()) {
                    this->selected_idx = 0;
                }
                break;
            default:
                break;
            }
        });

        reset();

        return MangoEngine::Result::eSuccess;
    }

    MangoEngine::Result on_draw_frame() override {
        auto &command = MangoEngine::render_system->get_alpha_render_command();
        command.begin_scene(camera);

        int i = 0;
        auto get_length = [] (glm::vec2 &x) {
            return glm::clamp(glm::sqrt(glm::length(x)) / 100.0f, 0.04f, 0.14f) + 0.1f;
        };
        auto draw_agent = [&command, &get_length, this] (Agent<glm::vec2> &agent, bool is_selected = false) {
            command.draw_quad(agent.state.x, 80, agent.color, this->agent_texture);
            
            float length = get_length(agent.state.v);
            command.draw_quad(agent.state.x + glm::normalize(agent.state.v) * length * 64.0f, glm::vec2 { 128, 51.8 } * length, get_vec_rotate(agent.state.v), { 0, 0, 0, 1 }, arrow_texture);
            
            length = get_length(agent.state.u);
            command.draw_quad(agent.state.x + glm::normalize(agent.state.u) * length * 64.0f, glm::vec2 { 128, 51.8 } * length, get_vec_rotate(agent.state.u), { 0.4, 0.9, 0.6, 1 }, arrow_texture);
        };

        std::for_each(agents.begin(), agents.end(), [&i, &draw_agent, this] (Agent<glm::vec2> &agent) {
            if (i != this->selected_idx) {
                draw_agent(agent);
            }
            i ++;
        });
        draw_agent(agents[selected_idx], true);

        command.end_scene();
        return MangoEngine::Result::eSuccess;
    }
    
    MangoEngine::Result on_draw_imgui() override {
        if (ImGui::Button("Reset")) {
            reset();
        }

        ImGui::Separator();

        ImGui::SliderFloat("Time-Slow", &time_scaler, 0, 4);
        ImGui::SliderFloat("Time-Fast", &time_scaler, 4, 20);
        ImGui::Checkbox("Pause", &is_paused);
        ImGui::Checkbox("Follow Mouse", &is_follow_mouse);

        ImGui::Separator();

        auto &agent = agents[selected_idx];
        ImGui::Text("%s", (agent.name + " Editor").c_str());
        ImGui::SliderFloat("Alpha", &agent.alpha, 0, 5);
        ImGui::SliderFloat("Beta", &agent.beta, 0, 5);

        const char *items[agents.size()];
        int i = 0;
        for (const auto &agent : agents) {
            items[i++] = agent.name.c_str();
        }
        ImGui::ListBox("Agent Selector", &selected_idx, items, agents.size(), -1);

        return MangoEngine::Result::eSuccess;
    }

    MangoEngine::Result on_update(float fdt) override {
        if (is_paused || time_scaler == 0) {
            return MangoEngine::Result::eSuccess;
        }
        double t =  glm::clamp(fdt * time_scaler, 0.0f, static_cast<float>(max_dt * 100000.0));
        double dt;
        if (t > max_dt) {
            dt = max_dt;
        } else {
            dt = t;
        }

        while (t > 0) {
            std::for_each(agents.begin(), agents.end(), [] (Agent<glm::vec2> &agent) {
                std::swap(agent.state, agent.back_state);
            });

            std::for_each(agents.begin(), agents.end(), [this] (Agent<glm::vec2> &agent) {
                agent.state.u = { 0, 0 };
                for (const auto &leader_name : agent.leaders) {
                    const auto &leader = get_agent(leader_name);
                    agent.state.u += agent.alpha * (leader.back_state.x - agent.back_state.x) + agent.beta * (leader.back_state.v - agent.back_state.v);
                }
            });

            if (is_follow_mouse) {
                auto pos = MangoEngine::input_system->get_mouse_pos();
                auto extent = camera->get_extent();
                pos.x -= extent.x / 2.0f;
                pos.y = extent.y / 2.0f - pos.y;
                auto &selected_agent = agents[selected_idx];
                selected_agent.state.u += selected_agent.alpha * (pos - selected_agent.back_state.x) - selected_agent.beta * selected_agent.back_state.v;
            }

            std::for_each(agents.begin(), agents.end(), [dt] (Agent<glm::vec2> &agent) {
                agent.state.v = agent.back_state.v + static_cast<float>(dt) * agent.back_state.u;
                agent.state.x = agent.back_state.x + static_cast<float>(dt) * agent.back_state.v;
            });

            t -= max_dt;
            if (t < max_dt) {
                dt = t;
            }
        }

        return MangoEngine::Result::eSuccess;
    }

    MangoEngine::Result quit() override {
        return MangoEngine::Result::eSuccess;
    }
};

MangoEngine::Application *MangoEngine::create_application() {
    return new Sandbox();
}

#include <MangoEngine/entry.hpp>
