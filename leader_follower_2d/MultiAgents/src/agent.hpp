#pragma once

#include <string>
#include <vector>
#include <map>
#include <glm/glm.hpp>

using T = glm::vec2;
using uuid_t = unsigned int;

class World;
enum class AgentEventType;
struct QueryTeamReply;

class Agent {
    friend World;
    static constexpr float display_size = 35;
public:
    struct Team {
        uuid_t id = 0;
        int member_num = 0;
        std::string leader;
        glm::vec4 color = { 0, 0, 0, 0 };
    };
private:
    // float alpha = 2.5f, beta = 4.0f;
    float alpha = 12.0f, beta = 8.0f;
    struct State {
        T x {}, v {}, u {};
    } state, back_state;

    std::string name;
    uuid_t uuid;
    glm::vec4 color;

    std::vector<std::string> neighbors;  // Auto set by World per tick
    std::vector<std::string> last_neighbors;  // Auto set by World per tick
    std::map<std::string, glm::vec2> deltas;  // this->other  Auto set by World per tick
    struct ControlState {
        bool has_team = false;
        Agent::Team team;
        glm::vec2 rel;
        glm::vec2 dst_rel;
        glm::vec2 dst_v;
        std::map<std::string, QueryTeamReply> neighbor_info;
    } ctrl;
public:
    Agent(std::string name, const Agent::State &initial_state);
    void update(World &world, float dt);
    void *handle_event(World &world, AgentEventType type, void *data);
};

enum class AgentEventType {
    QueryState, QueryTeam, InviteToTeam, ChangeTeam, QueryLeader, Locate, SetTask,
};

struct QueryStateReply {
    glm::vec2 x, v;
};

struct QueryTeamReply {
    bool has_team;
    uuid_t uuid;
    Agent::Team team;
};

struct InviteToTeamData {
    Agent::Team team;
};

struct ChangeTeamData {
    Agent::Team src_team, dst_team;
    std::vector<std::string> changed;
    int member_num;
};

struct QueryLeaderData {
    bool found;
    std::vector<std::string> asked;
};

struct LocateData {
    uuid_t team_id;
    glm::vec2 rel;
    int member_num;
    std::vector<std::string> asked;
    std::vector<uuid_t> members;
};

struct TaskData {
    uuid_t team_id;
    std::map<uuid_t, std::pair<glm::vec2, glm::vec2>> dst_rels_vs;
};
