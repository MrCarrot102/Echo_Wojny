#include "Rendering/PrimitiveRenderer.h"
#include <glm/gtc/matrix_transform.hpp>


PrimitiveRenderer:PrimitiveRenderer() {
    // shader prowadzacy do assets/ 
    m_Shader = std::make_unique<Shader>("assets/shaders/primitive.vert", "assets/shaders/primitive.frag");

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

}