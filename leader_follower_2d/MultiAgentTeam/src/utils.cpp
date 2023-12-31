#include "commons.hpp"

glm::vec4 random_color(int min, int max, float alpha) {
    int d = max - min;
    return glm::vec4 { (rand() % d) + min, (rand() % d) + min, (rand() % d) + min, alpha * 255.0f } / 255.0f;
}

glm::vec2 random_pos(int min_x, int max_x, int min_y, int max_y) {
    return { (rand() % (max_x - min_x)) + min_x, (rand() % (max_y - min_y)) + min_y };
}
