#pragma once 
#include <glm/glm.hpp> 
#include "Game/Villager.h"


struct ResourceNode {
    glm::vec2 position; 

    enum Type { TREE, ROCK, FOOD_BUSH }; 
    Type resourceType; 

    int amountLeft; 

    ResourceNode(Type type, const glm::vec2& pos, int amount)
        : resourceType(type), position(pos), amountLeft(amount) {}
};

