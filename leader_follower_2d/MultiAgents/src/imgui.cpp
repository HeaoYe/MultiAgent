#include "world.hpp"
#include <MangoEngine/MangoEngine.hpp>

void World::draw_ui() {
    if (ImGui::Button("Reset")) {
        reset();
    }
    ImGui::Checkbox("Pause", &is_paused);
    ImGui::Checkbox("Follow Mouse", &is_follow_mouse);
    ImGui::Checkbox("Mouse Picker", &is_mouse_drag);
    ImGui::Checkbox("Highlight Selected Agent", &is_highlight);

    const char *items[agents.size()];
    int i = 0;
    std::for_each(agents.begin(), agents.end(), [&items, &i] (Agent &agent) {
        items[i++] = agent.name.c_str();
    });
    ImGui::ListBox("Selector", &selector, items, agents.size(), 3);

    ImGui::Separator();
    auto &agent = agents[selector];

    ImGui::Text("name=%s", agent.name.c_str());
    ImGui::Text("has_team=%d, team=%d, leader=%s", agent.ctrl.has_team, agent.ctrl.team.id, agent.ctrl.team.leader.c_str());
    ImGui::Text("member_num=%d", agent.ctrl.team.member_num);
    ImGui::Text("relx=%f, rely=%f", agent.ctrl.rel.x, agent.ctrl.rel.y);

    ImGui::Separator();

    ImGui::SliderFloat("Alpha", &agent.alpha, 0, 5);
    ImGui::SliderFloat("Beta", &agent.beta, 0, 5);
    ImGui::SliderFloat2("Pos", &agent.state.x.x, -640, 640);
    ImGui::SliderFloat2("Speed", &agent.state.v.x, -100, 100);
    ImGui::SliderFloat2("Control", &agent.state.v.x, -100, 100);
}
