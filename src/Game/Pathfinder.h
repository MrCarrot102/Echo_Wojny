#pragma once
#include <glm/glm.hpp>
#include <vector>

#include "Game/WorldMap.h"

struct Node
{
  glm::ivec2 pos;  // zbieranie pozycji
  int gCost;       // koszt od startu
  int hCost;       // heurystyka szacowany koszt do celu
  int fCost() const { return gCost + hCost; }

  Node* parent;  // skad przyszliscmy

  Node(glm::ivec2 p) : pos(p), gCost(0), hCost(0), parent(nullptr) {}
};

class Pathfinder
{
 public:
  // zwracanie wektorow punktow w swiecie do omijania
  static std::vector<glm::vec2> findPath(const glm::vec2& startWorld,
                                         const glm::vec2& endWorld,
                                         const WorldMap& map);
};