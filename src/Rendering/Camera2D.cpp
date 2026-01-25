#include "Rendering/Camera2D.h"

Camera2D::Camera2D(float width, float height)
    : m_Position(0.0f, 0.0f), m_Zoom(1.0f), m_Width(width), m_Height(height)
{
    // ustawianie projekcji 
    m_ProjectionMatrix = glm::ortho(0.0f, m_Width, 0.0f, m_Height, -1.0f, 1.0f);
    // ustawianie widoku 
    recalculateViewMatrix(); 
}

void Camera2D::update(float deltaTime, const sf::Window& window){
    // tymczasowa logika sterowania 
    float speed = 200.0f * deltaTime; 
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) m_Position.y += speed; 
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) m_Position.y -= speed; 
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) m_Position.x -= speed; 
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) m_Position.x += speed; 

    m_Zoom += (m_TargetZoom - m_Zoom) * 10.0f * deltaTime; 

    recalculateViewMatrix(); 
}

void Camera2D::recalculateViewMatrix(){
    glm::mat4 transform = glm::mat4(1.0f); 

    transform = glm::translate(transform, glm::vec3(m_Width * 0.5f, m_Height * 0.5f, 0.0f));
    transform = glm::scale(transform, glm::vec3(m_Zoom, m_Zoom, 1.0f));
    transform = glm::translate(transform, glm::vec3(-m_Position.x - (m_Width * 0.5f), -m_Position.y - (m_Height * 0.5f), 0.0f));

    m_ViewMatrix = transform;
    m_ProjectionViewMatrix = m_ProjectionMatrix * m_ViewMatrix; 
}

glm::vec2 Camera2D::screenToWorld(const glm::vec2& screenCoords, const sf::Window& window){
    // kordy myszki 
    // obracamy os y myszli bo 0, 0 to lewy gorny rog a w opeen gl to jest lewy dolny roh 
    float y_normalized = (float)window.getSize().y - screenCoords.y; 

    // kordy w przestrzeni ekranu 
    glm::mat4 invProjView = glm::inverse(m_ProjectionViewMatrix); 

    // przeksztalcanie pozycji ekranowej z powrotem do swiata 
    float x = (2.0f * screenCoords.x) / (float)window.getSize().x - 1.0f; 
    float y = (2.0f * y_normalized) / (float)window.getSize().y - 1.0f;

    glm::vec4 worldPos = invProjView * glm::vec4(x, y, 0.0f, 1.0f); 

    // dzielenie przez w dla perspektywy 
    if (worldPos.w != 0.0f){
        worldPos.x /= worldPos.w; 
        worldPos.y /- worldPos.w; 
    }

    return glm::vec2(worldPos.x, worldPos.y); 
}

void Camera2D::addZoom(float delta)
{
    m_TargetZoom += delta; 

    if (m_TargetZoom < 0.3f) m_TargetZoom = 0.3f;
    if (m_TargetZoom > 3.0f) m_TargetZoom = 3.0f; 
}