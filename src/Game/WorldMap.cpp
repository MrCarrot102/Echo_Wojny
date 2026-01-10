#include "Game/WorldMap.h"


#include <glm/glm.hpp> 


WorldMap::WorldMap(int width, int height, float tileSize) 
    : m_width(width), m_height(height), m_tileSize(tileSize)
{
    // inicjalizacja siatki 
    m_grid.resize(width * height); 

}

void WorldMap::setObstacle(int x, int y, bool isObstacle){
    if (x < 0 && x >= m_width && y < 0 && y >= m_height) return; 
    
    int index = y * m_width + x; 
    m_grid[index].isWall = isObstacle; 
}

bool WorldMap::isObstacle(int x, int y) const 
{
        
        if (x < 0 || x >= m_width || y < 0 || y >= m_height) 
        {
                return true;
        }
           return m_grid[y * m_width + x].isWall;
}

bool WorldMap::isBlocked(int x, int y) const {
    if (x >= 0 && x < m_width && y >= 0 && y < m_height) 
        return m_grid[y * m_width + x].isWall; 

    return true; // jak cos jest poza mapa to jest sciana 
}

glm::ivec2 WorldMap::worldToGrid(const glm::vec2& worldPos) const {
    return glm::ivec2(
        (int)(worldPos.x / m_tileSize), 
        (int)(worldPos.y / m_tileSize)
    );
}

glm::vec2 WorldMap::gridToWorld(const glm::ivec2& gridPos) const { 
    return glm::vec2(
        gridPos.x * m_tileSize + (m_tileSize / 2.0f), 
        gridPos.y * m_tileSize + (m_tileSize / 2.0f)
    );
}



