#pragma once 
 
#include "Rendering/Camera2D.h"
#include "Rendering/PrimitiveRenderer.h"
#include "Game/GameState.h"
#include "Game/Villager.h"
#include <imgui-SFML.h>
#include "imgui.h" 

#include <SFML/Window.hpp>
#include <memory> 
#include <GL/glew.h> 
#include <glm/glm.hpp> 

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



        enum class BuildMode {
            NONE, 
            KITCHEN, 
            WELL, 
            STOCKPILE,
            CAMPFIRE
        };

        sf::Window m_Window; 
        sf::Clock m_DeltaClock; 
        bool m_Running; 

        // smart pointery do kontroli czasu zycia opiektow w opengl BO MOGE 
        std::unique_ptr<Camera2D> m_Camera; 
        std::unique_ptr<PrimitiveRenderer> m_Renderer; 


        std::unique_ptr<GameState> m_GameState; 


        Villager* m_selectedVillager; 
        BuildMode m_currentBuildMode = BuildMode::NONE; 
        glm::vec2 m_ghostBuildingPos; 

        float m_timeScale = 1.0f; 


    };