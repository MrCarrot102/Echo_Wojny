#include "Game/GameState.h"
#include "Game/Pathfinder.h"

#include <iostream> 

GameState::GameState() 
    : dayCounter(1),
      timeOfDay(6.0f), // zaczynanie dnia od 6 :( 
      globalFood(100),
      currentSeason(Season::SUMMER),
      gloablTemperature(25.0f), 
      seasonTimer(0.0f), 
      heatingTimer(0.0f), 
      globalWood(50),
      globalWater(50),
      m_TimeAccumulator(0.0f),
      m_eventDay3Triggered(false), 
      m_eventDay5Triggered(false),
      m_currentMode(Mode::PLAYING),
      m_eventRefugeesTriggered(false),
      m_worldMap(std::make_unique<WorldMap>(100, 100, 20.0f))
      
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

void GameState::update(float deltaTime) {

    if (m_currentMode != Mode::PLAYING) return;

    // --- 1. Parametry Symulacji ---
    float gameHoursPassed = deltaTime * 1.0f; 
    timeOfDay += gameHoursPassed; 
    m_TimeAccumulator += deltaTime; 
    
    float speed = 60.0f; 
    const float GATHER_TIME = 3.0f; 
    
    const float HUNGER_RATE = 2.0f; 
    const float THIRST_RATE = 3.0f; 
    const float CRITICAL_THIRST = 40.0f;
    const float CRITICAL_HUNGER = 30.0f;


    // --- system pogody i ogrzewania --- 
    // cykl por roku zmiana co 3 dni 
    int seasonCycle = (dayCounter - 1) / 3; 

    if (seasonCycle % 2 == 0) 
    { 
        currentSeason = Season::SUMMER; 
        gloablTemperature = 25.0f; 
    }
    else 
    {
        currentSeason = Season::WINTER; 
        gloablTemperature = -10.0f; 
    }

    // mechanika wplywu temperatury na zycie 
    if (currentSeason == Season::WINTER) 
    {
        heatingTimer += deltaTime; 

        if (heatingTimer > 2.0f) 
        {
            heatingTimer = 0.0f; 

            // sprawdzanie czy istnieje ognisko 
            bool hasCampfire = false; 
            for (const auto& b : m_buildings) 
            {
                if (b.buildingType == Building::CAMPFIRE)
                {
                    hasCampfire = true; 
                    break; 
                }
            }

            if (hasCampfire)
            {
                if (globalWood >= 5)
                {
                    globalWood -= 5; 
                }
                else
                {
                    std::cout << "[ALARM] Ognisko zgasło! Brak drewna! Ludzie marzną!\n";
                    if (!m_villagers.empty())
                    {
                        m_villagers.pop_back(); 
                        std::cout << "[ŚMIERĆ] Mieszkaniec zamarzł!\n";
                    }
                }
            }
            else 
            {
                // kiedy w ogole nie ma ogniska 
                std::cout << "[ALARM] Brak ogniska w zimie! Too wyrok śmierci!\n";
                if (globalWood >= 10) 
                {
                    globalWood -= 10; // probowanie sie ogrzac w mniej wydajny sposob 
                }
                else 
                { 
                    if (!m_villagers.empty())
                    {
                        m_villagers.pop_back(); 
                        std::cout << "[ŚMIERĆ] Mieszkaniec zamarzł z braku ogniska!\n";
                    }
                }
            }
        }
    }


    // --- 2. Pętla Mieszkańców ---
    for (Villager& villager : m_villagers) {
        
        // Symulacja potrzeb
        villager.hunger -= (gameHoursPassed * HUNGER_RATE);
        villager.thirst -= (gameHoursPassed * THIRST_RATE);
        if (villager.hunger < 0.0f) villager.hunger = 0.0f;
        if (villager.thirst < 0.0f) villager.thirst = 0.0f;

        // --- PRIORYTETY ---

        // 1. WODA
        if (villager.thirst < CRITICAL_THIRST && 
            villager.currentState != Villager::State::DRINKING && 
            villager.currentState != Villager::State::MOVING_TO_DRINK) 
        {
            Building* targetWell = nullptr;
            for (Building& b : m_buildings) {
                if (b.buildingType == Building::WELL) { targetWell = &b; break; }
            }
            
            if (targetWell) {
                std::cout << "\n[AI] " << villager.name << " idzie pić.\n";
                villager.currentState = Villager::State::MOVING_TO_DRINK;
                villager.targetPosition = targetWell->position;
                villager.currentPath.clear();
            }
        }
        // 2. GŁÓD
        else if (villager.hunger < CRITICAL_HUNGER && 
                 villager.currentState != Villager::State::EATING && 
                 villager.currentState != Villager::State::MOVING_TO_EAT)
        {
            Building* targetKitchen = nullptr;
            for (Building& b : m_buildings) {
                if (b.buildingType == Building::KITCHEN) { targetKitchen = &b; break; }
            }

            if (targetKitchen) {
                std::cout << "\n[AI] " << villager.name << " idzie jeść.\n";
                villager.currentState = Villager::State::MOVING_TO_EAT;
                villager.targetPosition = targetKitchen->position;
                villager.currentPath.clear();
            }
        }

        // --- Funkcja Pomocnicza Ruchu (Lambda) ---
        auto moveOnPath = [&](float reachDistance) -> bool {
            
            // A. Jeśli brak ścieżki - oblicz
            if (villager.currentPath.empty()) {
                // Optymalizacja: Jeśli blisko, idź prosto
                float directDist = glm::distance(villager.position, villager.targetPosition);
                if (directDist < 20.0f) {
                    glm::vec2 dir = villager.targetPosition - villager.position;
                    if (glm::length(dir) > 0.1f) 
                        villager.position += glm::normalize(dir) * speed * deltaTime;
                    
                    if (directDist <= reachDistance) return true;
                    return false;
                }

                // A* Pathfinder
                villager.currentPath = Pathfinder::findPath(villager.position, villager.targetPosition, *m_worldMap);
                villager.currentPathIndex = 0;
                
                if (villager.currentPath.empty()) {
                    villager.currentState = Villager::State::IDLE;
                    return false;
                }
            }

            // B. Poruszanie po ścieżce
            if (villager.currentPathIndex < villager.currentPath.size()) {
                glm::vec2 nextWaypoint = villager.currentPath[villager.currentPathIndex];
                glm::vec2 direction = nextWaypoint - villager.position;
                float distToWaypoint = glm::length(direction);

                if (distToWaypoint > 2.0f) {
                    villager.position += glm::normalize(direction) * speed * deltaTime;
                } else {
                    villager.currentPathIndex++;
                }
                return false; 
            } else {
                // C. Końcówka trasy
                glm::vec2 direction = villager.targetPosition - villager.position;
                float distToTarget = glm::length(direction);

                if (distToTarget > reachDistance) {
                    villager.position += glm::normalize(direction) * speed * deltaTime;
                    return false;
                } else {
                    villager.currentPath.clear();
                    villager.currentPathIndex = 0;
                    return true; // Dotarł!
                }
            }
            return false; // Bezpiecznik
        }; 


        // --- Maszyna Stanów ---

        if (villager.currentState == Villager::State::MOVING_TO_POINT) {
            if (moveOnPath(1.0f)) {
                villager.currentState = Villager::State::IDLE;
            }
        }
        else if (villager.currentState == Villager::State::MOVING_TO_WORK) {
            if (!villager.targetNode) { 
                villager.currentState = Villager::State::IDLE; continue; 
            }
            if (moveOnPath(10.0f)) {
                villager.currentState = Villager::State::GATHERING;
                villager.workTimer = GATHER_TIME;
            }
        }
        else if (villager.currentState == Villager::State::MOVING_TO_EAT) {
            if (moveOnPath(10.0f)) {
                villager.currentState = Villager::State::EATING;
                villager.workTimer = 2.0f;
            }
        }
        else if (villager.currentState == Villager::State::MOVING_TO_DRINK) {
            if (moveOnPath(10.0f)) {
                villager.currentState = Villager::State::DRINKING;
                villager.workTimer = 2.0f;
            }
        }
        else if (villager.currentState == Villager::State::MOVING_TO_HAUL) {
            if (moveOnPath(10.0f)) {
                std::cout << "\n[AI] " << villager.name << " zrzucił zasoby.\n";
                if (villager.carryingResourceType == ResourceNode::Type::TREE) globalWood += villager.carryingAmount;
                
                villager.carryingAmount = 0;
                villager.carryingResourceType = ResourceNode::Type::NONE; // Reset typu

                if (villager.targetNode && villager.targetNode->amountLeft > 0) {
                    villager.currentState = Villager::State::MOVING_TO_WORK;
                    villager.targetPosition = villager.targetNode->position;
                    villager.currentPath.clear();
                } else {
                    villager.currentState = Villager::State::IDLE;
                }
            }
        }
        
        // --- Stany Akcji ---
        
        else if (villager.currentState == Villager::State::GATHERING) {
            if (!villager.targetNode) { villager.currentState = Villager::State::IDLE; continue; }
            villager.workTimer -= deltaTime;
            
            if (villager.workTimer <= 0.0f) {
                villager.carryingAmount = 10;
                villager.carryingResourceType = villager.targetNode->resourceType;
                villager.targetNode->amountLeft -= 10;
                if (villager.targetNode->amountLeft <= 0) villager.targetNode = nullptr;

                Building* stockpile = findNearestStockpile(villager.position);
                if (stockpile) {
                    villager.currentState = Villager::State::MOVING_TO_HAUL;
                    villager.targetPosition = stockpile->position;
                    villager.currentPath.clear();
                } else {
                    villager.carryingAmount = 0;
                    villager.currentState = Villager::State::IDLE;
                }
            }
        }
        else if (villager.currentState == Villager::State::EATING) {
            villager.workTimer -= deltaTime;
            if (villager.workTimer <= 0.0f) {
                if (globalFood >= 10) { globalFood -= 10; villager.hunger = 100.0f; }
                villager.currentState = Villager::State::IDLE;
            }
        }
        else if (villager.currentState == Villager::State::DRINKING) {
            villager.workTimer -= deltaTime;
            if (villager.workTimer <= 0.0f) {
                if (globalWater >= 10) { globalWater -= 10; villager.thirst = 100.0f; }
                villager.currentState = Villager::State::IDLE;
            }
        }

    } // Koniec pętli for

    // --- Logika Dnia ---
    if(timeOfDay >= 24.0f){
        dayCounter++; 
        timeOfDay = 0.0f; 
        std::cout << "\n--- Dzien " << dayCounter << " ---\n"; 
        checkForDailyEvents(); 
    }

    // --- Debug ---
    if (m_TimeAccumulator > 0.5f) {
        m_TimeAccumulator = 0.0f; 
        std::cout << "D: " << dayCounter 
                  << " | Drewno: " << globalWood
                  << " | Populacja: " << m_villagers.size() 
                  << " \r";
        std::cout.flush(); 
    }
}


