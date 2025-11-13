#pragma once 
 
#include "Core/Camera2D.h"
#include "Rendering/PrimitiveRenderer.h"
#include "Game/GameState.h"

#include <SFML/Window.hpp>
#include <memory> 
#include <GL/glew.h> 

class Application{
    public: 
        Application(); 
        ~Application(); 


        void run(); 

    private: 
        void init(); 
        void pollEvents(); 
        void update(float deltaTime); 
        void render(); 

        sf::Window m_Window; 
        sf::Clock m_DeltaClock; 
        bool m_Running; 

        // smart pointery do kontroli czasu zycia opiektow w opengl BO MOGE 
        std::unique_ptr<Camera2D> m_Camera; 
        std::unique_ptr<PrimitiveRenderer> m_Renderer; 


        std::unique_ptr<GameState> m_GameState; 


        Villager* m_selectedVillager; 
};
