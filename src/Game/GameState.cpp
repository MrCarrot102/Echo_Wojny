#include "Game/GameState.h"


#include <iostream> 

GameState::GameState() 
    : dayCounter(1),
      timeOfDay(6.0f), // zaczynanie dnia od 6 :( 
      globalFood(100),
      globalWood(50),
      globalWater(50),
      m_TimeAccumulator(0.0f),
      m_eventDay3Triggered(false), 
      m_eventDay5Triggered(false)
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
    float speed = 50.00f; // ruch bardziej predkosc 
    
    const float GATHER_TIME = 3.0f; // czas zbierania czegos na razie 3s 
    
    const float HUNGER_THRESHOLD = 30.0f; // moment kiedy robi sie glodny 
    const float THIRST_THRESHOLD = 40.0f; // 
    
    const float HUNGER_PRE_HOUR = 2.0f; // jak szybko postaci chce sie jesc 
    const float THIRST_PER_HOUR = 3.0f; 


    for (Villager& villager : m_villagers) {
        // symulowanie pragnienia i glodu 
        // pragnienie 
        villager.thirst -= (gameHoursPassed * THIRST_PER_HOUR); 
        // symulowanie glodu 
        villager.hunger -= (gameHoursPassed * HUNGER_PRE_HOUR); 
        if (villager.hunger < 0.0f) villager.hunger = 0.0f; 
        if (villager.thirst < 0.0f) villager.thirst = 0.0f; 



        // logika cos tam oparta na priorytetach bo jedzienie najwazniejsze kuzwa 
        
        if ( villager.thirst < THIRST_THRESHOLD && 
            villager.currentState != Villager::State::DRINKING && 
            villager.currentState != Villager::State::MOVING_TO_DRINK)
        {
            // szukanie najblizszej studnii 
            Building* targetWell = nullptr; 
            for (Building& b : m_buildings) {
                if (b.buildingType == Building::WELL){
                    targetWell = &b; 
                    break; 
                }
            }

            if (targetWell != nullptr) {
                std::cout << "\b[AI] " << villager.name << " jest spragniony! Idzie do studni.\n";
                villager.currentState = Villager::State::MOVING_TO_DRINK; 
                villager.targetPosition = targetWell->position; 
            } else {
                std::cout << "\n[AI] " << villager.name << " jest spragniony, ALE NIE MA STUDNI!\n";
            }
        }
        
        
        
        
        else if  (villager.hunger < HUNGER_THRESHOLD && 
            villager.currentState != Villager::State::EATING && 
            villager.currentState != Villager::State::MOVING_TO_EAT)
        {
            // szuka najblizeszj kuchni 
            if (!m_buildings.empty()){ 
                std::cout << "\n[AI] " << villager.name << " jest głodny! Idzie jeść.\n";
                villager.currentState = Villager::State::MOVING_TO_EAT;
                villager.targetPosition = m_buildings[0].position; // pierwsza dostepna kuchnia 
            }
        }


        //---------------
        // IDZIE TYLKO D PUNKU 
        //---------------
        else if (villager.currentState == Villager::State::MOVING_TO_POINT) {
            glm::vec2 direction = villager.targetPosition - villager.position; 
            float distance = glm::length(direction); 

            if (distance > 1.0f){
                villager.position += glm::normalize(direction) * speed * deltaTime; 
            } else {
                villager.position = villager.targetPosition; 
                villager.currentState = Villager::State::IDLE; // jak dotarl to sobie odpoczywa 
            }
        }

        //----------------
        // IDZIE SOBIE DO PRACY 
        //----------------
        else if (villager.currentState == Villager::State::MOVING_TO_WORK){
            if (villager.targetNode == nullptr){
                // jak cel nie istnieje to sobie odpoczywamy 
                villager.currentState = Villager::State::IDLE; 
                continue; // pomijanie reszty logiki
            }

            glm::vec2 direction = villager.targetPosition - villager.position; 
            float distance = glm::length(direction); 

            // zasieg na 5.0f zeby sie nie nachodzilo 
            if (distance > 5.0f){
                villager.position += glm::normalize(direction) * speed * deltaTime; 
            } else {
                // musi pracowac bo dotarl 
                villager.currentState = Villager::State::GATHERING; 
                villager.workTimer = GATHER_TIME; 
            }
        }

        //-----------------
        // ROBOTAJU
        //-----------------
        else if (villager.currentState == Villager::State::GATHERING) {
        if (villager.targetNode == nullptr) {
            villager.currentState = Villager::State::IDLE;
            continue;
        }

        // Możesz odkomentować tę linię, aby zobaczyć timer w akcji
        // std::cout << "Postać " << villager.name << " pracuje... Timer: " << villager.workTimer << "\r";

        // Odliczaj czas
        villager.workTimer -= deltaTime;
        
        if (villager.workTimer <= 0.0f) {
            
            // --- PUNKT KONTROLNY 1 ---
            std::cout << "\n[DEBUG] Timer zerowy! Sprawdzam typ zasobu... \n";

            if (villager.targetNode->resourceType == ResourceNode::Type::TREE) {
                // --- PUNKT KONTROLNY 2 (Sukces) ---
                std::cout << "[DEBUG] Typ to DRZEWO. Dodaję drewno.\n";
                globalWood += 10; 
                std::cout << "[EVENT] Zebrano Drewno! Total: " << globalWood << std::endl;
            
            } else if (villager.targetNode->resourceType == ResourceNode::Type::ROCK) {
                // --- PUNKT KONTROLNY 2 (Sukces) ---
                std::cout << "[DEBUG] Typ to KAMIEŃ. Dodaję kamień.\n";
                std::cout << "[EVENT] Zebrano Kamień!" << std::endl;
            } else {
                // --- PUNKT KONTROLNY 2 (BŁĄD) ---
                std::cout << "[DEBUG] BŁĄD! Nie rozpoznano typu zasobu!\n";
            }

            // Zmniejsz ilość zasobu w miejscu
            villager.targetNode->amountLeft -= 10;
            
            if (villager.targetNode->amountLeft <= 0) {
                // Zasób się wyczerpał!
                std::cout << "[EVENT] Zasób wyczerpany!" << std::endl;
                villager.targetNode = nullptr; 
                villager.currentState = Villager::State::IDLE;
            } else {
                // Zasób jeszcze jest, resetuj zegar i pracuj dalej
                villager.workTimer = GATHER_TIME;
            }
            }
        }


        // biegnie zrec 
        else if (villager.currentState == Villager::State::MOVING_TO_EAT){
            glm::vec2 direction = villager.targetPosition - villager.position; 
            float distance = glm::length(direction); 
            if (distance > 5.0f){
                villager.position += glm::normalize(direction) * speed * deltaTime; 
            } else {
                // juz sobie w kuchni siedzie 
                villager.currentState = Villager::State::EATING;
                villager.workTimer = 2.0f; // ile je (2s)
            }
        }

        else if (villager.currentState == Villager::State::EATING){
            villager.workTimer -= deltaTime; 
            if (villager.workTimer <= 0.0f){
                if (globalFood >= 10){
                    globalFood -= 10; // jeden zjada 10 jedzenia 
                    villager.hunger = 100.0f; // jest najedzony
                    villager.currentState = Villager::State::IDLE;
                    std::cout << "\n[AI] " << villager.name << " zjadł. Zapasy: " << globalFood << std::endl; 
                } else {
                    // kiedy nie mamy jedzenia 
                    std::cout << "\n[AI] " << villager.name << "chciał jeść, ale nie ma zapasów!\n";
                    villager.currentState = Villager::State::IDLE; 
                }
            }
        }

        else if (villager.currentState == Villager::State::MOVING_TO_DRINK) {
            glm::vec2 direction = villager.targetPosition - villager.position; 
            float distance = glm::length(direction); 
            if (distance > 5.0f) {
                villager.position += glm::normalize(direction) * speed * deltaTime; 
            } else {
                villager.currentState = Villager::State::DRINKING; 
                villager.workTimer = 2.0f; 
            }
        }

        // stan picia 
        else if (villager.currentState == Villager::State::DRINKING) { 
            villager.workTimer -= deltaTime; 
            if (villager.workTimer <= 0.0f) {
                if (globalWater >= 10) {
                    globalWater -= 10; 
                    villager.thirst = 100.f; 
                    villager.currentState = Villager::State::IDLE; 
                    std::cout << "\n[AI] " << villager.name << " napił się. Zapasy wody: " << globalWater << std::endl; 
                } else {
                    std::cout << "\n[AI] " << villager.name << " chciał pić, ale nie ma wody!\n"; 
                    villager.currentState = Villager::State::IDLE; 
                }
            }
        }
    }



    // logika nowego dnia 
    if(timeOfDay >= 24.0f){
        dayCounter++; 
        timeOfDay = 0.0f; // resetowanie zegara 

        // teoretyczne zuzywanie zasobow 
        std::cout << "\n--- Nowy dzien " << dayCounter << " ----" << std::endl; 
    
        checkForDailyEvents(); 
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

    m_buildings.emplace_back(Building::KITCHEN, glm::vec2(150.0f, 150.0f));
}


