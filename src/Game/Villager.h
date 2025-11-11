#pragma once 
#include <glm/glm.hpp>
#include <string> 

// szkielet mieszkanca to tylko kontener na dane (chwilowo) 
struct Villager {
    glm::vec2 position; 
    std::string name; 

    // stany jejaja 
    enum State { IDLE, WORKING, EATING }; 
    State currentState; 

    // statystyki
    int hunger; 

    // konstruktor ulatwiajacy tworzenie 
    Villager(const std::string& n, const glm::vec2& pos)
        : name(n), position(pos), currentState(State::IDLE), hunger(100) {}
};

