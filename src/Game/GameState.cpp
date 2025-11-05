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
    // nie trzeba konstruktora wszystko jest w liscie inicjalizowane 
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
        globalFood -= 50; 
        std::cout << "\n--- Nowy dzien" << dayCounter << " ----" << std::endl; 
    }


    // debugowanie w konsoli 
    // drukowanie co jakis czas zeby nie bylo co klatke dla czytelnosci 
    if (m_TimeAccumulator > 0.5f) {
        m_TimeAccumulator = 0.0f; 

        std::cout << "Dzień: " << dayCounter 
                    << " | Godzina: " << (int)timeOfDay
                    << " | Jedzenie: " << globalFood
                    << " \r"; // czyszczenie starych znakow
        std::cout.flush(); // wypisywanie bufora 
    }
}