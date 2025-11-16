#pragma once 
#include <vector> 

#include "Game/Villager.h"
#include "Game/ResourceNode.h"     
#include "Game/Building.h"


class GameState {
    public: 
        GameState(); 

        // glowna funkcja ticki symulacji 
        void update(float deltaTime); 
        void checkForDailyEvents(); 
        
        // dane gry 
        int dayCounter;  
        float timeOfDay;  
        int globalFood; 
        int globalWood; 
        int globalWater; 

        // npc 
        std::vector<Villager> m_villagers; 
        std::vector<ResourceNode> m_resourceNodes; 
        std::vector<Building> m_buildings; 


        // gettery 
        //const std::vector<Villager>& getVillagers() const {return m_villagers; }
        //const std::vector<ResourceNode>& getResourceNodes() const {return m_resourceNodes; }

    private: 
        // liczenie czasu 
        float m_TimeAccumulator; 
        
        // wydarzenia
        bool m_eventDay3Triggered; 
        bool m_eventDay5Triggered; 

};

