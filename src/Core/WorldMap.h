#pragma once 
#include <vector> 
#include <glm/glm.hpp> 

struct Tile {
    bool isWall; 

    Tile() : isWall(false) {} 
};

class WorldMap {
    public: 
        WorldMap(int width, int height, float tileSize); 

        // funkcje pomocnicze 
        void setObstacle(int x, int y, bool isObstacle); 
        bool isBlocked(int x, int y) const; 

        // konwersja z swiata na siatke 
        glm::ivec2 worldToGrid(const glm::vec2& worldPos) const; 
        glm::vec2 gridToWorld(const glm::ivec2& gridPos) const; 

        // gettery 
        int getWidth() const {return m_width; }
        int getHeight() const {return m_height; }
        float getTileSize() const {return m_tileSize; }

    private: 
        int m_width; 
        int m_height; 
        float m_tileSize; 
        std::vector<Tile> m_grid; // tablica jednowymiarowa 2d (dla wydajnosc)

};
