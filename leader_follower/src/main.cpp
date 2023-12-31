#include <initializer_list>
#include <memory>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

static enum class ParamFlag {
    T, DT, CFG, NONE,
} flag = ParamFlag::NONE;
static float t = 10;
static float dt = 0.1;
static std::string filename;

template<typename T>
class World;

template<typename T>
class Agent {
    friend World<T>;
private:
    Agent(std::string name, const std::vector<std::string> &leaders, const World<T> &world);
    std::string name;
    const World<T> &world;
    std::vector<std::string> leaders;

public:
    T get_u();
    void set_u(T u);
    void update();
    void swap();
private:
    bool is_leader() {
        return leaders.empty();
    }
    float alpha = 1.0f, beta = 1.5f;
    struct {
        T x {};
        T v {};
        T u {};
    } state, back_state;
};

template<typename T>
Agent<T>::Agent(std::string name, const std::vector<std::string> &leaders, const World<T> &world) : name(name), leaders(leaders), world(world) {}

template<typename T>
T Agent<T>::get_u() {
    return state.u;
}

template<typename T>
void Agent<T>::set_u(T u) {
    state.u = u;
}

template<typename T>
void Agent<T>::update() {
    if (!is_leader()) {
        state.u = 0;
        for (const auto &leader_name : leaders) {
            const auto &leader = world.get_agent(leader_name);
            state.u += alpha * (leader->back_state.x - state.x) + beta * (leader->back_state.v - state.v);
        }
        // state.u /= leaders.size();
    }
    state.v += dt * state.u;
    state.x += dt * state.v;
    std::cout << "[" << name << "] " << "x: " << state.x << ", v: " << state.v << ", u: " << state.u << std::endl;
}

template<typename T>
void Agent<T>::swap() {
    std::swap(state, back_state);
}

template<typename T>
class World {
public:
    void create_agent(std::string name, const std::vector<std::string> &leaders);
    void setup_agent(std::string name, T x, T v, T u, float alpha, float beta);
    const std::shared_ptr<Agent<T>> get_agent(std::string name) const;
    std::shared_ptr<Agent<T>> get_agent(std::string name);

    void step();
private:
    std::map<std::string, std::shared_ptr<Agent<T>>> agents;
};

template<typename T>
void World<T>::create_agent(std::string name, const std::vector<std::string> &leaders) {
    auto agent = std::shared_ptr<Agent<T>>(new Agent<T>(name, leaders, *this));
    agents.insert(std::make_pair(name, agent));
}

template<typename T>
void World<T>::setup_agent(std::string name, T x, T v, T u, float alpha, float beta) {
    auto agent = get_agent(name);
    agent->state.x = x;
    agent->state.v = v;
    agent->state.u = u;
    agent->alpha = alpha;
    agent->beta = beta;
    agent->back_state = agent->state;
}

template<typename T>
const std::shared_ptr<Agent<T>> World<T>::get_agent(std::string name) const {
    return agents.at(name);
}

template<typename T>
std::shared_ptr<Agent<T>> World<T>::get_agent(std::string name) {
    return agents.at(name);
}

template<typename T>
void World<T>::step() {
    for (auto &[name, agent] : agents) {
        agent->update();
    }
    for (auto &[name, agent] : agents) {
        agent->swap();
    }
}

void parser_args(int argc, char **argv) {
    for (int i = 0; i < argc; i ++) {
        switch (flag) {
        case ParamFlag::T: {
            t = std::atof(argv[i]);
            flag = ParamFlag::NONE;
            break;
        }
        case ParamFlag::DT: {
            dt = std::atof(argv[i]);
            flag = ParamFlag::NONE;
            break;
        }
        case ParamFlag::CFG: {
            filename = std::string(argv[i]);
            flag = ParamFlag::NONE;
            break;
        }
        default: {
            if (strcmp("-t", argv[i]) == 0) {
                flag = ParamFlag::T;
            }
            else if (strcmp("-dt", argv[i]) == 0) {
                flag = ParamFlag::DT;
            }
            else if (strcmp("-cfg", argv[i]) == 0) {
                flag = ParamFlag::CFG;
            }
        }
        }
    }
}

template<typename T>
void load_config(World<T> &world) {
    std::ifstream file(filename, std::ios::in);
    if (!file.is_open()) {
        world.create_agent("leader", {});
        world.create_agent("a1", { "leader" });
        world.create_agent("a2", { "a1", "leader" });
        return;
    }
    std::string line;
    while (!file.eof()) {
        std::getline(file, line);
        while ((!line.empty()) && (line.back() == ' ')) {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }
        if (*line.begin() == '#') {
            continue;
        }
        std::istringstream iss(line.c_str());
        std::string name;
        iss >> name;
        T x, v, u;
        iss >> x >> v >> u;
        float alpha, beta;
        iss >> alpha >> beta;
        std::vector<std::string> leaders;
        while (!iss.eof()) {
            std::string leader_name;
            iss >> leader_name;
            leaders.push_back(leader_name);
        }
        world.create_agent(name, leaders);
        world.setup_agent(name, x, v, u, alpha, beta);
    }
}

int main(int argc, char **argv) {
    parser_args(argc, argv);

    World<float> world;
    load_config<float>(world);
    
    auto leader = world.get_agent("leader");

    int frame_count = t / dt;
    for (int i = 1; i <= frame_count; i ++) {
        std::cout << "frame: " << i << ", dt: " << dt << ", t: " << dt * i << std::endl;
        // leader->set_u(((frame_count % 2) * 2 - 1) * static_cast<float>(i) / static_cast<float>(frame_count));
        // leader->set_u(leader->get_u() + (rand() % 3 - 1));

        world.step();

        std::cout << std::endl; 
    }
}
