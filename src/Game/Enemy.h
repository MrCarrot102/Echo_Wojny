#pragma once 
#include <glm/glm.hpp> 
#include <vector> 


struct Enemy
{
    glm::vec2 position; 

    enum State 
    {
        PATROL,
        CHASE
    };
    State currentState; 

    glm::vec2 targetPosition; 
    std::vector<glm::vec2> currentPath; 
    int currentPathIndex; 

    float speed; 
    float aggroRange; 

    Enemy(glm::vec2 pos)
        : position(pos), 
        currentState(State::PATROL),
        targetPosition(pos), 
        currentPathIndex(0),
        speed(70.0f), 
        aggroRange(300.0f)
    {}
};