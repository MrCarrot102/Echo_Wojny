#include "Game/Pathfinder.h"

#include <algorithm>
#include <cmath> 
#include <set> 
#include <vector>


// pomocnicze do obliczania dystansu (heurystyka mangattan) 
int getDistance(glm::ivec2 a, glm::ivec2 b){
    return std::abs(a.x - b.x) + std::abs(a.y - b.y); 
}

std::vector<glm::vec2> Pathfinder::findPath(const glm::vec2& startWorld, const glm::vec2& endWorld, const WorldMap& map) {
    std::vector<glm::vec2> path; 

    // konwersja swiata na siatke 
    glm::ivec2 startNodePos = map.worldToGrid(startWorld); 
    glm::ivec2 endNodePos = map.worldToGrid(endWorld); 

    // ochrona jak jest w scianie 
    /*
    if (map.isBlocked(endNodePos.x, endNodePos.y)) {
        return path; 
    }
    */

    std::vector<Node*> openSet; 
    std::vector<Node*> closedSet; 

    openSet.push_back(new Node(startNodePos)); 

    Node* targetNode = nullptr; 

    // glowna petla a*
    while (!openSet.empty()){
        // szukanie wezla z najnizszym fcost 
        Node* currentNode = openSet[0]; 
        for (size_t i = 1; i <openSet.size(); i++){
            if (openSet[i]->fCost() < currentNode->fCost() ||
                    (openSet[i]->fCost() == currentNode->fCost() && openSet[i]->hCost < currentNode->hCost)) {
                    currentNode = openSet[i]; 
            }
        }



        // usuwanie z openset i dodawanie do closedset 
        openSet.erase(std::remove(openSet.begin(), openSet.end(), currentNode), openSet.end()); 
        closedSet.push_back(currentNode); 

        // sprawdzanie czy dotarlismy 
        if (currentNode->pos == endNodePos){
            targetNode = currentNode; 
            break; 
        }

        glm::ivec2 neighbors[4] = { {0, 1}, {0, -1}, {1, 0}, {-1, 0} }; 


        for (const auto& offset : neighbors){ 
            glm::ivec2 neighborPos = currentNode->pos + offset; 
            
            // jesli sciana lub juz w closedset pomijamy 
            if (map.isBlocked(neighborPos.x, neighborPos.y) && neighborPos != endNodePos) continue;

            bool inClosed = false; 
            for (Node* n : closedSet) { if (n->pos == neighborPos) { inClosed = true; break; }}
            if (inClosed) continue;

            // obliczanie kosztu 
            int newMovementCostToNeighbor = currentNode->gCost + getDistance(currentNode->pos, neighborPos); 

            // sprawdzanie czy jest w openset 
            Node* neighborNode = nullptr; 
            for (Node* n : openSet) { if (n->pos == neighborPos) { neighborNode = n; break; }}

            if (newMovementCostToNeighbor < (neighborNode ? neighborNode->gCost : 999999) || neighborNode == nullptr) { 
                if (neighborNode == nullptr){ 
                    neighborNode = new Node(neighborPos); 
                    openSet.push_back(neighborNode); 
                }

                neighborNode->gCost = newMovementCostToNeighbor; 
                neighborNode->hCost = getDistance(neighborPos, endNodePos); 
                neighborNode->parent = currentNode; 
            } 
        }
    }

    // odtwarzanie sciezki 
    if (targetNode != nullptr) {
        Node* curr = targetNode; 
        while (curr != nullptr) {
            path.push_back(map.gridToWorld(curr->pos)); 
            curr = curr->parent; 
        }
        std::reverse(path.begin(), path.end()); 
    }

    for (Node* n : openSet) delete n; 
    for (Node* n : closedSet) delete n; 

    return path; 

}