void GameState::checkForDailyEvents() {
    
    // wydarzenie 1: zrzuty zaopatrzenia 
    if (dayCounter == 3 && !m_eventDay3Triggered) {
        
        m_eventDay3Triggered = true; 

        // narracja na razie do konsoli 
        currentEventTitle = "Zrzut zapasow";
        currentEventDescription = "W nocy nad nasza wioska przelecial przyjacielski samolot i zrzucil troche zapasow.\nZysk: +50 Jedzenie, +20 Woda.\n";

        globalFood += 50; 
        globalWater += 20; 
    }

    if (dayCounter == 5 && !m_eventDay5Triggered){
        
        m_eventDay5Triggered = true; 

        currentEventTitle = "Najazd na bazę";
        currentEventDescription = "W nocy banda wrogiego wojska napadla na nasza wioske i ukradla nasze zapasy.\n\nStracilismy: -30 Jedzenia, -40 Drewna";
        globalFood -= 30;
        globalWood -= 40; 

        if (globalFood < 0 ) globalFood = 0; 
        if (globalWood < 0 ) globalWood = 0; 
    }

    if (dayCounter == 2 && !m_eventRefugeesTriggered) {
        m_eventRefugeesTriggered = true; 
        
        currentEventTitle = "Uchodzcy u bram";
        currentEventDescription = "Do wioski zbliza sie grupa wyglodnialych cywilow.\n Prosza o schornienie i jedzenie.\n\nKoszt: -30 Jedzenia\nZysk: +2 Mieszkancow";

        setMode(Mode::EVENT_PAUSED); 
        
    }
}


