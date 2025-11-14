#pragma once 
#include <glm/glm.hpp>
#include <string> 

struct ResourceNode; 

// szkielet mieszkanca to tylko kontener na dane (chwilowo) 
struct Villager {
    glm::vec2 position; 
    std::string name; 
    int hunger; 
    // stany jejaja 
    enum State { 
        IDLE, // bezczynnosc 
        MOVING_TO_POINT, // idzie do jakiegos losowego punktu 
        MOVING_TO_WORK, // idzie to zasobu 
        GATHERING // zbiera 
    }; 
    
    
    
    State currentState; 

    // cel 
    glm::vec2 targetPosition; 

    // statystyki
    ResourceNode* targetNode; // wskaznik na zasob, przy ktorym sie pracuje 
    float workTimer; // zegar odliczania pracy 

    // konstruktor ulatwiajacy tworzenie 
    Villager(const std::string& n, const glm::vec2& pos)
        : name(n), 
        position(pos), 
        currentState(State::IDLE), 
        targetPosition(pos), 
        targetNode(nullptr),
        workTimer(0.0f),
        hunger(100){}
};

