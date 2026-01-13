#include "Game/GameState.h"
#include "Game/Pathfinder.h"

#define STB_PERLIN_IMPLEMENTATION
#include "../../vendor/stb/stb_perlin.h"

#include <iostream> 



GameState::GameState() 
    : dayCounter(1),
      timeOfDay(6.0f), // zaczynanie dnia od 6 :( 
      globalFood(100),
      currentSeason(Season::SUMMER),
      globalTemperature(25.0f), 
      seasonTimer(0.0f), 
      heatingTimer(0.0f), 
      globalWood(150),
      globalWater(50),
      globalStone(50),
      globalMorale(100.0f),
      m_TimeAccumulator(0.0f),
      m_eventDay3Triggered(false), 
      m_eventDay5Triggered(false),
      m_currentMode(Mode::PLAYING),
      m_eventRefugeesTriggered(false),
      m_worldMap(std::make_unique<WorldMap>(200, 200, 20.0f))
      
{
    // 1. ustawianie osadnikow 
    m_villagers.emplace_back("Adam", glm::vec2(2000.0f, 2000.0f)); 
    m_villagers.emplace_back("Ewa", glm::vec2(2020.0f, 2000.0f));
    m_villagers.emplace_back("Radek", glm::vec2(2000.0f, 2020.0f));

    // 2. proceduralnie generowany swiat perlin noise 
    // parametry szumu 
    m_resourceNodes.emplace_back(ResourceNode::TREE, glm::vec2(1960.0f, 2000.0f), 100);
    m_resourceNodes.emplace_back(ResourceNode::TREE, glm::vec2(1980.0f, 2040.0f), 100);
    m_resourceNodes.emplace_back(ResourceNode::TREE, glm::vec2(2040.0f, 1960.0f), 100);
    
    m_resourceNodes.emplace_back(ResourceNode::ROCK, glm::vec2(2080.0f, 2080.0f), 200);
    
    m_resourceNodes.emplace_back(ResourceNode::BERRY_BUSH, glm::vec2(2020.0f, 1950.0f), 50);
    m_resourceNodes.emplace_back(ResourceNode::BERRY_BUSH, glm::vec2(2060.0f, 1960.0f), 50);


    // KROK B: GENERATOR PERLIN
    float scale = 0.005f; 
    int step = 40;        

    float perlinSeed = (float)(rand() % 10000) / 100.0f; 

    for (int y = 0; y < 4000; y += step) {
        for (int x = 0; x < 4000; x += step) {
            
            // --- ZMIANA: STREFA OCHRONNA 70 (Bardzo ciasno) ---
            // Tylko ścisłe centrum (tam gdzie stoją ludzie) jest puste.
            // Las będzie rósł tuż przy budynkach.
            bool safePlayer = glm::distance(glm::vec2(x, y), glm::vec2(2000.0f, 2000.0f)) < 70.0f;
            
            // 2. Punkt spawnu uchodźców (150, 150) - To naprawia crash losowości!
            bool safeRefugees = glm::distance(glm::vec2(x, y), glm::vec2(150.0f, 150.0f)) < 60.0f;

            if (glm::distance(glm::vec2(x, y), glm::vec2(2000.0f, 2000.0f)) < 150.0f || 
                glm::distance(glm::vec2(x, y), glm::vec2(150.0f, 150.0f)) < 60.0f)   // Strefa dla uchodźców
            {
                continue;
            }

            float noiseValue = stb_perlin_noise3(x * scale, y * scale, perlinSeed, 0, 0, 0);

            float offsetX = (rand() % 20) - 10.0f;
            float offsetY = (rand() % 20) - 10.0f;

            // Progi bez zmian (gęsty las)
            if (noiseValue > 0.25f) {
                m_resourceNodes.emplace_back(ResourceNode::TREE, glm::vec2(x + offsetX, y + offsetY), 100);
            }
            else if (noiseValue > 0.0f && noiseValue <= 0.25f) {
                if (rand() % 3 == 0) { 
                    m_resourceNodes.emplace_back(ResourceNode::ROCK, glm::vec2(x + offsetX, y + offsetY), 200);
                }
            }
            else if (noiseValue < -0.2f) {
                if (rand() % 4 == 0) {
                     m_resourceNodes.emplace_back(ResourceNode::BERRY_BUSH, glm::vec2(x + offsetX, y + offsetY), 50);
                }
            }
        }
    }
     
    

    // 3. budynki startowe 
    //m_buildings.emplace_back(Building::KITCHEN, glm::vec2(2050.0f, 2050.0f));
    //m_buildings.emplace_back(Building::STOCKPILE, glm::vec2(2100.0f, 2050.0f));
    //m_buildings.emplace_back(Building::WELL, glm::vec2(2050.0f, 2100.0f));

    // 4. aktualizowanie mapy logicznej bla bla bla pathtracing 
    for (const auto& node : m_resourceNodes)
    {
        // konwersja pozycji z piksela na kratke 
        glm::ivec2 gridPos = m_worldMap->worldToGrid(node.position); 

        // ustawianie przeszkody 
        m_worldMap->setObstacle(gridPos.x, gridPos.y, true); 
    }

    for (const auto& building : m_buildings) 
    {
        glm::ivec2 gridPos = m_worldMap->worldToGrid(building.position); 

        m_worldMap->setObstacle(gridPos.x, gridPos.y, true); 
        m_worldMap->setObstacle(gridPos.x + 1 , gridPos.y, true); 
        m_worldMap->setObstacle(gridPos.x, gridPos.y + 1 , true); 
        m_worldMap->setObstacle(gridPos.x + 1 , gridPos.y + 1, true);
        
        m_worldMap->setObstacle(gridPos.x - 1, gridPos.y, true); 
        m_worldMap->setObstacle(gridPos.x, gridPos.y - 1, true); 
    }


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
        globalTemperature = 25.0f; 
    }
    else 
    {
        currentSeason = Season::WINTER; 
        globalTemperature = -10.0f; 

        heatingTimer += deltaTime;
        if (heatingTimer > 2.0f)
        {
            heatingTimer = 0.0f; 

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
                if (globalWood >= 5) globalWood -= 5; 
            }
            else 
            {
                if (globalWood >= 10) globalWood -= 10; 
            }
        }
    }
    
    // --- 2. petla po mieszkancach  ---
    for (auto it = m_villagers.begin(); it != m_villagers.end(); ) 
    {
        Villager& villager = *it; // dostep do osadnika przez referencje 
        
        // potrzeby 
        villager.hunger -= (gameHoursPassed * HUNGER_RATE);
        villager.thirst -= (gameHoursPassed * THIRST_RATE);
        if (villager.hunger < 0.0f) villager.hunger = 0.0f;
        if (villager.thirst < 0.0f) villager.thirst = 0.0f;

        bool isTakingDamage = false; 

        // obrazenia od glodu 
        if (villager.hunger <= 0.0f) 
        {
            villager.health -= 5.0f * deltaTime; 
            isTakingDamage = true; 
        }
        // obrazenie od pragnienia 
        if (villager.thirst <= 0.0f) 
        {
            villager.health -= 10.0f * deltaTime; 
            isTakingDamage = true; 
        }
        // obrazenia od zimna jak jest zima a nie ma czym sie ogrzac 
        if (currentSeason == Season::WINTER && globalWood <= 0)
        {
            villager.health -= 2.0f * deltaTime; 
            isTakingDamage = true; 
        }

        // regeneracja zycia kiedy potrzeby sa spelnione 
        if (!isTakingDamage && villager.hunger > 50.0f && villager.thirst > 50.0f)
        {
            villager.health += 5.0f * deltaTime; 
        }
        // limit zdrowia 
        if (villager.health > 100.0f) villager.health = 100.0f; 

        // smierc 
        if (villager.health <= 0.0f)
        {
            std::cout << "[ŚMIERĆ] " << villager.name << " zmarł z wycienczenia.\n";

            globalMorale -= 25.0f; 
            if (globalMorale < 0.0f) globalMorale = 0.0f; 
            std::cout << "[MORALE] Spada drastycznie! Aktualnie: " << globalMorale << "%\n";
            it = m_villagers.erase(it); 
            continue;
        }

        // =========================================================
        // 2. INSTYNKT SAMOZACHOWAWCZY
        // =========================================================
        // --- PRIORYTET 1: WODA (Poniżej 40%) ---
        auto getSafeInteractionPoint = [&](glm::vec2 buildingPos) -> glm::vec2 
        {

            std::vector<float> distances = { 25.0f, 35.0f, 45.0f, 60.0f };
            
            glm::vec2 directions[] = { 
                {1,0}, {-1,0}, {0,1}, {0,-1},   // Prosto
                {0.7f, 0.7f}, {-0.7f, 0.7f}, {0.7f, -0.7f}, {-0.7f, -0.7f} // Skosy
            };
            
            for (float dist : distances) {
                for(auto dir : directions) {
                    glm::vec2 testPos = buildingPos + (dir * dist);
                    glm::ivec2 gridPos = m_worldMap->worldToGrid(testPos);
                    

                    if (!m_worldMap->isObstacle(gridPos.x, gridPos.y)) {
                        return testPos;
                    }
                }
            }
            return buildingPos;
        };

        bool isBusySurviving = 
        (
            villager.currentState == Villager::State::MOVING_TO_DRINK ||
            villager.currentState == Villager::State::DRINKING ||
            villager.currentState == Villager::State::MOVING_TO_EAT ||
            villager.currentState == Villager::State::EATING
        );

        if (!isBusySurviving) 
        {
            if (villager.thirst < 30.0f) 
            {
                Building* targetWell = nullptr; 
                for (auto& b : m_buildings) { 
                    if (b.buildingType == Building::WELL) { targetWell = &b; break; }
                }

                if (targetWell != nullptr)
                {
                    glm::vec2 safeSpot = getSafeInteractionPoint(targetWell->position);
                    
                    std::vector<glm::vec2> path = Pathfinder::findPath(villager.position, safeSpot, *m_worldMap);

                    if (!path.empty()) 
                    {
                        std::cout << "[AI] " << villager.name << " idzie pić. (Droga: " << path.size() << " krokow)\n";
                        villager.currentState = Villager::State::MOVING_TO_DRINK; 
                        villager.targetPosition = safeSpot; 
                        villager.targetNode = nullptr; 
                        villager.carryingAmount = 0; 

                        villager.currentPath = path;
                        villager.currentPathIndex = 0;  
                    }
                    else 
                    {
                        if (rand() % 100 == 0) 
                            std::cout << "[BLOKADA] " << villager.name << " chce pić, ale nie widzi drogi do studni!\n";
                    }
                }
            }
            // PRIORYTET B: JEDZENIE 
            else if (villager.hunger < 30.0f) 
            {
                Building* targetKitchen = nullptr;
                for (Building& b : m_buildings) {
                    if (b.buildingType == Building::KITCHEN) { targetKitchen = &b; break; }
                }

                if (targetKitchen != nullptr) 
                {
                    glm::vec2 safeSpot = getSafeInteractionPoint(targetKitchen->position);
                    std::vector<glm::vec2> path = Pathfinder::findPath(villager.position, safeSpot, *m_worldMap);

                    if (!path.empty())
                    {
                        std::cout << "[AI] " << villager.name << " idzie jeść. (Droga: " << path.size() << " krokow)\n";
                        villager.currentState = Villager::State::MOVING_TO_EAT;
                        villager.targetPosition = safeSpot;
                        villager.targetNode = nullptr; 
                        villager.carryingAmount = 0;   

                        villager.currentPath = path; 
                        villager.currentPathIndex = 0; 
                    }
                    else 
                    {
                         if (rand() % 100 == 0)
                            std::cout << "[BLOKADA] " << villager.name << " chce jeść, ale kuchnia zablokowana!\n";
                    }
                }
            }
        }

        // -- logika ruchu po sciezce --
       auto moveAlongPath = [&](float moveSpeed) -> bool {
            
            if (villager.currentPath.empty()) 
            {
                return true;
            }

            if (villager.currentPathIndex >= villager.currentPath.size()) return true; // koniec trasy

           glm::vec2 nextWorldPos = villager.currentPath[villager.currentPathIndex];

            glm::vec2 dir = nextWorldPos - villager.position;
            float dist = glm::length(dir);

            if (dist > 5.0f) // czy jest blisko punktu posredniego?
            {
                villager.position += glm::normalize(dir) * moveSpeed * deltaTime;
                return false; // idzie
            } 
            else 
            {
                // zaliczyl ten punkt, idzie do nastepnego
                villager.currentPathIndex++;
                
                // sprawdzamy czy to byl ostatni
                if (villager.currentPathIndex >= villager.currentPath.size()) return true;
                return false;
            }
        };

        // =========================================================
        // MASZYNA STANÓW (Z UŻYCIEM NOWEJ FUNKCJI)
        // =========================================================

        // 1. biegnie gdzie mu pokazemy na mapie 
        if (villager.currentState == Villager::State::MOVING_TO_POINT) 
        {
            if (moveAlongPath(speed))
            {
                villager.currentState = Villager::State::IDLE; 
            }
        }

        // 2. biegnie pracowac bo trzeba zyc 
        else if (villager.currentState == Villager::State::MOVING_TO_WORK) 
        {
            // sprawdzanie czy istnieje jeszcze zasob 
            if (villager.targetNode == nullptr) 
            {
                villager.currentState = Villager::State::IDLE;
                continue;
            }

            if (moveAlongPath(speed)) 
            {
                // jest u celu pracuje 
                villager.currentState = Villager::State::GATHERING;
                villager.workTimer = GATHER_TIME;
            }
        }

        // 3. praca praca
        else if (villager.currentState == Villager::State::GATHERING) 
        {
            if (villager.targetNode == nullptr) 
            {
                villager.currentState = Villager::State::IDLE;
                continue;
            }

            villager.workTimer -= deltaTime;

            if (villager.workTimer <= 0.0f) 
            {
                // konczenie pracy 
                if (villager.targetNode->resourceType == ResourceNode::Type::TREE) 
                {
                    villager.carryingResourceType = ResourceNode::Type::TREE;
                    villager.carryingAmount = 10;
                }
                else if (villager.targetNode->resourceType == ResourceNode::Type::ROCK) 
                {
                    villager.carryingResourceType = ResourceNode::Type::ROCK;
                    villager.carryingAmount = 5;
                }
                else if (villager.targetNode->resourceType == ResourceNode::Type::BERRY_BUSH) 
                {
                    villager.carryingResourceType = ResourceNode::Type::BERRY_BUSH;
                    villager.carryingAmount = 15;
                }

                // zmniejszanie zasobu na mapie 
                villager.targetNode->amountLeft -= villager.carryingAmount;
                if (villager.targetNode->amountLeft <= 0) {
                    villager.targetNode = nullptr;
                }

                // szukanie magazynu 
                Building* stockpile = findNearestStockpile(villager.position);
                if (stockpile != nullptr) 
                {
                    villager.currentState = Villager::State::MOVING_TO_HAUL;
                    villager.targetPosition = stockpile->position;
                    
                    villager.currentPath = Pathfinder::findPath(villager.position, stockpile->position, *m_worldMap); 
                    villager.currentPathIndex = 0; 

                } else {
                    villager.carryingAmount = 0;
                    villager.currentState = Villager::State::IDLE;
                    std::cout << "[AI] Brak magazynu!\n";
                }
            }
        }

        else if (villager.currentState == Villager::State::MOVING_TO_GATHER_WATER)
        {
            if (moveAlongPath(speed))
            {
                villager.currentState = Villager::State::GATHERING_WATER;
                villager.workTimer = 3.0f; 
            }
        }

        else if (villager.currentState == Villager::State::GATHERING_WATER)
        {
            villager.workTimer -= deltaTime; 
            if (villager.workTimer <= 0.0f)
            {
                villager.carryingResourceType = ResourceNode::Type::WATER; 
                villager.carryingAmount = 20; 

                Building* stockpile = findNearestStockpile(villager.position); 
                if (stockpile != nullptr)
                {
                    glm::vec2 safeSpot = getSafeInteractionPoint(stockpile->position); 

                    villager.currentPath = Pathfinder::findPath(villager.position, safeSpot, *m_worldMap); 

                    if(!villager.currentPath.empty())
                    {
                        villager.currentState = Villager::State::MOVING_TO_HAUL; 
                        villager.targetPosition = safeSpot; 
                        villager.currentPathIndex = 0; 
                    }
                    else 
                    {
                        villager.carryingAmount = 0; 
                        villager.currentState = Villager::State::IDLE; 
                    }
                }

                else 
                {
                    std::cout << "[AI] Mam wodę, ale nie ma magazynu!\n";
                    villager.currentState = Villager::State::IDLE; 
                }
            }
        }

        // 4. ruch do magazynu 
        else if (villager.currentState == Villager::State::MOVING_TO_HAUL) 
        {
            if (moveAlongPath(speed)) 
            {
                // wywala rzeczy w magazynie 
                if (villager.carryingResourceType == ResourceNode::Type::TREE) 
                    globalWood += villager.carryingAmount;
                else if (villager.carryingResourceType == ResourceNode::Type::ROCK) 
                    globalStone += villager.carryingAmount;
                else if (villager.carryingResourceType == ResourceNode::Type::BERRY_BUSH) 
                    globalFood += villager.carryingAmount;
                else if  (villager.carryingResourceType == ResourceNode::Type::WATER)
                    globalWater += villager.carryingAmount; 

                    
                villager.carryingAmount = 0;
                villager.carryingResourceType = ResourceNode::Type::NONE;

                // decydujemy czy wracamy pracowac czy nam sie nie chce i wolne 
                if (villager.targetNode != nullptr && villager.targetNode->amountLeft > 0) 
                {
                   villager.currentState = Villager::State::MOVING_TO_WORK; 

                   glm::vec2 dir = glm::normalize(villager.targetNode->position - villager.position);
                   if (glm::length(dir) == 0) dir = glm::vec2(1, 0); 
                   villager.targetPosition = villager.targetNode->position - (dir * 30.0f); 

                   villager.currentPath = Pathfinder::findPath(villager.position, villager.targetPosition, *m_worldMap); 
                   villager.currentPathIndex = 0; 
                } 
                else 
                {
                    villager.currentState = Villager::State::IDLE;
                }
            }
        }
        
        // 5. biegnie chać
        else if (villager.currentState == Villager::State::MOVING_TO_DRINK) 
        {
            if (villager.currentPath.empty() && glm::distance(villager.position, villager.targetPosition) > 40.0f) 
            {
                villager.currentState = Villager::State::IDLE; 
                continue;
            }

            if (moveAlongPath(speed)) 
            {
                villager.currentState = Villager::State::DRINKING;
                villager.workTimer = 2.0f;
            }
        }
        
        // 6. piciu 
        else if (villager.currentState == Villager::State::DRINKING) 
        {
            villager.workTimer -= deltaTime;
            if (villager.workTimer <= 0.0f) 
            {
                if (globalWater >= 10) { globalWater -= 10; villager.thirst = 100.0f; }
                villager.currentState = Villager::State::IDLE;
            }
        }

        // 7. biegnie żreć 
        else if (villager.currentState == Villager::State::MOVING_TO_EAT) 
        {
            if (villager.currentPath.empty() && glm::distance(villager.position, villager.targetPosition) > 40.0f) 
            {
                villager.currentState = Villager::State::IDLE; 
                continue;
            }

            if (moveAlongPath(speed)) 
            {
                villager.currentState = Villager::State::EATING;
                villager.workTimer = 2.0f;
            }
        }

        // 8. żarcie 
        else if (villager.currentState == Villager::State::EATING) 
        {
            villager.workTimer -= deltaTime;
            if (villager.workTimer <= 0.0f) 
            {
                if (globalFood >= 10) { globalFood -= 10; villager.hunger = 100.0f; }
                villager.currentState = Villager::State::IDLE;
            }
        }
        ++it;
    } // Koniec pętli for

    // --- Logika Dnia ---
    if(timeOfDay >= 24.0f)
    {
        dayCounter++; 
        timeOfDay = 0.0f; 
        std::cout << "\n--- Dzien " << dayCounter << " ---\n"; 
        checkForDailyEvents(); 
    }

    // --- Debug ---
    if (m_TimeAccumulator > 0.5f) 
    {
        m_TimeAccumulator = 0.0f; 
        std::cout << "D: " << dayCounter 
                  << " | Drewno: " << globalWood
                  << " | Populacja: " << m_villagers.size() 
                  << " \r";
        std::cout.flush(); 
    } 
}



