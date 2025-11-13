#pragma once 
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <SFML/Window.hpp> 

class Camera2D {
    public: 
        Camera2D(float width, float height);
        void update(float deltaTime, const sf::Window& window); 
        const glm::mat4& getProjectionViewMatrix() const {return m_ProjectionViewMatrix;}

        
        glm::vec2 screenToWorld(const glm::vec2& screenCoords, const sf::Window& window); 

    private: 
        void recalculateViewMatrix();

        glm::vec2 m_Position; 
        float m_Zoom; 
        float m_Width, m_Height;
        glm::mat4 m_ProjectionMatrix;
        glm::mat4 m_ViewMatrix; 
        glm::mat4 m_ProjectionViewMatrix; 
};