Building* GameState::findNearestStockpile(const glm::vec2& fromPos){
    Building* closestStockpile = nullptr; 
    float minDistanceSquared = 999999.0f * 999999.0f; 
    
    for(Building& b : m_buildings){
        
        if (b.buildingType == Building::Type::STOCKPILE){
            
            float distanceSquared = glm::length(b.position - fromPos);

            if (distanceSquared < minDistanceSquared){
                minDistanceSquared = distanceSquared; 
                closestStockpile = &b; 
            }
        }
    }

    return closestStockpile;
}

void GameState::setMode(Mode newMode){
    m_currentMode = newMode; 
}


void GameState::resolveRefugeeEvent(bool accepted) {
    if (accepted) {

        std::cout << "\n[DECYZJA] 'Nie możemy ich tak zostawić.' Wpuściliście ich.\n";
        std::cout << "[EFEKT] +2 Mieszkańców, -30 Jedzenia (natychmiast)\n";

        // dodawanie nowych mieszkancow 
        m_villagers.emplace_back("Mariusz", glm::vec2(150.0f, 150.0f)); 
        m_villagers.emplace_back("Mateusz", glm::vec2(155.0f, 150.0f));
        globalFood -= 30; 

    } else {
        std::cout << "\n[DECYZJA] 'Musimy myśleć o sobie.' Odprawiliście ich.\n";
        std::cout << "[EFEKT] Mieszkańcy są przybici.";
    }

    setMode(Mode::PLAYING); 
}