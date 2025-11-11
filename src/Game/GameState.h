#pragma once 
#include <vector> 

#include "Game/Villager.h"
#include "Game/ResourceNode.h"     

class GameState {
    public: 
        GameState(); 

        // glowna funkcja ticki symulacji 
        void update(float deltaTime); 

        // dane gry 
        int dayCounter;  
        float timeOfDay;  
        int globalFood; 
        int globalWood; 
        int globalWater; 

        // npc 
        std::vector<Villager> m_villagers; 
        std::vector<ResourceNode> m_resourceNodes; 

        // gettery 
        const std::vector<Villager>& getVillagers() const {return m_villagers; }
        const std::vector<ResourceNode>& getResourceNodes() const {return m_resourceNodes; }

    private: 
        // liczenie czasu 
        float m_TimeAccumulator; 

};

