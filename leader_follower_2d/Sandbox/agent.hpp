#pragma once

#include <string>
#include <set>
#include <glm/glm.hpp>

static constexpr int n = 200;
template <typename T>
struct Agent {
    Agent(std::string name, const std::set<std::string> &leaders) : name(name), leaders(leaders) {
        color = { rand() % n + 55, rand() % n + 55, rand() % n + 55, 220 };
        color /= 255;
    }

    std::string name;
    float alpha = 1.0f, beta = 1.5f;
    struct state_t {
        T x {}, v {}, u {};
    } state, back_state;
    std::set<std::string> leaders;
    glm::vec4 color;
};