void GameState::checkForDailyEvents() {
    
    // wydarzenie 1: zrzuty zaopatrzenia 
    if (dayCounter == 3 && !m_eventDay3Triggered) {
        
        m_eventDay3Triggered = true; 

        // narracja na razie do konsoli 
        std::cout << "\n=============== WYDARZENIE ===============\n";
        std::cout << "W nocy nad wioska przeleciał samolot\n";
        std::cout << "Słyszeliście huk, a rano znaleźliście zrzut na spadochronie!\n";
        std::cout << "[EFEKT] +50 Jedzenia, +20 Wody\n";
        std::cout << "==============================================\n";

        globalFood += 50; 
        globalWater += 20; 
    }

    if (dayCounter == 5 && !m_eventDay5Triggered){
        
        m_eventDay5Triggered = true; 

        std::cout << "\n=============== WYDARZENIE ===============\n";
        std::cout << "!!! ALARM !!!\n";
        std::cout << "Grupa szabrowników napadła na wasze magazyny pod osłoną nocy!\n";
        std::cout << "[EFEKT] -30 Jedzenia, -40 Drewna\n";
        std::cout << "================================\n";

        globalFood -= 30;
        globalWood -= 40; 

        if (globalFood < 0 ) globalFood = 0; 
        if (globalWood < 0 ) globalWood = 0; 
    }
}