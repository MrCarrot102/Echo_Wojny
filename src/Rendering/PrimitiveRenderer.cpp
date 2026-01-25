#include "Rendering/PrimitiveRenderer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream> 



PrimitiveRenderer::PrimitiveRenderer() {
    // shader prowadzacy do assets/ 
    m_Shader = std::make_unique<Shader>("assets/shaders/simple.vert", "assets/shaders/simple.frag");

    // tworzenie geometri vbo i vao 
   float vertices[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f
    };

    glGenVertexArrays(1, &m_QuadVAO);
    glBindVertexArray(m_QuadVAO); 

    glGenBuffers(1, &m_QuadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); 

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); 

        int segments = 32; // Im więcej segmentów, tym gładsze koło (32 to standard)
    m_CircleVertexCount = segments + 2; 

    std::vector<float> circleVertices;
    circleVertices.push_back(0.0f); // Środek koła X
    circleVertices.push_back(0.0f); // Środek koła Y

    for (int i = 0; i <= segments; i++) {
        // Obliczanie kąta w radianach
        float angle = i * 2.0f * 3.14159265359f / segments;
        // Współrzędne na obwodzie (X, Y)
        circleVertices.push_back(std::cos(angle));
        circleVertices.push_back(std::sin(angle));
    }

    // Przesyłanie do karty graficznej (VAO i VBO)
    glGenVertexArrays(1, &m_CircleVAO);
    glBindVertexArray(m_CircleVAO); 

    glGenBuffers(1, &m_CircleVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_CircleVBO);
    glBufferData(GL_ARRAY_BUFFER, circleVertices.size() * sizeof(float), circleVertices.data(), GL_STATIC_DRAW); 

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

    // dodanie destruktora 
PrimitiveRenderer::~PrimitiveRenderer() {
    glDeleteVertexArrays(1, &m_QuadVAO); 
    glDeleteBuffers(1, &m_QuadVBO);
    glDeleteVertexArrays(1, &m_CircleVAO); 
    glDeleteBuffers(1, &m_CircleVBO);
}


void PrimitiveRenderer::drawSquare(Camera2D& camera, 
                                    const glm::vec2& position, 
                                    const glm::vec2& size, 
                                    const glm::vec4& color) 

{
    m_Shader->bind();

    // budowanie macierzy modelu 
    glm::mat4 model = glm::mat4(1.0f); 
    model = glm::translate(model, glm::vec3(position, 0.0f));
    model = glm::scale(model, glm::vec3(size, 1.0f)); 
    
    // ustawianie uniformow 
    m_Shader->setUniformMat4("u_projectionView", camera.getProjectionViewMatrix());
    m_Shader->setUniformMat4("u_model", model);
    m_Shader->setUniformVec4("u_color", color); 
    
    // rysowanie 
    glBindVertexArray(m_QuadVAO);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
  
    glBindVertexArray(0);
    m_Shader->unbind();
}

void PrimitiveRenderer::drawCircle(Camera2D& camera, 
                                    const glm::vec2& position, 
                                float radius, 
                            const glm::vec4& color)
{
    m_Shader->bind(); 
    glm::mat4 model = glm::mat4(1.0f); 
    model = glm::translate(model, glm::vec3(position, 0.0f)); 
    model = glm::scale(model, glm::vec3(radius, radius, 1.0f)); 

    m_Shader->setUniformMat4("u_projectionView", camera.getProjectionViewMatrix());
    m_Shader->setUniformMat4("u_model", model);
    m_Shader->setUniformVec4("u_color", color); 
    
    // Rysowanie techniką TRIANGLE_FAN
    glBindVertexArray(m_CircleVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, m_CircleVertexCount);
    glBindVertexArray(0);

    m_Shader->unbind();
}