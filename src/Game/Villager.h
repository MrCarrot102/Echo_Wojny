#pragma once 
#include <glm/glm.hpp>
#include <string> 
#include <vector> 


#include "Game/ResourceNode.h"

struct ResourceNode; 

// szkielet mieszkanca to tylko kontener na dane (chwilowo) 
struct Villager {
    glm::vec2 position; 
    std::string name; 
     
    // stany jejaja 
    enum State { 
        IDLE, // bezczynnosc 
        MOVING_TO_POINT, // idzie do jakiegos losowego punktu 
        MOVING_TO_WORK, // idzie to zasobu 
        GATHERING, // zbiera
        MOVING_TO_EAT, // chce mu sie zrec 
        EATING,  // zre cos
        MOVING_TO_DRINK, 
        DRINKING,
        MOVING_TO_HAUL, 
        HAULING  
    }; 
    
    
    
    State currentState; 
    
    float workTimer; // zegar odliczania pracy 
    float hunger; 
    float thirst; 
    // cel 
    glm::vec2 targetPosition; 
    std::vector<glm::vec2> currentPath; 
    int currentPathIndex;  

    ResourceNode::Type carryingResourceType; 
    int carryingAmount; 

    // statystyki
    ResourceNode* targetNode; // wskaznik na zasob, przy ktorym sie pracuje 
    
    // konstruktor ulatwiajacy tworzenie 
    Villager(const std::string& n, const glm::vec2& pos)
        : name(n), 
        position(pos), 
        currentState(State::IDLE), 
        targetPosition(pos), 
        targetNode(nullptr),
        workTimer(0.0f),
        hunger(100.0f),
        thirst(100.0f),
        carryingResourceType(ResourceNode::Type::NONE),
        carryingAmount(0),
        currentPathIndex(0)
        {}
};

