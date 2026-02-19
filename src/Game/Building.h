#pragma once
#include <glm/glm.hpp>

struct Building
{
  glm::vec2 position;

  enum Type
  {
    TENT,
    KITCHEN,
    WELL,
    STOCKPILE,
    CAMPFIRE,
    WALL,
    STONE_WELL,
    WOODEN_BED,
    STONE_BED
  };

  Type buildingType;

  Building(Type type, const glm::vec2& pos) : buildingType(type), position(pos)
  {
  }
};