#pragma once

#include <MangoEngine/MangoEngine.hpp>
#include "commons.hpp"


class MultiAgentTeamDemo final : public MangoEngine::Application {
public:
    void generate_engine_config(MangoEngine::EngineConfig *engine_config) override;
    MangoEngine::Result initialize() override;
    MangoEngine::Result on_update(float dt) override;
    MangoEngine::Result on_draw_frame() override;
    MangoEngine::Result on_draw_imgui() override;
    MangoEngine::Result quit() override;

private:
    World world;
};
