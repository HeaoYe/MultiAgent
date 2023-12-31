#pragma once

#include <MangoEngine/MangoEngine.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <set>
#include <map>


using uuid_t = unsigned int;
using Callback = std::function<void (void *)>;
class Agent;
enum class AgentEventType;
class AgentController;
class Network;
class World;

class Agent {
    friend World;
    static constexpr float display_size = 50;
    static constexpr float neighbor_dist = 150;
private:
    struct state_t {
        glm::vec2 x, v, u;
    } state, back_state;
    float alpha, beta;
    glm::vec4 color;
public:
    std::set<uuid_t> last_neighbors;
    std::set<uuid_t> neighbors;
    std::map<uuid_t, glm::vec2> deltas;
    uuid_t uuid;
    float get_alpha() { return alpha; }
    float get_beta() { return beta; }
    glm::vec2 get_v() { return state.v; }
    void set_u(glm::vec2 u) { state.u = u; }
};

struct AgentController {
    void operator() (Agent &agent);
};

class Network {
public:
    static Network &GetInstance() { return *_instance; }
    static void Initiallize(World &world) { _instance.reset(new Network { world }); };
    static void Destroy() { _instance.reset(); }

    void send(AgentEventType type, uuid_t dst);
    void send_recv(AgentEventType type, uuid_t dst, Callback callback);
private:
    static std::unique_ptr<Network> _instance;
    Network(World &world);
    World &world;
public:
    void update_step(float dt);
};

class World {
    static constexpr float dt_step = 0.01;
    static constexpr float max_step_num = 100;
    static constexpr int agent_reserve_num = 128;
public:
    void startup();
    void update(float dt_sum);
    void draw_agents();
    void draw_ui();
    void destroy();
private:
    Agent &create_agent();
    void reset();
    void update_step(float dt);
    std::vector<Agent> agents;
    float time_scale = 1.0f;
    struct NeighborQuery;
private:
    std::shared_ptr<MangoEngine::OrthographicCamera> camera;
    std::map<std::string, std::shared_ptr<MangoEngine::Texture>> textures;
};

glm::vec4 random_color(int min, int max, float alpha);
glm::vec2 random_pos(int min_x, int max_x, int min_y, int max_y);
