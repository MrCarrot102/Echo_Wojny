#pragma once 
#include <glm/glm.hpp>
#include <string> 

// szkielet mieszkanca to tylko kontener na dane (chwilowo) 
struct Villager {
    glm::vec2 position; 
    std::string name; 

    // stany jejaja 
    enum State { IDLE, WORKING, EATING, MOVING}; 
    State currentState; 

    // cel 
    glm::vec2 targetPosition; 

    // statystyki
    int hunger; 

    // konstruktor ulatwiajacy tworzenie 
    Villager(const std::string& n, const glm::vec2& pos)
        : name(n), 
        position(pos), 
        currentState(State::IDLE), 
        targetPosition(pos), 
        hunger(100) {}
};

