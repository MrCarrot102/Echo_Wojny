#include "Game/GameState.h"
#include "Game/Pathfinder.h"

#include <iostream> 

GameState::GameState() 
    : dayCounter(1),
      timeOfDay(6.0f), // zaczynanie dnia od 6 :( 
      globalFood(100),
      currentSeason(Season::SUMMER),
      globalTemperature(25.0f), 
      seasonTimer(0.0f), 
      heatingTimer(0.0f), 
      globalWood(50),
      globalWater(50),
      globalStone(0),
      m_TimeAccumulator(0.0f),
      m_eventDay3Triggered(false), 
      m_eventDay5Triggered(false),
      m_currentMode(Mode::PLAYING),
      m_eventRefugeesTriggered(false),
      m_worldMap(std::make_unique<WorldMap>(100, 100, 20.0f))
      
{
    // 1. ustawianie osadnikow 
    m_villagers.emplace_back("Adam", glm::vec2(2000.0f, 2000.0f)); 
    m_villagers.emplace_back("Ewa", glm::vec2(2020.0f, 2000.0f));
    m_villagers.emplace_back("Radek", glm::vec2(2000.0f, 2020.0f));

    // 2. proceduralny generator swiata losowanie pozycji startowych 
    // drzewo 
    for (int i = 0; i < 300; i++)
    {
        float x = rand() % 4000; 
        float y = rand() % 4000; 
        m_resourceNodes.emplace_back(ResourceNode::TREE, glm::vec2(x, y), 100); 
    }
    // sklala 
    for (int i = 0; i < 100; i++)
    {
        float x = rand() % 4000; 
        float y = rand() % 4000; 
        m_resourceNodes.emplace_back(ResourceNode::ROCK, glm::vec2(x, y), 200);
    }
    // krzakow 
    for (int i = 0; i < 100; i++)
    {
        float x = rand() % 4000; 
        float y = rand() % 4000; 
        m_resourceNodes.emplace_back(ResourceNode::BERRY_BUSH, glm::vec2(x, y), 50);
    }

    // 3. budynki startowe 
    m_buildings.emplace_back(Building::KITCHEN, glm::vec2(2050.0f, 2050.0f));
    m_buildings.emplace_back(Building::STOCKPILE, glm::vec2(2100.0f, 2050.0f));
    m_buildings.emplace_back(Building::WELL, glm::vec2(2050.0f, 2100.0f));
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


    // --- 2. petla po mieszkancach  ---
    for (Villager& villager : m_villagers) 
    {
        
        // potrzeby 
        villager.hunger -= (gameHoursPassed * HUNGER_RATE);
        villager.thirst -= (gameHoursPassed * THIRST_RATE);
        if (villager.hunger < 0.0f) villager.hunger = 0.0f;
        if (villager.thirst < 0.0f) villager.thirst = 0.0f;

        // =========================================================
        // 2. INSTYNKT SAMOZACHOWAWCZY
        // =========================================================
        // --- PRIORYTET 1: WODA (Poniżej 40%) ---
        // Czy osadnik jest już w trakcie ratowania życia?
        bool isBusySurviving = 
        (
            villager.currentState == Villager::State::MOVING_TO_DRINK ||
            villager.currentState == Villager::State::DRINKING ||
            villager.currentState == Villager::State::MOVING_TO_EAT ||
            villager.currentState == Villager::State::EATING
        );

        // SPRAWDZAMY POTRZEBY TYLKO JEŚLI NIE JEST ZAJĘTY PRZETRWANIEM
        if (!isBusySurviving) 
        {
            // PRIORYTET A: WODA (Najważniejsza)
            if (villager.thirst < 40.0f) 
            {
                Building* targetWell = nullptr;
                for (Building& b : m_buildings) 
                {
                    if (b.buildingType == Building::WELL) { targetWell = &b; break; }
                }

                if (targetWell != nullptr) 
                {
                    std::cout << "[AI] " << villager.name << " idzie pić.\n";
                    villager.currentState = Villager::State::MOVING_TO_DRINK;
                    villager.targetPosition = targetWell->position;
                    villager.targetNode = nullptr; 
                    villager.carryingAmount = 0;   
                }
            }
            // PRIORYTET B: JEDZENIE 
            else if (villager.hunger < 30.0f) 
            {
                Building* targetKitchen = nullptr;
                for (Building& b : m_buildings) 
                {
                    if (b.buildingType == Building::KITCHEN) { targetKitchen = &b; break; }
                }

                if (targetKitchen != nullptr) 
                {
                    std::cout << "[AI] " << villager.name << " idzie jeść.\n";
                    villager.currentState = Villager::State::MOVING_TO_EAT;
                    villager.targetPosition = targetKitchen->position;
                    villager.targetNode = nullptr; 
                    villager.carryingAmount = 0;   
                }
            }
        }

        // =========================================================
        // MASZYNA STANÓW
        // =========================================================

        // 1. biegnie gdzie mu pokazemy na mapie 
        if (villager.currentState == Villager::State::MOVING_TO_POINT) 
        {
            glm::vec2 direction = villager.targetPosition - villager.position;
            float distance = glm::length(direction);

            if (distance > 5.0f) 
            {
                villager.position += glm::normalize(direction) * speed * deltaTime;
            } 
            else 
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

            glm::vec2 direction = villager.targetPosition - villager.position;
            float distance = glm::length(direction);

            // duza toleracja zeby w teksturach nie siedziec 
            if (distance > 20.0f) 
            {
                villager.position += glm::normalize(direction) * speed * deltaTime;
            } 
            else 
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
                    // zasób zniknął (wyczerpany)
                    // tutaj trzeba by go usunąć z wektora m_resourceNodes
                    villager.targetNode = nullptr;
                }

                // szukanie magazynu 
                Building* stockpile = findNearestStockpile(villager.position);
                if (stockpile != nullptr) 
                {
                    villager.currentState = Villager::State::MOVING_TO_HAUL;
                    villager.targetPosition = stockpile->position;
                } else {
                    // jak nie ma magazynu strajkujemy nic nie robimy 
                    villager.carryingAmount = 0;
                    villager.currentState = Villager::State::IDLE;
                    std::cout << "[AI] Brak magazynu!\n";
                }
            }
        }

        // 4. ruch do magazynu 
        else if (villager.currentState == Villager::State::MOVING_TO_HAUL) 
        {
            glm::vec2 direction = villager.targetPosition - villager.position;
            float distance = glm::length(direction);

            if (distance > 10.0f) 
            {
                villager.position += glm::normalize(direction) * speed * deltaTime;
            } 
            else 
            {
                // wywala rzeczy w magazynie 
                if (villager.carryingResourceType == ResourceNode::Type::TREE) 
                    globalWood += villager.carryingAmount;
                else if (villager.carryingResourceType == ResourceNode::Type::ROCK) 
                    globalStone += villager.carryingAmount;
                else if (villager.carryingResourceType == ResourceNode::Type::BERRY_BUSH) 
                    globalFood += villager.carryingAmount;

                villager.carryingAmount = 0;
                villager.carryingResourceType = ResourceNode::Type::NONE;

                // decydujemy czy wracamy pracowac czy nam sie nie chce i wolne 
                if (villager.targetNode != nullptr && villager.targetNode->amountLeft > 0) 
                {
                    villager.currentState = Villager::State::MOVING_TO_WORK;
                    villager.targetPosition = villager.targetNode->position;
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
            glm::vec2 direction = villager.targetPosition - villager.position;
            float distance = glm::length(direction);
            
            if (distance > 10.0f) 
            {
                villager.position += glm::normalize(direction) * speed * deltaTime;
            } else 
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
            glm::vec2 direction = villager.targetPosition - villager.position;
            float distance = glm::length(direction);
            
            if (distance > 10.0f) 
            {
                villager.position += glm::normalize(direction) * speed * deltaTime;
            } 
            else 
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
        << (int)currentSeason << "\n";

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
            // Wczytujemy do zmiennych tymczasowych, żeby sprawdzić czy się udało
            int d; float t; int wood, water, food; float temp;
            
            file >> d >> t >> wood >> water >> food >> temp >> seasonInt;

            // Przypisujemy do stanu gry
            dayCounter = d;
            timeOfDay = t;
            globalWood = wood;
            globalWater = water;
            globalFood = food;
            globalTemperature = temp;
            currentSeason = (Season)seasonInt;

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