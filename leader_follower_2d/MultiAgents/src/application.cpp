#include "application.hpp"

void MultiAgentDemo::generate_engine_config(MangoEngine::EngineConfig *engine_config) {
    engine_config->api = MangoEngine::RenderAPI::eVulkan;
    engine_config->title = "Multi Agent";
    engine_config->window_width = 1280;
    engine_config->window_height = 720;
    engine_config->window_x = 300;
    engine_config->window_y = 60;
}

MangoEngine::Result MultiAgentDemo::initialize() {
    world.startup();
    return MangoEngine::Result::eSuccess;
}

MangoEngine::Result MultiAgentDemo::on_update(float dt) {
    world.update(dt);
    return MangoEngine::Result::eSuccess;
}

MangoEngine::Result MultiAgentDemo::on_draw_frame() {
    auto &command = MangoEngine::render_system->get_render_command();
    command.begin_scene(world.get_camera());
    world.draw_agents();
    command.end_scene();
    return MangoEngine::Result::eSuccess;
}

MangoEngine::Result MultiAgentDemo::on_draw_imgui() {
    world.draw_ui();
    return MangoEngine::Result::eSuccess;
}

MangoEngine::Result MultiAgentDemo::quit() {
    world.destroy();
    return MangoEngine::Result::eSuccess;
}

MangoEngine::Application *MangoEngine::create_application() {
    return new MultiAgentDemo();
}
