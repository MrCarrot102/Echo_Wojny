#include "Game/Villager.h"
#include "Game/GameState.h"
#include "Game/Pathfinder.h"
#include "Game/Building.h"

#include <iostream> 


Villager::Villager(const std::string& n, const glm::vec2& pos) 
    : name(n), 
    position(pos),
    currentState(State::IDLE), 
    targetPosition(pos), 
    targetNode(nullptr), 
    workTimer(0.0f), 
    hunger(100.0f),
    thirst(100.0f), 
    carryingResourceType(ResourceNode::Type::NONE), 
    carryingAmount(0), 
    currentPathIndex(0)
{}

// glowna funkcja aktualizujaca 
void Villager::update(float deltaTime, GameState& world) {
    updateNeeds(deltaTime); 
    think(world); 
    act(deltaTime, world); 
}

// fizjologia 
void Villager::updateNeeds(float deltaTime) { 
    const float HUNGER_RATE = 2.0f;
    const float THIRST_RATE = 3.0f; 

    hunger -= (deltaTime * 1.0f * HUNGER_RATE); 
    thirst -= (deltaTime * 1.0f * THIRST_RATE); 

    if (hunger < 0.0f) hunger = 0.0f; 
    if (thirst < 0.0f) thirst = 0.0f; 
}

// mozg decyzje itd 
void Villager::think(GameState& world) {
    const float CRITICAL_THIRST = 40.0f; 
    const float CRITICAL_HUNGER = 30.0f; 

    // priorytet 1 woda 
    if (thirst < CRITICAL_THIRST && 
        currentState != State::DRINKING && 
        currentState != State::MOVING_TO_DRINK)
    {
            Building* targetWell = nullptr; 
            for (Building& b : world.m_buildings) {
                if (b.buildingType == Building::WELL) { targetWell = &b; break; }
            }

            if (targetWell) 
            { 
                std::cout << "[AI] " << name << " rzuca prace (WODA)!\n";
                currentState = State::MOVING_TO_DRINK;
                targetPosition = targetWell->position; 
                currentPath.clear(); 
            }
    }

    // priorytet 2 glod
    else if (hunger < CRITICAL_HUNGER && currentState != State::EATING && currentState != State::MOVING_TO_EAT)
    { 
        Building* targetKitchen = nullptr; 
        for (Building& b : world.m_buildings)
        {
            if (b.buildingType == Building::KITCHEN) { targetKitchen = &b; break; }
        }

        if (targetKitchen) 
        { 
            std::cout << "[AI] " << name << " rzuca prace (JEDZENIE)!\n";
            currentState = State::MOVING_TO_EAT;
            targetPosition = targetKitchen->position; 
            currentPath.clear(); 
        }
    }
}


