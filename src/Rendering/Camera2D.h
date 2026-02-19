#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "SFML/Window.hpp"

class Camera2D
{
 public:
  Camera2D(float width, float height);
  const glm::mat4& getProjectionViewMatrix() const
  {
    return m_ProjectionViewMatrix;
  }
  glm::vec2 screenToWorld(const glm::vec2& screenCoords,
                          const sf::Window& window);

  void update(float deltaTime, const sf::Window& window);
  void setPosition(const glm::vec2& pos) { m_Position = pos; }
  const glm::vec2& getPosition() const { return m_Position; }

  sf::Vector2f getSize() const { return sf::Vector2f(m_Width, m_Height); }

  void addZoom(float delta);

  void startPanning(const glm::vec2& mousePos);
  void updatePanning(const glm::vec2& mousePos);
  void stopPanning();

  float getZoom() const { return m_Zoom; }

 private:
  void recalculateViewMatrix();

  glm::vec2 m_Position;
  glm::vec2 m_PanStartMousePos;
  glm::vec2 m_PanStartCameraPos;

  glm::mat4 m_ProjectionMatrix;
  glm::mat4 m_ViewMatrix;
  glm::mat4 m_ProjectionViewMatrix;

  float m_Zoom;
  float m_Width, m_Height;
  float m_TargetZoom = 1.0f;

  bool m_IsPanning = false;
};
