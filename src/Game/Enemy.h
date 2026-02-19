#pragma once
#include <glm/glm.hpp>
#include <vector>

struct Enemy
{
  glm::vec2 position;

  enum State
  {
    PATROL,
    CHASE,
    ATTACKING
  };
  State currentState;

  glm::vec2 targetPosition;
  std::vector<glm::vec2> currentPath;
  int currentPathIndex;

  float speed;
  float aggroRange;

  float health;
  float maxHealth;
  float damage;
  float attackRange;
  float attackTimer;
  float attackCooldown;

  Enemy(glm::vec2 pos)
      : position(pos),
        currentState(State::PATROL),
        targetPosition(pos),
        currentPathIndex(0),
        speed(70.0f),
        aggroRange(300.0f),
        health(50.0f),
        maxHealth(50.0f),
        damage(10.0f),
        attackRange(20.0f),
        attackTimer(0.0f),
        attackCooldown(1.5f)
  {
  }
};