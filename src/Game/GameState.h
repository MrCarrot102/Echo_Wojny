#pragma once 


class GameState {
    public: 
        GameState(); 

        // glowna funkcja ticki symulacji 
        void update(float deltaTime); 

        // dane gry 

        // czas 
        int dayCounter; 
        // czas w godzinach 
        float timeOfDay; 

        // zasoby globalne 
        int globalFood; 
        int globalWood; 
        int globalWater; 

    private: 
        // liczenie czasu 
        float m_TimeAccumulator; 

};

