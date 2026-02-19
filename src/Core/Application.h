#pragma once

#include <GL/glew.h>
#include <imgui-SFML.h>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <glm/glm.hpp>
#include <memory>

#include "Game/GameState.h"
#include "Game/Villager.h"
#include "Rendering/Camera2D.h"
#include "Rendering/PrimitiveRenderer.h"
#include "imgui.h"

class Application
{
 public:
  Application();
  ~Application();

  void run();

 private:
  void init();
  void pollEvents();
  void update(float deltaTime);
  void render();

  // stany aplikacji
  enum class AppState
  {
    MENU,
    GAME,
    GAME_OVER,
    PAUSE_MENU
  };

  enum class BuildMode
  {
    NONE,
    KITCHEN,
    WELL,
    STOCKPILE,
    CAMPFIRE,
    STONE_WELL,
    WALL,
    WOODEN_BED,
    STONE_BED
  };

  // -- oswietlenie --
  sf::RenderTexture m_lightMapTexture;
  sf::Sprite m_lightMapSprite;
  sf::RenderWindow m_Window;
  sf::Clock m_DeltaClock;

  // smart pointery do kontroli czasu zycia opiektow w opengl BO MOGE
  std::unique_ptr<Camera2D> m_Camera;
  std::unique_ptr<PrimitiveRenderer> m_Renderer;
  std::unique_ptr<GameState> m_GameState;

  AppState m_AppState = AppState::MENU;
  char m_saveFilename[128] = "save.txt";

  Villager* m_selectedVillager;
  BuildMode m_currentBuildMode = BuildMode::NONE;
  glm::vec2 m_ghostBuildingPos;

  bool m_Running;
  float m_timeScale = 1.0f;
};