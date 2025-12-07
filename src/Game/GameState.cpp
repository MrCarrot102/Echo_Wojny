#include "Game/GameState.h"
#include "Game/Pathfinder.h"
#include "Game/Villager.h"

#include <iostream> 
#include <algorithm>

GameState::GameState() 
    : dayCounter(1),
      timeOfDay(6.0f),
      globalFood(100),
      globalWood(50),
      globalWater(50),
      m_TimeAccumulator(0.0f),
      m_eventDay3Triggered(false),
      m_eventDay5Triggered(false),
      m_eventRefugeesTriggered(false),
      m_currentMode(Mode::PLAYING)
{
    // Inicjalizacja Mapy
    m_worldMap = std::make_unique<WorldMap>(100, 100, 20.0f);

    // Mieszkańcy
    m_villagers.emplace_back("Adam", glm::vec2(100.0f, 100.0f));
    m_villagers.emplace_back("Ewa", glm::vec2(120.0f, 100.0f));
    m_villagers.emplace_back("Radek", glm::vec2(100.0f, 120.0f));

    // Zasoby
    m_resourceNodes.emplace_back(ResourceNode::TREE, glm::vec2(200.0f, 200.0f), 100);
    m_resourceNodes.emplace_back(ResourceNode::TREE, glm::vec2(250.0f, 210.0f), 100);
    m_resourceNodes.emplace_back(ResourceNode::ROCK, glm::vec2(300.0f, 100.0f), 200);

    // Budynki startowe
    m_buildings.emplace_back(Building::KITCHEN, glm::vec2(150.0f, 150.0f));
    /*
    for(const auto& b : m_buildings) {
        glm::ivec2 gridPos = m_worldMap->worldToGrid(b.position);
        m_worldMap->setObstacle(gridPos.x, gridPos.y, true);
    }*/
}

void GameState::update(float deltaTime) {
    
    if (m_currentMode != Mode::PLAYING) return;

    // 1. Czas globalny
    float gameHoursPassed = deltaTime * 1.0f; 
    timeOfDay += gameHoursPassed; 
    m_TimeAccumulator += deltaTime; 

    // 2. Aktualizacja wszystkich mieszkańców (DELEGACJA do Villager.cpp)
    for (Villager& villager : m_villagers) {
        villager.update(deltaTime, *this);
    }

    // 3. Logika Dnia
    if(timeOfDay >= 24.0f){
        dayCounter++; 
        timeOfDay = 0.0f; 
        std::cout << "\n--- Dzien " << dayCounter << " ---\n"; 
        checkForDailyEvents(); 
    }

    // 4. Debug
    if (m_TimeAccumulator > 0.5f) {
        m_TimeAccumulator = 0.0f; 
        std::cout << "D: " << dayCounter 
                  << " | Drewno: " << globalWood
                  << " | Populacja: " << m_villagers.size() 
                  << " \r";
        std::cout.flush(); 
    }
}


Building* GameState::findNearestStockpile(const glm::vec2& fromPos) {
    Building* closest = nullptr;
    float minDst = 100000.0f;
    for (Building& b : m_buildings) {
        if (b.buildingType == Building::STOCKPILE) {
            float dst = glm::distance(fromPos, b.position);
            if (dst < minDst) { minDst = dst; closest = &b; }
        }
    }
    return closest;
}

void GameState::checkForDailyEvents() {
    if (dayCounter == 3 && !m_eventDay3Triggered) {
        m_eventDay3Triggered = true;
        currentEventTitle = "Zrzut zapasow";
        currentEventDescription = "Zysk: +50 Jedzenie, +20 Woda.";
        globalFood += 50;
        globalWater += 20;
    }

    if (dayCounter == 5 && !m_eventDay5Triggered){
        m_eventDay5Triggered = true;
        currentEventTitle = "Najazd na baze";
        currentEventDescription = "Stracilismy: -30 Jedzenia, -40 Drewna";
        globalFood -= 30;
        globalWood -= 40;
        if (globalFood < 0 ) globalFood = 0;
        if (globalWood < 0 ) globalWood = 0;
    }

    if (dayCounter == 7 && !m_eventRefugeesTriggered) {
        m_eventRefugeesTriggered = true;
        currentEventTitle = "Uchodzcy";
        currentEventDescription = "Chca jesc.\nKoszt: -30 Jedzenia\nZysk: +2 Mieszkancow";
        setMode(Mode::EVENT_PAUSED);
    }
}

void GameState::resolveRefugeeEvent(bool accepted) {
    if (accepted) {
        glm::vec2 spawnPos = {100.0f, 150.0f};
        m_villagers.emplace_back("Uchodzca", spawnPos);
        m_villagers.emplace_back("Uchodzca", spawnPos + glm::vec2(5.0f, 5.0f));
        globalFood -= 30;
    }
    setMode(Mode::PLAYING);
}

void GameState::setMode(Mode newMode) {
    m_currentMode = newMode;
}