// ciala wykonywanie ruchu i akcji 
void Villager::act(float deltaTime, GameState& world) 
{
    float speed = 60.0f; 
    const float GATHER_TIME = 3.0f; 

    if (currentState == State::MOVING_TO_POINT) 
    {
        if (moveOnPath(deltaTime, 1.0f, speed, world)) 
        {
            currentState = State::IDLE; 
        }
    }

    else if (currentState = State::MOVING_TO_WORK)
    {
        if (!targetNode) { currentState = State::IDLE; return; }
        if (moveOnPath(deltaTime, 2.0f, speed, world))
        {
            currentState = State::GATHERING; 
            workTimer = GATHER_TIME; 
        }
    }
   
    else if (currentState == State::MOVING_TO_EAT) 
    {
        if (moveOnPath(deltaTime, 10.0f, speed, world))
        {
            currentState = State::EATING;
            workTimer = 2.0f; 
        }
    }      
       

    else if (currentState == State::MOVING_TO_EAT) 
    {
        if (moveOnPath(deltaTime, 10.0f, speed, world))
        {
            currentState = State::EATING;
            workTimer = 2.0f; 
        }
    }

    else if (currentState == State::MOVING_TO_DRINK) 
    {
        if (moveOnPath(deltaTime, 10.0f, speed, world))
        {
            currentState = State::DRINKING;
            workTimer = 2.0f; 
        }
    }

    else if (currentState == State::MOVING_TO_HAUL) 
    {
        if (moveOnPath(deltaTime, 5.0f, speed, world)) 
        {
            std::cout << "[AI] " << name << " zrzucił zasoby.\n";
            if (carryingResourceType == ResourceNode::Type::TREE) world.globalWood += carryingAmount; 

            carryingAmount = 0; 
            carryingResourceType = ResourceNode::Type::NONE; 

            if (targetNode && targetNode->amountLeft > 0)
            {
                currentState = State::MOVING_TO_WORK; 
                targetPosition = targetNode->position; 
                currentPath.clear();
            }
            else 
            { 
                currentState = State::IDLE; 
            }
        }
    }

    else if (currentState == State::GATHERING) 
    {
        if (!targetNode) { currentState = State::IDLE; return; }
        workTimer -= deltaTime; 

        if (workTimer <= 0.0f)
        {

            std::cout << "\n[AI] " << name << " skończył pracę!\n"; 

            carryingAmount = 10; 
            carryingResourceType = targetNode->resourceType;
            targetNode->amountLeft -= 10; 
            if (targetNode->amountLeft <= 0) targetNode = nullptr; 

            Building* stockpile = world.findNearestStockpile(position); 
            if (stockpile) 
            {
                currentState = State::MOVING_TO_HAUL; 
                targetPosition = stockpile->position;
                currentPath.clear();
            }
            else 
            {
                carryingAmount = 0;
                currentState = State::IDLE; 
            }
        }
    }

    else if (currentState == State::EATING) 
    {
        workTimer -= deltaTime; 
        if (workTimer <= 0.0f) 
        {
            if (world.globalFood >= 10)
            {
                world.globalFood -= 10; 
                hunger = 100.0f; 
            }
            currentState = State::IDLE; 
        }
    }

    else if (currentState == State::DRINKING) 
    {
        workTimer -= deltaTime; 
        if (workTimer <= 0.0f) 
        {
            if (world.globalWater >= 10)
            {
                world.globalWater -= 10; 
                thirst = 100.0f; 
            }
            currentState = State::IDLE; 
        }
    }
}

// helper do ruchu 
bool Villager::moveOnPath(float deltaTime, float reachDistance, float speed, GameState& world) 
{
    if (currentPath.empty())
    {
        float directDist = glm::distance(position, targetPosition); 
        if (directDist < 20.0f)
        {
            glm::vec2 dir = targetPosition - position; 
            if (glm::length(dir) > 0.1f)
                position += glm::normalize(dir) * speed * deltaTime; 
            
                if (directDist <= reachDistance) return true; 
                return false; 
        }

        currentPath = Pathfinder::findPath(position, targetPosition, *world.m_worldMap);
        currentPathIndex = 0;

        if (currentPath.empty()) 
        {
            currentState = State::IDLE; 
            return false;
        }
    }

    if (currentPathIndex < currentPath.size()) 
    {
        glm::vec2 nextWaypoint = currentPath[currentPathIndex]; 
        glm::vec2 direction = nextWaypoint - position; 
        float distToWaypoint = glm::length(direction); 

        if (distToWaypoint > 2.0f)
        {
            position += glm::normalize(direction) * speed * deltaTime; 
        }
        else 
        {
            currentPathIndex++; 
        }
        return false; 
    }
    else 
    {
        glm::vec2 direction = targetPosition - position; 
        float distToTarget = glm::length(direction); 

        if (distToTarget > reachDistance)
        {
            position += glm::normalize(direction) * speed * deltaTime; 
            return false; 
        }
        else 
        {
            currentPath.clear(); 
            currentPathIndex = 0;
            return true;
        }
    }
    return false; 
}