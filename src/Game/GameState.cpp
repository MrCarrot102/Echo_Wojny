#include "Game/GameState.h"
#include <iostream> 

GameState::GameState() 
    : dayCounter(1),
      timeOfDay(6.0f), // zaczynanie dnia od 6 :( 
      globalFood(100),
      globalWood(50),
      globalWater(50),
      m_TimeAccumulator(0.0f)
{
    // testowanie npc 
    m_villagers.emplace_back("Adam", glm::vec2(100.0f, 100.0f));
    m_villagers.emplace_back("Ewa", glm::vec2(120.0f, 100.0f));
    m_villagers.emplace_back("Radek", glm::vec2(100.0f, 120.0f));

    // obiekty 
    m_resourceNodes.emplace_back(ResourceNode::TREE, glm::vec2(200.0f, 200.0f), 100);
    m_resourceNodes.emplace_back(ResourceNode::TREE, glm::vec2(250.0f, 210.0f), 100);
    m_resourceNodes.emplace_back(ResourceNode::ROCK, glm::vec2(300.0f, 100.0f), 200);
}

void GameState::update(float deltaTime){

    // Symulacja uplywu czasu 
    // skala 1 sekunda 1 godzina 
    float gameHoursPassed = deltaTime * 1.0f; 
    timeOfDay += gameHoursPassed; 
    m_TimeAccumulator += deltaTime; 

    // logika nowego dnia 
    if(timeOfDay >= 24.0f){
        dayCounter++; 
        timeOfDay = 0.0f; // resetowanie zegara 

        // teoretyczne zuzywanie zasobow 
        globalFood -= m_villagers.size() * 10;  // jeden mieszkaniec zjada 10 jednostek jedzenia dziennie 
        std::cout << "\n--- Nowy dzien " << dayCounter << " ----" << std::endl; 
    }


    // debugowanie w konsoli 
    // drukowanie co jakis czas zeby nie bylo co klatke dla czytelnosci 
    if (m_TimeAccumulator > 0.5f) {
        m_TimeAccumulator = 0.0f; 

        std::cout << "Dzień: " << dayCounter 
                    << " | Godzina: " << (int)timeOfDay
                    << " | Mieszkańcy: " << m_villagers.size()
                    << " | Jedzenie: " << globalFood
                    << " \r"; // czyszczenie starych znakow
        std::cout.flush(); // wypisywanie bufora 
    }
}