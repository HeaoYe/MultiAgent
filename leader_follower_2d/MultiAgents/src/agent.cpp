#include "agent.hpp"
#include "world.hpp"

Agent::Agent(std::string name, const Agent::State &initial_state) : name(name), state(initial_state), back_state(initial_state) {};

void Agent::update(World &world, float dt) {
    bool found;
    for (const auto &last_neighbor_name : last_neighbors) {
        found = false;
        for (const auto &neighbor_name : neighbors) {
            if (last_neighbor_name == neighbor_name) {
                found = true;
                break;
            }
        }
        if (!found) {
            // Lose Neighbor
            if (neighbors.empty()) {
                // Destroy Team
                ctrl.has_team = false;
                ctrl.team = { 0, 0, "", { 0, 0, 0, 0 } };
            } else {
                // Change Team
                auto *leader_data = new QueryLeaderData { false, { name } };
                for (const auto &neighbor_name : neighbors) {
                    found = false;
                    for (const auto &asked_name : leader_data->asked) {
                        if (asked_name == neighbor_name) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        world.send_event<void>(AgentEventType::QueryLeader, neighbor_name, leader_data);
                    }
                    if (leader_data->found) {
                        break;
                    }
                }

                if ((!leader_data->found) && (ctrl.team.leader != name)) {
                    auto *data = new ChangeTeamData { ctrl.team, { static_cast<uuid_t>(rand()), 1, name, glm::vec4 { rand() % 200, rand() % 200, rand() % 200, 255 } / 255.0f }, {}, 1 };
                    for (const auto &neighbor_name : neighbors) {
                        found = false;
                        for (const auto &changed_name : data->changed) {
                            if (neighbor_name == changed_name) {
                                found = true;
                                break;
                            }
                        }
                        if (!found) {
                            world.send_event<void>(AgentEventType::ChangeTeam, neighbor_name, data);
                        }
                    }
                    delete data;
                }
                delete leader_data;
            }
            ctrl.neighbor_info.erase(last_neighbor_name);
        }
    }

    for (const auto &neighbor_name : neighbors) {
        found = false;
        for (const auto &last_neighbor_name : last_neighbors) {
            if (neighbor_name == last_neighbor_name) {
                found = true;
                break;
            }
        }
        auto *reply = world.send_event<QueryTeamReply>(AgentEventType::QueryTeam, neighbor_name);
        if (!found) {
            // Link Neighbor
            if (ctrl.has_team) {
                if (reply->has_team) {
                    // Merge Team
                    if (reply->team.id != ctrl.team.id && ((reply->team.member_num < ctrl.team.member_num) || (reply->team.member_num == ctrl.team.member_num && uuid > reply->uuid))) {
                        auto *data = new ChangeTeamData { reply->team, ctrl.team, {}, ctrl.team.member_num };
                        world.send_event<void>(AgentEventType::ChangeTeam, neighbor_name, data);
                        ctrl.team.member_num = data->member_num;
                        delete data;
                    }
                } else {
                    // Invite To Team
                    auto *data = world.send_event<InviteToTeamData>(AgentEventType::InviteToTeam, neighbor_name, new InviteToTeamData { ctrl.team });
                    ctrl.team.member_num = data->team.member_num;
                    delete data;
                }
            } else {
                if (reply->has_team) {
                    // Join Team
                    ctrl.has_team = true;
                    ctrl.team = reply->team;
                    ctrl.team.member_num += 1;
                } else {
                    // Create Team
                    ctrl.has_team = true;
                    ctrl.team = { static_cast<uuid_t>(rand()), 1, name, glm::vec4 { rand() % 200, rand() % 200, rand() % 200, 255 } / 255.0f };
                    auto *data = world.send_event<InviteToTeamData>(AgentEventType::InviteToTeam, neighbor_name, new InviteToTeamData { ctrl.team });
                    ctrl.team.member_num = data->team.member_num;
                    delete data;
                }
            }
        }

        delete reply;
        reply = world.send_event<QueryTeamReply>(AgentEventType::QueryTeam, neighbor_name);
        ctrl.neighbor_info[neighbor_name] = *reply;
        delete reply;
    }

    if (ctrl.team.leader == name) {
        // Leader TODO
        auto *data = new LocateData { ctrl.team.id, { 0, 0 }, ctrl.team.member_num, {} };
        handle_event(world, AgentEventType::Locate, data);
        std::sort(data->members.begin(), data->members.end());
        ctrl.team.member_num = data->members.size();
       
        auto *tasks = new TaskData { ctrl.team.id };
        tasks->dst_rels_vs[uuid] = { { 0, 0 }, { 0, 0 } };
        
        int w = glm::ceil(glm::sqrt(data->members.size() - 1)), i = 0;
        for (uuid_t mem_uuid : data->members) {
            if (mem_uuid != uuid) {
                tasks->dst_rels_vs[mem_uuid] = { { (i % w - (w - 1) / 2.0f) * 50, -(i / w + 1) * 50 }, back_state.v };
                i += 1;
            }
        }
        handle_event(world, AgentEventType::SetTask, tasks);
        delete data;
        delete tasks;
    }

    if (neighbors.empty()) {
        state.u = -beta * back_state.v;
    } else {
        state.u = { 0, 0 };
        state.u += alpha * (ctrl.dst_rel - ctrl.rel) + beta * (ctrl.dst_v - back_state.v);
    }
}

