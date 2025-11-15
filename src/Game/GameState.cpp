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
    float speed = 50.00f; // ruch bardziej predkosc 
    const float GATHER_TIME = 3.0f; // czas zbierania czegos na razie 3s 

    for (Villager& villager : m_villagers) {
        //---------------
        // IDZIE TYLKO D PUNKU 
        //---------------
        if(villager.currentState == Villager::State::MOVING_TO_POINT) {
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
    }



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