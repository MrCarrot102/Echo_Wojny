#pragma once 
#include <glm/glm.hpp>


struct Building {
    glm::vec2 position;; 

    enum Type { TENT, 
                KITCHEN, 
                WELL,
                STOCKPILE,
                CAMPFIRE
            };
             
    Type buildingType; 

    Building(Type type, const glm::vec2& pos)
            : buildingType(type), position(pos) {}

};