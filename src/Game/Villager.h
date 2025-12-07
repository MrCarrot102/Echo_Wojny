#pragma once 
#include <glm/glm.hpp>
#include <string> 
#include <vector> 

#include "Game/ResourceNode.h"


class GameState; 
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
    
    // cel 
    glm::vec2 targetPosition; 
    std::vector<glm::vec2> currentPath; 
    int currentPathIndex; 

    // statystyki
    ResourceNode* targetNode; // wskaznik na zasob, przy ktorym sie pracuje 
    float workTimer; // zegar odliczania pracy 
    float hunger; 
    float thirst; 
   

    ResourceNode::Type carryingResourceType; 
    int carryingAmount; 

    
    // konstruktor ulatwiajacy tworzenie 
    Villager(const std::string& n, const glm::vec2& pos);


    void update(float deltaTime, GameState& world); 

private: 
    // funkcje pomocnicze (mozg) 
    void updateNeeds(float deltaTime); 
    void think(GameState& world);  
    void act(float deltaTime, GameState& world); 

    bool moveOnPath(float deltaTime, float reachDistance, float speed, GameState& world); 

};