void GameState::checkForDailyEvents() 
{
    // wydarzenie 1: zrzuty zaopatrzenia 
    if (dayCounter == 3 && !m_eventDay3Triggered) 
    {
        m_eventDay3Triggered = true; 

        // narracja na razie do konsoli 
        currentEventTitle = "Zrzut zapasow";
        currentEventDescription = "W nocy nad nasza wioska przelecial przyjacielski samolot i zrzucil troche zapasow.\nZysk: +50 Jedzenie, +20 Woda.\n";

        globalFood += 50; 
        globalWater += 20; 
    }

    if (dayCounter == 5 && !m_eventDay5Triggered)
    {   
        m_eventDay5Triggered = true; 

        currentEventTitle = "Najazd na bazę";
        currentEventDescription = "W nocy banda wrogiego wojska napadla na nasza wioske i ukradla nasze zapasy.\n\nStracilismy: -30 Jedzenia, -40 Drewna";
        globalFood -= 30;
        globalWood -= 40; 

        if (globalFood < 0 ) globalFood = 0; 
        if (globalWood < 0 ) globalWood = 0; 
    }

    if (dayCounter == 2 && !m_eventRefugeesTriggered) 
    {
        m_eventRefugeesTriggered = true; 
        
        currentEventTitle = "Uchodzcy u bram";
        currentEventDescription = "Do wioski zbliza sie grupa wyglodnialych cywilow.\n Prosza o schornienie i jedzenie.\n\nKoszt: -30 Jedzenia\nZysk: +2 Mieszkancow";

        setMode(Mode::EVENT_PAUSED); 
        
    }
}


