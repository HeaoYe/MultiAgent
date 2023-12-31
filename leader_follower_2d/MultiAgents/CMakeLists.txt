project(MultiAgents)

file(GLOB_RECURSE SRC src/*.cpp)
add_executable(MultiAgents ${SRC})
target_link_libraries(MultiAgents PUBLIC MangoEngine)
