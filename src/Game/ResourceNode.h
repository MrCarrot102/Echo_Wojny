#pragma once 
#include <glm/glm.hpp> 

#include "Game/Villager.h"


struct ResourceNode {

    enum Type { NONE, 
                TREE, 
                ROCK, 
                BERRY_BUSH }; 
                
    Type resourceType; 
    glm::vec2 position; 
    int amountLeft; 

    ResourceNode(Type type, const glm::vec2& pos, int amount)
        : resourceType(type), position(pos), amountLeft(amount) {}
};

