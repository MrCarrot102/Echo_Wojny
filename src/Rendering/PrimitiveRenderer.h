#pragma once 
#include "Rendering/Shader.h"
#include "Rendering/Camera2D.h"
#include <glm/glm.hpp>
#include <memory>

class PrimitiveRenderer {
    public: 
        PrimitiveRenderer(); 
        ~PrimitiveRenderer();

        void drawSquare(Camera2D& camera, 
                    const glm::vec2& position, 
                    const glm::vec2& size, 
                    const glm::vec4& color);

        void drawCircle(Camera2D& camera, const glm::vec2& position, float radius, const glm::vec4& color);

    private:
        std::unique_ptr<Shader> m_Shader;
        GLuint m_QuadVAO; 
        GLuint m_QuadVBO; 

        unsigned int m_CircleVAO, m_CircleVBO; 
        int m_CircleVertexCount; 
                    
};

