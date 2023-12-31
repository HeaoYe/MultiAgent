project(MultiAgentTeam)

file(GLOB_RECURSE SRC src/*.cpp)
add_executable(MultiAgentTeam ${SRC})
target_link_libraries(MultiAgentTeam PUBLIC MangoEngine)