Building* GameState::findNearestStockpile(const glm::vec2& fromPos)
{
    Building* closestStockpile = nullptr; 
    float minDistanceSquared = 999999.0f * 999999.0f; 
    
    for(Building& b : m_buildings)
    {
        
        if (b.buildingType == Building::Type::STOCKPILE)
        {     
            float distanceSquared = glm::length(b.position - fromPos);

            if (distanceSquared < minDistanceSquared)
            {
                minDistanceSquared = distanceSquared; 
                closestStockpile = &b; 
            }
        }
    }

    return closestStockpile;
}

void GameState::setMode(Mode newMode)
{
    m_currentMode = newMode; 
}


void GameState::resolveRefugeeEvent(bool accepted) 
{
    if (accepted) 
    {
        std::cout << "\n[DECYZJA] 'Nie możemy ich tak zostawić.' Wpuściliście ich.\n";
        std::cout << "[EFEKT] +2 Mieszkańców, -30 Jedzenia (natychmiast)\n";
        globalMorale += 15.0f; 
        if (globalMorale > 100.0f) globalMorale = 100.0f; 
        // dodawanie nowych mieszkancow 
        m_villagers.emplace_back("Mariusz", glm::vec2(150.0f, 150.0f)); 
        m_villagers.emplace_back("Mateusz", glm::vec2(155.0f, 150.0f));
        globalFood -= 30; 
    } 
    else 
    {   
        globalMorale -= 20.0f; 
        std::cout << "\n[DECYZJA] 'Musimy myśleć o sobie.' Odprawiliście ich.\n";
        std::cout << "[EFEKT] Mieszkańcy są przybici. (Morale -20)\n";
    }

    setMode(Mode::PLAYING); 
}

