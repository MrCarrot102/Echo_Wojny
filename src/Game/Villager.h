#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "Game/ResourceNode.h"

struct ResourceNode;

struct Villager
{
  glm::vec2 position;
  std::string name;

  enum State
  {
    IDLE,
    MOVING_TO_POINT,
    MOVING_TO_WORK,
    GATHERING,
    MOVING_TO_EAT,
    EATING,
    MOVING_TO_DRINK,
    DRINKING,
    MOVING_TO_HAUL,
    HAULING,

    MOVING_TO_GATHER_WATER,
    GATHERING_WATER,

    MOVING_TO_BED,
    SLEEPING,

    COMBAT
  };

  State currentState;

  float workTimer;
  float hunger;
  float thirst;
  float energy = 100.0f;

  float attackTimer = 0.0f;
  float damage = 5.0f;

  float health;
  glm::vec2 targetPosition;
  std::vector<glm::vec2> currentPath;
  int currentPathIndex;

  ResourceNode::Type carryingResourceType;
  int carryingAmount;

  ResourceNode* targetNode;

  Villager(const std::string& n, const glm::vec2& pos)
      : name(n),
        position(pos),
        currentState(State::IDLE),
        targetPosition(pos),
        targetNode(nullptr),
        workTimer(0.0f),
        hunger(100.0f),
        thirst(100.0f),
        health(100.0f),
        carryingResourceType(ResourceNode::Type::NONE),
        carryingAmount(0),
        currentPathIndex(0)
  {
  }
};