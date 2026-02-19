#pragma once
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "Game/Building.h"
#include "Game/Enemy.h"
#include "Game/ResourceNode.h"
#include "Game/Villager.h"
#include "Game/WorldMap.h"

class GameState
{
 public:
  GameState();

  enum class Mode
  {
    PLAYING,
    EVENT_PAUSED
  };

  // system pogody
  enum Season
  {
    SUMMER,
    WINTER
  };
  Season currentSeason;

  float globalTemperature;
  float seasonTimer;
  float heatingTimer;

  // glowna funkcja ticki symulacji
  void update(float deltaTime);
  void checkForDailyEvents();
  Building* findNearestStockpile(const glm::vec2& fromPos);
  void setMode(Mode newMode);
  Mode getMode() const { return m_currentMode; }
  void resolveRefugeeEvent(bool accepted);

  // dane gry
  int dayCounter;
  float timeOfDay;
  int globalFood;
  int globalWood;
  int globalWater;
  int globalStone;
  int globalMorale;
  Mode m_currentMode;
  std::string currentEventTitle;
  std::string currentEventDescription;

  // zapisywanie gry
  void saveGame(const std::string& filename);
  void loadGame(const std::string& filename);

  // npc
  std::vector<Villager> m_villagers;
  std::vector<ResourceNode> m_resourceNodes;
  std::vector<Building> m_buildings;
  std::vector<Enemy> m_enemies;

  void updateCombat(float deltaTime);

  std::unique_ptr<WorldMap> m_worldMap;

 private:
  // liczenie czasu
  float m_TimeAccumulator;

  // wydarzenia
  bool m_eventDay3Triggered;
  bool m_eventDay5Triggered;
  bool m_eventRefugeesTriggered;
};