// --- obsluga zapisu i odczytu gry ---
void GameState::saveGame(const std::string& filename)
{
    std::ofstream file(filename); 

    if (!file.is_open())
    {
        std::cerr<< "Nie mozna otworzyc pliku do zapisu!\n";
        return; 
    }

    // 1. zapisywanie zmiennych lokalnych 
    // kolejnosc: dzien, czas, drewno, woda, jedzenie, temperatura, pora roku
    file << "GLOBAL " 
        << dayCounter << " " 
        << timeOfDay << " " 
        << globalWood << " " 
        << globalWater << " " 
        << globalFood << " "
        << globalTemperature << " " 
        << (int)currentSeason << " "
        << globalMorale << "\n";

    // 2. zapisywanie mieszkancow 
    for (const auto& v : m_villagers) 
    {
        file << "VILLAGER " 
        << v.name << " " 
        << v.position.x << " " << v.position.y << " " 
        << v.hunger << " " << v.thirst << "\n";
    }

    // 3. zapisywanie budynkow 
    for (const auto& b : m_buildings) 
    {
        file << "BUILDING " 
        <<(int)b.buildingType << " " 
        << b.position.x << " " << b.position.y << "\n";
    }

    file.close(); 
}

// odczyt gry 
void GameState::loadGame(const std::string& filename)
{
    std::ifstream file(filename); // otwieranie pliku do odczytu 

    if (!file.is_open()) 
    {
        std::cerr << "Nie mozna otworzyc pliku do wczytania!\n";
        return; 
    }

    // oczyszczanie obecnego stanu bardzo wazne 
    m_villagers.clear(); 
    m_buildings.clear(); 

    std::string lineType; 
    int globalLinesRead = 0; 

    while (file >> lineType) 
    {
        if (lineType == "GLOBAL")
        {
            int seasonInt;

            int d; float t; int wood, water, food; float temp; float mor;
            
            file >> d >> t >> wood >> water >> food >> temp >> seasonInt >> mor;

            dayCounter = d;
            timeOfDay = t;
            globalWood = wood;
            globalWater = water;
            globalFood = food;
            globalTemperature = temp;
            currentSeason = (Season)seasonInt;
            globalMorale = mor; 

            globalLinesRead++;
            std::cout << "[DEBUG] Wczytano GLOBAL: Dzien " << dayCounter << ", Czas " << timeOfDay << "\n";
        }
        else if (lineType == "VILLAGER")
        {
            std::string name; 
            float x, y, hunger, thirst; 
            file >> name >> x >> y >> hunger >> thirst; 

            // tworzenie nowego mieszkanca 
            m_villagers.emplace_back(name, glm::vec2(x, y));
            // ustawianie mu statystyk 
            m_villagers.back().hunger = hunger; 
            m_villagers.back().thirst = thirst; 
        }
        else if (lineType == "BUILDING")
        {
            int typeInt; 
            float x, y; 
            file >> typeInt >> x >> y; 
            m_buildings.emplace_back((Building::Type)typeInt, glm::vec2(x, y)); 
        }
    }

    file.close(); 

    // resetowanie timerow
    heatingTimer = 0.0f; 
}