void *Agent::handle_event(World &world, AgentEventType type, void *data) {
    switch (type) {
    case AgentEventType::QueryState:
        return new QueryStateReply { back_state.x, back_state.v };
    case AgentEventType::QueryTeam:
        return new QueryTeamReply { ctrl.has_team, uuid, ctrl.team };
    case AgentEventType::InviteToTeam: {
        auto *info = reinterpret_cast<InviteToTeamData *>(data);
        ctrl.has_team = true;
        info->team.member_num += 1;
        ctrl.team = info->team;
        return data;
    }
    case AgentEventType::ChangeTeam: {
        auto *info = reinterpret_cast<ChangeTeamData *>(data);
        info->changed.push_back(name);
        if (ctrl.has_team && ctrl.team.id == info->src_team.id) {
            info->member_num += 1;
            info->dst_team.member_num += 1;
            ctrl.team = info->dst_team;
        }
        bool found;
        for (const auto &neighbor_name : neighbors) {
            found = false;
            for (const auto &changed_name : info->changed) {
                if (neighbor_name == changed_name) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                world.send_event<void>(AgentEventType::ChangeTeam, neighbor_name, data);
            }
        }
        return nullptr;
    }
    case AgentEventType::QueryLeader: {
        auto *info = reinterpret_cast<QueryLeaderData *>(data);
        info->asked.push_back(name);
        if (ctrl.team.leader == name) {
            info->found = true;
            return nullptr;
        }
        bool found;
        for (const auto &neighbor_name : neighbors) {
            found = false;
            for (const auto &asked_name : info->asked) {
                if (neighbor_name == asked_name) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                world.send_event<void>(AgentEventType::QueryLeader, neighbor_name, data);
            }
        }
        return nullptr;
    }
    case AgentEventType::Locate: {
        auto *info = reinterpret_cast<LocateData *>(data);
        if ((!ctrl.has_team) || info->team_id != ctrl.team.id) {
            return nullptr;
        }
        ctrl.rel = info->rel;
        info->asked.push_back(name);
        info->members.push_back(uuid);
        ctrl.team.member_num = info->member_num;
        bool found;
        for (const auto &neighbor_name : neighbors) {
            found = false;
            for (const auto &asked_name : info->asked) {
                if (neighbor_name == asked_name) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                info->rel = ctrl.rel + deltas[neighbor_name];
                world.send_event<void>(AgentEventType::Locate, neighbor_name, data);
            }
        }
        return nullptr;
    }
    case AgentEventType::SetTask: {
        auto *info = reinterpret_cast<TaskData *>(data);
        if ((!ctrl.has_team) || info->team_id != ctrl.team.id) {
            return nullptr;
        }
        ctrl.dst_rel = info->dst_rels_vs[uuid].first;
        ctrl.dst_v = info->dst_rels_vs[uuid].second;
        info->dst_rels_vs.erase(uuid);
        for (const auto &neighbor_name : neighbors) {
            for (const auto &[not_asked_uuid, pair] : info->dst_rels_vs) {
                if (ctrl.neighbor_info[neighbor_name].uuid == not_asked_uuid) {
                    world.send_event<void>(AgentEventType::SetTask, neighbor_name, data);
                    break;
                }
            }
        }
        return nullptr;
    }
    default:
        return nullptr;
    }
}
