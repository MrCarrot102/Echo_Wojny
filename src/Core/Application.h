#pragma once 
#include <SFML/Window.hpp> 
#include <GL/glew.h> 
#include "Core/Camera2D.h"
#include "Rendering/PrimitiveRenderer.h"
#include <memory> 


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
};
