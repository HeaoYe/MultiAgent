#include "commons.hpp"

void World::draw_agents() {
    auto &command = MangoEngine::render_system->get_alpha_render_command();
    command.begin_scene(camera);
    std::for_each(agents.begin(), agents.end(), [&command, this] (auto &agent) {        
        command.draw_quad(agent.state.x, Agent::display_size, agent.color, textures["agent"]);
        // if (agent.ctrl.has_team) {
            // command.draw_quad(agent.state.x, Agent::display_size / 3, agent.ctrl.team.color, textures["agent"]);
        // }
    });
    command.end_scene();
}

void World::draw_ui() {
}
