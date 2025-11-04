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
}

// dodanie destruktora 
PrimitiveRenderer::~PrimitiveRenderer() {
    glDeleteVertexArrays(1, &m_QuadVAO); 
    glDeleteBuffers(1, &m_QuadVBO);
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