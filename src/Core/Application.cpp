#include "Core/Application.h"
#include "Game/Pathfinder.h"

#include <SFML/Graphics.hpp> 
#include <iostream> 
#include <ctime> 

Application::Application()
    : m_Window(), m_Running(true), m_selectedVillager(nullptr)
{
    init(); 
}

Application::~Application() 
{
    ImGui::SFML::Shutdown(); 
}

void Application::init() {
    // konfigurowanie okna 
    sf::ContextSettings settings; 
    settings.depthBits = 24; 
    settings.stencilBits = 8; 
    settings.majorVersion = 3; 
    settings.minorVersion = 3; 
    settings.attributeFlags = sf::ContextSettings::Default; 
    unsigned int windowWidth = 800; 
    unsigned int windowHeight = 600; 
    std::srand(static_cast<unsigned int>(time(NULL))); 

    m_Window.create(sf::VideoMode(windowWidth, windowHeight), "Echo Wojny", sf::Style::Default, settings); 
    m_Window.setVerticalSyncEnabled(true); 

    // -- inicjalizacja imgui -- 
    // Rzutujemy rozmiar na Vector2f
    if (!ImGui::SFML::Init(m_Window, static_cast<sf::Vector2f>(m_Window.getSize())))
    {
        std::cerr << "Nie udało się zainicjować ImGui!\n";
    }

    // -- inicjiowanie glew --
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Nie udało się zainicjować GLEW!" << std::endl;
        m_Running = false; 
        return; 
    }
    // -- inicjalizacja oswietlenia -- 
    if (!m_lightMapTexture.create(windowWidth, windowHeight)) 
    {
        std::cerr << "Blad tworzenia LightMapy!\n";
    }

    // tworzenie obieku rdzenia 
    m_Camera = std::make_unique<Camera2D>((float)windowWidth, (float)windowHeight); 
    m_Camera->setPosition(glm::vec2(2000.0f, 2000.0f));
    m_Renderer = std::make_unique<PrimitiveRenderer>(); 
    m_GameState = std::make_unique<GameState>(); 

    

    // ustawianie koloru tla
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}


void Application::run() 
{
    while(m_Running) 
    {
        float deltaTime = m_DeltaClock.restart().asSeconds(); 

        pollEvents(); 
        update(deltaTime); 
        render(); 
    }
}

void Application::pollEvents() 
{
    sf::Event event; 
    while (m_Window.pollEvent(event)) 
    {
        
        ImGui::SFML::ProcessEvent(m_Window, event);         


        if (event.type == sf::Event::Closed){
            m_Running = false; 
        }

        if (m_AppState == AppState::MENU) continue; 

        auto& io = ImGui::GetIO(); 

        if (io.WantCaptureMouse && (event.type == sf::Event::MouseButtonPressed || event.type == sf::Event::MouseWheelScrolled)){
            continue;
        }

        if (io.WantCaptureKeyboard && event.type == sf::Event::KeyPressed) {
            continue;
        }

        
        
        if( event.type == sf::Event::KeyPressed) {
            if (m_GameState->getMode() == GameState::Mode::EVENT_PAUSED){

                if (event.key.code == sf::Keyboard::T){
                    m_GameState->resolveRefugeeEvent(true); 
                } else if (event.key.code == sf::Keyboard::N){
                    m_GameState->resolveRefugeeEvent(false); 
                }
            } else {

                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape){
                    m_Running = false; 
                }

                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::B){
                    if(m_currentBuildMode == BuildMode::NONE){
                        m_currentBuildMode = BuildMode::KITCHEN; 
                        std::cout << "Tryb budowana: KUCHNIA\n";
                        m_selectedVillager = nullptr; // odznaczanie mieszkanca 
                    } else {
                        m_currentBuildMode = BuildMode::NONE; // wylaczanie trybu budowania 
                        std::cout << "Wyłączono tryb budowania\n";
                    }
                }

                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::N)
                {
                    if (m_currentBuildMode == BuildMode::NONE)
                    {
                        m_currentBuildMode = BuildMode::WELL;
                        std::cout << "Tryb budowania: STUDNIA\n";
                        m_selectedVillager = nullptr; 
                    } 
                    
                    else 
                    {
                        m_currentBuildMode = BuildMode::NONE; 
                        std::cout << "Wyłączono tryb budowania\n";
                    }
                }

                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::P)
                {
                    if (m_currentBuildMode == BuildMode::NONE)
                    {
                        m_currentBuildMode = BuildMode::STOCKPILE; 
                        std::cout << "Tryb budowania: MAGAZYN\n";
                        m_selectedVillager = nullptr; 
                    } 
                    
                    else 
                    {
                        m_currentBuildMode = BuildMode::NONE; 
                        std::cout << "Wyłączono tryb budowania\n";
                    }
                }
            }
        }
        
        else if (event.type == sf::Event::MouseButtonPressed) 
        {
            // pobbieranie funkcji myszy 
            glm::vec2 screenMousePos = {(float)event.mouseButton.x, (float)event.mouseButton.y};
            // konwersja na pozycje w swiecie 
            glm::vec2 worldMousePos = m_Camera->screenToWorld(screenMousePos, m_Window); 

            if (event.mouseButton.button == sf::Mouse::Left)
            {
                if (m_currentBuildMode != BuildMode::NONE)
                {
                    // === JESTEŚMY W TRYBIE BUDOWANIA ===
                    // budowanie kuchni 
                    if (m_currentBuildMode == BuildMode::KITCHEN)
                    {
                        int woodCost = 50;
                        if (m_GameState->globalWood >= woodCost)
                        {
                            m_GameState->globalWood -= woodCost;

                            m_GameState->m_buildings.emplace_back(Building::KITCHEN, m_ghostBuildingPos);

                            glm::ivec2 centerGrid = m_GameState->m_worldMap->worldToGrid(m_ghostBuildingPos);
                            
                            for (int x = -1; x <= 1; x++)
                            {
                                for (int y = -1; y <= 1; y++)
                                {
                                    m_GameState->m_worldMap->setObstacle(centerGrid.x + x, centerGrid.y + y, true); 
                                }
                            }

                            std::cout << "[MAPA] Zablokowano kratkę: " << centerGrid.x << ", " << centerGrid.y << "\n";
                            // ----------------------------------------

                            m_currentBuildMode = BuildMode::NONE;
                        } 
                        
                        else 
                        {
                            std::cout << "[BUDOWA] Potrzeba więcej drewna mój Panie, a dokładnie " << woodCost << std::endl; 
                        }
                    }
                    
                    // budowanie studni 
                    else if (m_currentBuildMode == BuildMode::WELL)
                    {
                        int woodCost = 25; 
                        if(m_GameState->globalWood >= woodCost)
                        {
                            m_GameState->globalWood -= woodCost; 

                            m_GameState->m_buildings.emplace_back(Building::WELL, m_ghostBuildingPos); 

                            glm::ivec2 centerGrid = m_GameState->m_worldMap->worldToGrid(m_ghostBuildingPos);
                            
                            for (int x = -1; x <= 1; x++)
                            {
                                for (int y = -1; y <= 1; y++)
                                {
                                    m_GameState->m_worldMap->setObstacle(centerGrid.x + x, centerGrid.y + y, true); 
                                }
                            }

                            std::cout << "[MAPA] Zbudowano kratkę: " << centerGrid.x << ", " << centerGrid.y << "\n";

                            m_currentBuildMode = BuildMode::NONE;

                        }
                        else 
                        {
                            std::cout << "[BUDOWA] Za mało drewna! Potrzeba " << woodCost << std::endl; 
                        }
                    }
                    
                    // budowanie magazynu 
                    else if (m_currentBuildMode == BuildMode::STOCKPILE) 
                    {
                        int woodCost = 10; 
                        if (m_GameState->globalWood >= woodCost)
                        {
                            m_GameState->globalWood -= woodCost; 
                            
                            m_GameState->m_buildings.emplace_back(Building::STOCKPILE, m_ghostBuildingPos); 

                            glm::ivec2 centerGrid = m_GameState->m_worldMap->worldToGrid(m_ghostBuildingPos);
                            
                            for (int x = -1; x <= 1; x++)
                            {
                                for (int y = -1; y <= 1; y++)
                                {
                                    m_GameState->m_worldMap->setObstacle(centerGrid.x + x, centerGrid.y + y, true); 
                                }
                            }
                            std::cout << "[MAPA] Zbudowano kartkę: " << centerGrid.x << ", " << centerGrid.y << "\n";

                            m_currentBuildMode = BuildMode::NONE;

                        } 
                        else 
                        {
                            std::cout << "[BUDOWA] Za mało drewna na magazyn! Potrzeba " << woodCost << std::endl; 
                        }
                    }

                    // budowanie ogniska 
                    else if (m_currentBuildMode == BuildMode::CAMPFIRE)
                    {
                        int woodCost = 30; 
                        if (m_GameState->globalWood >= woodCost)
                        {
                            m_GameState->globalWood -= woodCost; 
                            m_GameState->m_buildings.emplace_back(Building::CAMPFIRE, m_ghostBuildingPos); 
                            
                            glm::ivec2 centerGrid = m_GameState->m_worldMap->worldToGrid(m_ghostBuildingPos);
                            
                            for (int x = -1; x <= 1; x++)
                            {
                                for (int y = -1; y <= 1; y++)
                                {
                                    m_GameState->m_worldMap->setObstacle(centerGrid.x + x, centerGrid.y + y, true); 
                                }
                            }

                            std::cout << "[BUDOWA] Rozpalono Ognisko!\n";
                            m_currentBuildMode = BuildMode::NONE; 
                        }
                        else 
                        {
                            std::cout << "[BUDOWA] Brakuje drewna mój Panie!\n";
                        }
                    } 
                    
                    else if (m_currentBuildMode == BuildMode::WALL) 
                    {
                        int cost = 10; 

                        if (m_GameState->globalStone >= cost) 
                        {
                            m_GameState->globalStone -= cost; 
                            m_GameState->m_buildings.emplace_back(Building::WALL, m_ghostBuildingPos); 

                            // blokowanie terenu pod murem 
                            glm::ivec2 centerGrid = m_GameState->m_worldMap->worldToGrid(m_ghostBuildingPos); 
                            for (int x = -1; x <= 1; x++)
                            {
                                for (int y = -1; y <= 1; y++)
                                {
                                    m_GameState->m_worldMap->setObstacle(centerGrid.x + x, centerGrid.y + y, true); 
                                }
                            }
                            std::cout << "Postawiono mur!\n";
                        } else std::cout << "Brakuje kamienia!\n";
                    }

                    else if (m_currentBuildMode == BuildMode::STONE_WELL)
                    {
                        int cost = 50;
                        if (m_GameState->globalStone >= cost) 
                        {
                            m_GameState->globalStone -= cost; 
                            m_GameState->m_buildings.emplace_back(Building::STONE_WELL, m_ghostBuildingPos); 

                            glm::ivec2 centerGrid = m_GameState->m_worldMap->worldToGrid(m_ghostBuildingPos); 
                            for (int x = -1; x <= 1; x++)
                            {
                                for (int y = -1; y <= 1; y++)
                                {
                                    m_GameState->m_worldMap->setObstacle(centerGrid.x + x, centerGrid.y + y, true); 
                                }
                            }
                            m_currentBuildMode = BuildMode::NONE; 
                            std::cout << "Postawiono kamienną studnię!\n";
                        } else std::cout << "Brakuje kamienia!\n";
                    }

                    else if (m_currentBuildMode == BuildMode::WOODEN_BED)
                    {
                        int cost = 15;
                        if (m_GameState->globalWood >= cost) 
                        {
                            m_GameState->globalWood -= cost; 
                            m_GameState->m_buildings.emplace_back(Building::WOODEN_BED, m_ghostBuildingPos); 

                            glm::ivec2 centerGrid = m_GameState->m_worldMap->worldToGrid(m_ghostBuildingPos); 
                            for (int x = -1; x <= 1; x++)
                            {
                                for (int y = -1; y <= 1; y++)
                                {
                                    m_GameState->m_worldMap->setObstacle(centerGrid.x + x, centerGrid.y + y, true); 
                                }
                            }
                            m_currentBuildMode = BuildMode::NONE; 
                            std::cout << "Postawiono drewniane loze!\n";
                        } else std::cout << "Brakuje drewna!\n";
                    }

                    else if (m_currentBuildMode == BuildMode::STONE_BED)
                    {
                        int stoneCost = 10;
                        int woodCost = 10;
                        if (m_GameState->globalStone >= stoneCost && m_GameState->globalWood >= woodCost) 
                        {
                            m_GameState->globalStone -= stoneCost; 
                            m_GameState->globalWood -= woodCost; 
                            m_GameState->m_buildings.emplace_back(Building::STONE_BED, m_ghostBuildingPos); 

                            glm::ivec2 centerGrid = m_GameState->m_worldMap->worldToGrid(m_ghostBuildingPos); 
                            for (int x = -1; x <= 1; x++)
                            {
                                for (int y = -1; y <= 1; y++)
                                {
                                    m_GameState->m_worldMap->setObstacle(centerGrid.x + x, centerGrid.y + y, true); 
                                }
                            }
                            m_currentBuildMode = BuildMode::NONE; 
                            std::cout << "Postawiono krolewskie kamienne loze!\n";
                        } else std::cout << "Brakuje surowcow!\n";
                    }

                } 
                else 
                { 
                    std::cout << "Kliknięto w świat: " << worldMousePos.x << ", " << worldMousePos.y << std::endl; 
                    m_selectedVillager = nullptr; 
                    for (Villager& v : m_GameState->m_villagers)
                    {
                        if (glm::distance(worldMousePos, v.position) < 20.0f)
                        {   
                            m_selectedVillager = &v; 
                            std::cout << "Zaznaczono: " << v.name << std::endl; 
                            break;
                        }
                    }
                }
            } 

            // prawy przycisk 
            else if (event.mouseButton.button == sf::Mouse::Right)
            {
                m_Camera->startPanning({(float)event.mouseButton.x, (float)event.mouseButton.y});
                // === ROZKAZY (PPM) ===
                if (m_selectedVillager != nullptr){
                    

                    sf::Vector2i mousePos = sf::Mouse::getPosition(m_Window);
                    glm::vec2 screenPos = {(float)mousePos.x, (float)mousePos.y};
                    glm::vec2 worldMousePos = m_Camera->screenToWorld(screenPos, m_Window);
                    // ---------------------
                    
                    bool actionFound = false; 
                    glm::vec2 finalTargetPos = worldMousePos;

                    // 1. SPRAWDZANIE BUDYNKÓW
                    for (const auto& building : m_GameState->m_buildings) 
                    {
                        if (glm::distance(building.position, worldMousePos) < 40.0f) 
                        {
                            if (building.buildingType == Building::WELL || building.buildingType == Building::STONE_WELL)
                            {
                                std::cout << "Rozkaz: Pobieraj wodę ze studni!\n";
                                
                                m_selectedVillager->currentState = Villager::State::MOVING_TO_GATHER_WATER;
                                m_selectedVillager->targetNode = nullptr; 
                                m_selectedVillager->carryingAmount = 0; 

                                glm::vec2 dir = glm::normalize(m_selectedVillager->position - building.position);
                                if (glm::length(dir) == 0) dir = glm::vec2(1.0f, 0.0f);
                                
                                finalTargetPos = building.position + (dir * 45.0f); 
                                actionFound = true;
                                break; 
                            }
                        }
                    }

                    // 2. SPRAWDZANIE ZASOBÓW
                    if (!actionFound) {
                        ResourceNode* clickedNode = nullptr; 
                        
                        for (ResourceNode& node : m_GameState->m_resourceNodes)
                        {
                            if (node.amountLeft > 0 && glm::distance(node.position, worldMousePos) < 30.0f) 
                            {
                                clickedNode = &node; 
                                break; 
                            }
                        }

                        if (clickedNode != nullptr)
                        {
                                std::cout << "Rozkaz: Zbieraj zasoby\n";
                                m_selectedVillager->targetNode = clickedNode; 
                                m_selectedVillager->currentState = Villager::State::MOVING_TO_WORK;
                                
                                glm::vec2 dir = glm::normalize(m_selectedVillager->position - clickedNode->position);
                                if(glm::length(m_selectedVillager->position - clickedNode->position) < 0.1f) dir = glm::vec2(1.0f, 0.0f); 

                                float dist = 20.0f; 

                                glm::vec2 bestSpot = clickedNode->position + (dir * 35.0f);
                                bool foundSpot = false;

                                glm::vec2 offsets[] = 
                                {
                                    dir * dist,                    
                                    glm::vec2(dir.y, -dir.x) * dist, 
                                    glm::vec2(-dir.y, dir.x) * dist, 
                                    -dir * dist,                   
                                    glm::vec2(1,0)*35.0f, glm::vec2(-1,0)*dist, glm::vec2(0,1)*35.0f, glm::vec2(0,-1)*dist 
                                };

                                for (const auto& off : offsets) 
                                {
                                    glm::vec2 testPos = clickedNode->position + off;
                                    glm::ivec2 gridPos = m_GameState->m_worldMap->worldToGrid(testPos);

                                    if (!m_GameState->m_worldMap->isObstacle(gridPos.x, gridPos.y)) 
                                    {
                                        bestSpot = testPos;
                                        foundSpot = true;
                                        break;
                                    }
                                }

                                if (!foundSpot && glm::distance(m_selectedVillager->position, clickedNode->position) < 45.0f) 
                                {
                                    bestSpot = m_selectedVillager->position;
                                }

                                finalTargetPos = bestSpot;
                                actionFound = true;
                            }
                    }

                    // 3. ZWYKŁY RUCH
                    if (!actionFound) 
                    {
                        std::cout << "Rozkaz: Ruch\n";
                        m_selectedVillager->targetNode = nullptr; 
                        m_selectedVillager->currentState = Villager::State::MOVING_TO_POINT;
                        finalTargetPos = worldMousePos;
                    }

                    // FINALIZACJA
                    m_selectedVillager->targetPosition = finalTargetPos;

                    m_selectedVillager->currentPath = Pathfinder::findPath(
                        m_selectedVillager->position,   
                        finalTargetPos,                 
                        *m_GameState->m_worldMap        
                    );
                    
                    m_selectedVillager->currentPathIndex = 0; 
                }
            }   
        }
           
        else if (event.type == sf::Event::MouseButtonReleased) 
        {
            if (event.mouseButton.button == sf::Mouse::Right) 
            {
                m_Camera->stopPanning();
            }
        }
            
        else if (event.type == sf::Event::MouseMoved) 
        {
            if (!sf::Mouse::isButtonPressed(sf::Mouse::Right)) 
            {
                m_Camera->stopPanning();
            }

            m_Camera->updatePanning({(float)event.mouseMove.x, (float)event.mouseMove.y});
        }
        
        else if (event.type == sf::Event::MouseWheelScrolled)   
        {
            if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) 
            {
                m_Camera->addZoom(event.mouseWheelScroll.delta * 0.1f);
            }
        }
    }   
}


void Application::update(float deltaTime){
    // --- imgui update ---
    ImGui::SFML::Update(sf::Mouse::getPosition(m_Window), static_cast<sf::Vector2f>(m_Window.getSize()), sf::seconds(deltaTime));
    // -------------------- 

    // -- menu --
    if (m_AppState == AppState::MENU || m_AppState == AppState::GAME_OVER) 
        return; 

    if (m_GameState->m_villagers.empty() || m_GameState->globalMorale <= 0.0f)
    {
        m_AppState = AppState::GAME_OVER;
        return; 
    }

    // aktualizacja logiki gry 
    m_Camera->update(deltaTime, m_Window); 

    // tick w symulacji 
    m_GameState->update(deltaTime * m_timeScale); 

    // sprawdzanie czy wybrany osadnik dalej zyje XD 
    if (m_selectedVillager != nullptr)
    {
        bool stillAlive = false; 
        for (auto& v : m_GameState->m_villagers)
        {
            if (&v == m_selectedVillager)
            {
                stillAlive = true; 
                break; 
            }
        }
        if (!stillAlive)
        {
            m_selectedVillager = nullptr; 
        }
    }

    if (m_currentBuildMode != BuildMode::NONE){
        // aktualizowanie pozycji ducha na podstawie myszki 
        sf::Vector2i screenMousePos = sf::Mouse::getPosition(m_Window); 
        m_ghostBuildingPos = m_Camera->screenToWorld({(float)screenMousePos.x, (float)screenMousePos.y}, m_Window);

    }
}

void Application::render(){
    glClear(GL_COLOR_BUFFER_BIT); 
    float time = m_GameState->timeOfDay;
        glm::vec2 shadowOffset = {0.0f, 0.0f};
        float shadowStretch = 0.0f; // Jak bardzo cień się wydłuża
        bool drawShadows = false;

        if (time >= 6.0f && time <= 18.0f) 
        {
            drawShadows = true;
            // O 6:00 i 18:00 współczynnik wynosi +/- 6.0. W południe wynosi 0.
            float sunAngle = (time - 12.0f); 

            // Offset mówi, jak daleko przesuwa się ŚRODEK cienia
            shadowOffset.x = sunAngle * 1.5f; 
            shadowOffset.y = -2.0f; // Lekko w dół, pod nogi/podstawę

            // Stretch mówi, o ile PIKSELI cień staje się dłuższy (wartość bezwzględna)
            shadowStretch = std::abs(sunAngle) * 2.0f; 
        }

    if (m_AppState == AppState::MENU)
    {
        ImGui::SetNextWindowPos(ImVec2(m_Window.getSize().x * 0.5f, m_Window.getSize().y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(300, 250));
        
        ImGui::Begin("Menu Glowne", nullptr,ImGuiWindowFlags_NoDecoration); 

        ImGui::Text("       Echo Wojny");
        ImGui::Separator(); 

        // przycisk nowa gra 
        if (ImGui::Button("Nowa Gra", ImVec2(280, 40)))
        {
            // resetowanie gry 
            m_GameState = std::make_unique<GameState>(); 
            // resetowanie kamery 
            m_Camera = std::make_unique<Camera2D>((float)m_Window.getSize().x, (float)m_Window.getSize().y);
            m_Camera->setPosition(glm::vec2(2000.0f, 2000.0f)); 
            // przelaczanie stanu gry 
            m_AppState = AppState::GAME; 
        }

        // pole tekstowe i wczytaj 
        ImGui::InputText("Plik", m_saveFilename, 128); 
        if (ImGui::Button("Wczytaj Gre", ImVec2(280, 40)))
        {
            m_GameState->loadGame(m_saveFilename); 
            m_AppState = AppState::GAME; 
        }

        // wyjscie z gry 
        if (ImGui::Button("Wyjdz", ImVec2(280, 40)))
            m_Running = false; 
        
        ImGui::End(); 
    }
    
    
    else if (m_AppState == AppState::GAME)
    {
        // renderowanie gry 
        if (m_GameState->currentSeason == GameState::Season::SUMMER)
        {
            glClearColor(0.1f, 0.3f, 0.1f, 1.0f); 
        }
        else 
        {
            glClearColor(0.8f, 0.8f, 0.9f, 1.0f); 
        }
        glClear(GL_COLOR_BUFFER_BIT); 


        // 1 renderowanie zasobow 
        for (const auto& node : m_GameState->m_resourceNodes) { 
            glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f}; 

            if (node.amountLeft <= 0) continue;

            if (node.resourceType == ResourceNode::TREE)
            {
                color = {0.0f, 0.5f, 0.0f, 1.0f}; // ciemnozielony na drzewo 
            } 
            else if (node.resourceType == ResourceNode::ROCK) 
            {
                color = {0.5f, 0.5f, 0.5f, 1.0f}; // szary na kamienie 
            }
            else if (node.resourceType == ResourceNode::BERRY_BUSH) 
            {
                color = {1.0f, 0.2f, 0.6f, 1.0f}; 
            }
            
           if (drawShadows) {
                glm::vec2 size = {10.0f + shadowStretch, 8.0f}; 
                m_Renderer->drawSquare(*m_Camera, node.position + shadowOffset, size, {0.0f, 0.0f, 0.0f, 0.35f});
            }

            m_Renderer->drawSquare(*m_Camera, node.position, {10.0f, 10.0f}, color);
        }

        // 2 renderowanie mieszkancow 
        m_Window.pushGLStates(); 

        // Pobieramy dane kamery do obliczeń pozycji na ekranie
        glm::vec2 camPos = m_Camera->getPosition();
        sf::Vector2f camSize = m_Camera->getSize();
        sf::Vector2f winSize = static_cast<sf::Vector2f>(m_Window.getSize()); 
        float scaleX = winSize.x / camSize.x;
        float scaleY = winSize.y / camSize.y;

        // 2. renderowanie mieszkancow
        for (const auto& villager : m_GameState->m_villagers) 
        {
            // Domyślny kolor: Czerwony
            glm::vec4 color = {1.0f, 0.2f, 0.2f, 1.0f}; 
        
            // Kolor dla zaznaczonego osadnika: Żółty
            if (m_selectedVillager == &villager) {
                color = {1.0f, 1.0f, 0.0f, 1.0f}; 
            }
            if (drawShadows) {

                glm::vec2 v_shadowPos = villager.position + glm::vec2(shadowOffset.x * 0.5f, -3.0f);
                

                m_Renderer->drawSquare(*m_Camera, v_shadowPos, {8.0f + shadowStretch * 0.5f, 4.0f}, {0.0f, 0.0f, 0.0f, 0.35f});
            }

            if (villager.currentState == Villager::State::SLEEPING) 
            {

                glm::mat4 viewProj = m_Camera->getProjectionViewMatrix();
                glm::vec4 ndc = viewProj * glm::vec4(villager.position.x, villager.position.y, 0.0f, 1.0f);
                
                float screenX = (ndc.x + 1.0f) * 0.5f * m_Window.getSize().x;
                float screenY = (1.0f - ndc.y) * 0.5f * m_Window.getSize().y;

                float floatOffset = std::sin(m_GameState->timeOfDay * 15.0f) * 10.0f;


                ImGui::GetForegroundDrawList()->AddText(
                    ImVec2(screenX - 15.0f, screenY - 25.0f + floatOffset), 
                    IM_COL32(200, 200, 255, 255), 
                    "Zzz..."
                );
            }

            m_Renderer->drawCircle(*m_Camera, villager.position, 6.0f, color);
        }

        m_Window.popGLStates();

        // renderowanie budynkow 
        // renderowanie budynkow 
        for (const auto& b : m_GameState->m_buildings) {
            glm::vec4 color = {0.5f, 0.3f, 0.0f, 1.0f}; // brazowy domyślny
            glm::vec2 size = {20.0f, 20.0f}; // domyślny rozmiar (kwadrat)

            if (b.buildingType == Building::KITCHEN) color = {0.3f, 0.3f, 0.4f, 1.0f}; 
            else if (b.buildingType == Building::WELL) color = {0.0f, 0.4f, 0.9f, 1.0f}; 
            else if (b.buildingType == Building::STOCKPILE) color = {0.45f, 0.25f, 0.1f, 1.0f}; 
            else if (b.buildingType == Building::CAMPFIRE) color = {1.0f, 0.5f, 0.0f, 1.0f}; 
            else if (b.buildingType == Building::WALL) color = {0.4f, 0.4f, 0.4f, 1.0f}; 
            else if (b.buildingType == Building::STONE_WELL) color = {0.0f, 0.6f, 0.7f, 1.0f}; 
            // --- NOWE: ŁÓŻKA (Inny kolor i inny rozmiar!) ---
            else if (b.buildingType == Building::WOODEN_BED) {
                color = {0.6f, 0.4f, 0.2f, 1.0f}; // Jasne drewno
                size = {20.0f, 10.0f}; // Łóżko jest podłużne!
            }
            else if (b.buildingType == Building::STONE_BED) {
                color = {0.5f, 0.3f, 0.6f, 1.0f}; // Fioletowe (królewskie)
                size = {20.0f, 10.0f};
            }

            // CIEŃ BUDYNKÓW I ŁÓŻEK
            if (drawShadows) {
                glm::vec2 shadowSize = {size.x + (shadowStretch * 2.0f), size.y};
                glm::vec2 b_offset = shadowOffset * 2.0f;
                m_Renderer->drawSquare(*m_Camera, b.position + b_offset, shadowSize, {0.0f, 0.0f, 0.0f, 0.35f});
            }

            m_Renderer->drawSquare(*m_Camera, b.position, size, color); 
        }


       // 4. DUCH BUDOWANIA (Podgląd przy myszce)
        if (m_currentBuildMode != BuildMode::NONE) {
            
            // Domyślny kolor i rozmiar
            glm::vec4 ghostColor = {1.0f, 1.0f, 1.0f, 0.5f};
            glm::vec2 size = {20.0f, 20.0f};

            // Ustawienia dla konkretnych budynków
            if (m_currentBuildMode == BuildMode::KITCHEN) { 
                ghostColor = {0.8f, 0.8f, 0.8f, 0.5f}; 
            }
            else if (m_currentBuildMode == BuildMode::WELL) { 
                ghostColor = {0.0f, 0.5f, 1.0f, 0.5f}; 
                size = {15.0f, 15.0f}; 
            }
            else if (m_currentBuildMode == BuildMode::STOCKPILE) { 
                ghostColor = {0.9f, 0.9f, 0.8f, 0.5f}; 
                size = {25.0f, 25.0f}; 
            }
            else if (m_currentBuildMode == BuildMode::CAMPFIRE) { 
                ghostColor = {1.0f, 0.5f, 0.0f, 0.5f}; 
            }
            
            else if (m_currentBuildMode == BuildMode::WALL) { 
                ghostColor = {0.4f, 0.4f, 0.4f, 0.5f}; 
            }
            else if (m_currentBuildMode == BuildMode::STONE_WELL) { 
                ghostColor = {0.0f, 0.8f, 0.9f, 0.5f}; 
            }

            else if (m_currentBuildMode == BuildMode::WOODEN_BED) { 
                ghostColor = {0.6f, 0.4f, 0.2f, 0.5f}; 
                size = {20.0f, 10.0f}; 
            }
            else if (m_currentBuildMode == BuildMode::STONE_BED) { 
                ghostColor = {0.5f, 0.3f, 0.6f, 0.5f}; 
                size = {20.0f, 10.0f};
            }

            m_Renderer->drawSquare(*m_Camera, m_ghostBuildingPos, size, ghostColor);
        }


        // -- dodanie cyklu dobowego i oswietlenia -- 
        // obliczanie jasnosci otoczenia 
        float time = m_GameState->timeOfDay; 
        sf::Uint8 darknessAlpha = 0; 

        if (time >= 20.0f || time < 4.0f) {
            darknessAlpha = 200; 
        }
        else if (time >= 18.0f && time < 20.0f) {
            float factor = (time - 18.0f) / 2.0f; 
            darknessAlpha = (sf::Uint8)(factor * 200); 
        }
        else if (time >= 4.0f && time < 6.0f) {
            float factor = (time - 4.0f) / 2.0f; 
            darknessAlpha = (sf::Uint8)(200 - (factor * 200));
        }
        else {
            darknessAlpha = 0; 
        }

        if (darknessAlpha > 0)
        {
            m_Window.pushGLStates(); 

            m_Window.resetGLStates(); 

            m_lightMapTexture.clear(sf::Color(0, 0, 0, darknessAlpha));
            m_lightMapTexture.setView(m_lightMapTexture.getDefaultView());

            sf::BlendMode eraser(sf::BlendMode::Zero, sf::BlendMode::One, sf::BlendMode::Add, sf::BlendMode::Zero, sf::BlendMode::OneMinusSrcAlpha, sf::BlendMode::Add); 
            sf::CircleShape lightShape; 
            lightShape.setFillColor(sf::Color(255, 255, 255, 255)); 

            glm::vec2 camPos = m_Camera->getPosition();
            sf::Vector2f camSize = m_Camera->getSize();
            sf::Vector2f winSize = static_cast<sf::Vector2f>(m_Window.getSize()); 

            float scaleX = winSize.x / camSize.x;
            float scaleY = winSize.y / camSize.y;

            for (const auto& b : m_GameState->m_buildings)
            {

                glm::mat4 viewProj = m_Camera->getProjectionViewMatrix();
                float currentZoom = m_Camera->getZoom();
                sf::Vector2f winSize = static_cast<sf::Vector2f>(m_Window.getSize()); 

                for (const auto& b : m_GameState->m_buildings)
                {
                    float radius = 0.0f; 

                    
                    if (b.buildingType == Building::CAMPFIRE) radius = 180.0f; 
                    else if (b.buildingType == Building::KITCHEN) radius = 100.0f; 
                    else if (b.buildingType == Building::WELL) radius = 60.0f;
                    else if (b.buildingType == Building::STONE_WELL) radius = 80.0f; 

                    if (radius > 0.0f) 
                    {
                        
                        glm::vec3 centerPos = glm::vec3(b.position.x + 10.0f, b.position.y + 10.0f, 0.0f); 

                        
                        glm::vec4 ndc = viewProj * glm::vec4(centerPos, 1.0f);

                        
                        float screenX = (ndc.x + 1.0f) * 0.5f * winSize.x;
                        
                        float screenY = (1.0f - ndc.y) * 0.5f * winSize.y; 

                      
                        float screenRadius = radius * currentZoom; 

                        lightShape.setRadius(screenRadius); 
                        lightShape.setOrigin(screenRadius, screenRadius); 
                        lightShape.setPosition(screenX, screenY); 

                        m_lightMapTexture.draw(lightShape, eraser); 
                    }
                }
            }

            m_lightMapTexture.display(); 
            m_lightMapSprite.setTexture(m_lightMapTexture.getTexture()); 
            m_lightMapSprite.setPosition(0, 0);
            
            m_Window.setView(m_Window.getDefaultView()); 
            glDisable(GL_DEPTH_TEST);
            m_Window.draw(m_lightMapSprite); 
            
            m_Window.popGLStates(); 
        }
    
        float windowW = (float)m_Window.getSize().x;
        float windowH = (float)m_Window.getSize().y;
        float barHeight = 160.0f; 

        ImGui::SetNextWindowPos(ImVec2(0, windowH - barHeight));
        ImGui::SetNextWindowSize(ImVec2(windowW, barHeight));

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar;

        ImGui::Begin("MainBar", nullptr, windowFlags);


        ImGui::BeginGroup();
        {
            ImGui::TextColored(ImVec4(1, 0.8f, 0, 1), "Dzien %d  |  %02d:00", m_GameState->dayCounter, (int)m_GameState->timeOfDay);
            if (m_GameState->currentSeason == GameState::Season::SUMMER) ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "LATO (%.1f C)", m_GameState->globalTemperature);
            else ImGui::TextColored(ImVec4(0.4f, 0.6f, 1.0f, 1.0f), "ZIMA (%.1f C)", m_GameState->globalTemperature);

            ImGui::Separator();

            ImGui::Text("Drewno: %d", m_GameState->globalWood); ImGui::SameLine(90); ImGui::Text("Kamien: %d", m_GameState->globalStone);
            ImGui::Text("Woda:   %d", m_GameState->globalWater); ImGui::SameLine(90); ImGui::Text("Jedz.:  %d", m_GameState->globalFood);

            ImGui::Spacing();
            ImGui::Text("Morale (%lu os.):", m_GameState->m_villagers.size());
            if (m_GameState->globalMorale > 50.0f) ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.6f, 0.2f, 1.0f, 1.0f)); 
            else ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.3f, 0.3f, 0.3f, 1.0f)); 
            ImGui::ProgressBar(m_GameState->globalMorale / 100.0f, ImVec2(160, 15), "");
            ImGui::PopStyleColor();
        }
        ImGui::EndGroup();

        ImGui::SameLine(0, 15); 


        ImGui::BeginGroup();
        {
            ImGui::Text("PANEL BUDOWANIA:");
            auto DrawBuildButton = [&](const char* label, int cost1, int& pool1, int cost2, int& pool2, BuildMode mode, const char* tooltip) 
            {
                bool isActive = (m_currentBuildMode == mode);
                bool canAfford = (pool1 >= cost1 && pool2 >= cost2); 

                if (isActive) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.7f, 0.0f, 1.0f)); 
                else if (!canAfford) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.2f, 0.2f, 1.0f)); 
                else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.3f, 0.2f, 1.0f)); 

                if (ImGui::Button(label, ImVec2(80, 30))) // <-- ZMNIEJSZONE Z 95x35
                {
                    m_currentBuildMode = isActive ? BuildMode::NONE : mode;
                    m_selectedVillager = nullptr; 
                }
                ImGui::PopStyleColor();
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", tooltip);
            };

            int zero = 0; 
            DrawBuildButton("KUCHNIA", 50, m_GameState->globalWood, 0, zero, BuildMode::KITCHEN, "50 Drewna"); ImGui::SameLine(); 
            DrawBuildButton("STUDNIA", 25, m_GameState->globalWood, 0, zero, BuildMode::WELL, "25 Drewna"); ImGui::SameLine();
            DrawBuildButton("MAGAZYN", 10, m_GameState->globalWood, 0, zero, BuildMode::STOCKPILE, "10 Drewna"); ImGui::SameLine(); 
            DrawBuildButton("OGNISKO", 30, m_GameState->globalWood, 0, zero, BuildMode::CAMPFIRE, "30 Drewna");

            DrawBuildButton("MUR", 10, m_GameState->globalStone, 0, zero, BuildMode::WALL, "10 Kamienia"); ImGui::SameLine();
            DrawBuildButton("K.STUD.", 50, m_GameState->globalStone, 0, zero, BuildMode::STONE_WELL, "50 Kamienia"); ImGui::SameLine();
            DrawBuildButton("LOZE(D)", 15, m_GameState->globalWood, 0, zero, BuildMode::WOODEN_BED, "15 Drewna"); ImGui::SameLine();
            DrawBuildButton("LOZE(K)", 10, m_GameState->globalStone, 10, m_GameState->globalWood, BuildMode::STONE_BED, "10 Kam, 10 Drew");
        }
        ImGui::EndGroup();

        ImGui::SameLine(0, 15);


        float col4_width = 90.0f;
        float col4_posX = windowW - col4_width - 10.0f; 


        float availableWidth = col4_posX - ImGui::GetCursorPosX() - 15.0f; 
        float barWidth = std::max(80.0f, availableWidth); 

        ImGui::BeginGroup();
        {
            if (m_selectedVillager != nullptr) 
            {
                ImGui::TextColored(ImVec4(1,1,0,1), "WYBRANY: %s", m_selectedVillager->name.c_str());

                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.8f, 0.0f, 0.0f, 1.0f));
                ImGui::ProgressBar(m_selectedVillager->health / 100.0f, ImVec2(barWidth, 12), "Zdrowie"); ImGui::PopStyleColor();
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.8f, 0.5f, 0.0f, 1.0f));
                ImGui::ProgressBar(m_selectedVillager->hunger / 100.0f, ImVec2(barWidth, 12), "Glod"); ImGui::PopStyleColor();
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 0.5f, 1.0f, 1.0f));
                ImGui::ProgressBar(m_selectedVillager->thirst / 100.0f, ImVec2(barWidth, 12), "Woda"); ImGui::PopStyleColor();
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.9f, 0.1f, 1.0f));
                ImGui::ProgressBar(m_selectedVillager->energy / 100.0f, ImVec2(barWidth, 12), "Energia"); ImGui::PopStyleColor();
            }
            else 
            {
                ImGui::TextDisabled("Brak celu.");
                ImGui::Spacing(); ImGui::Spacing();
                ImGui::Text("Czas:");

                if (ImGui::Button("||", ImVec2(30, 25))) m_timeScale = 0.0f; ImGui::SameLine();
                if (ImGui::Button(">", ImVec2(30, 25))) m_timeScale = 1.0f; ImGui::SameLine();
                if (ImGui::Button(">>", ImVec2(30, 25))) m_timeScale = 5.0f;
            }
        }
        ImGui::EndGroup();

        ImGui::SameLine(col4_posX); 
        ImGui::BeginGroup();
        {
            if (ImGui::Button("ZAPISZ", ImVec2(90, 30))) m_GameState->saveGame("save.txt");
            if (ImGui::Button("WCZYTAJ", ImVec2(90, 30))) m_GameState->loadGame("save.txt");
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
            if (ImGui::Button("WYJDZ", ImVec2(90, 30))) m_Running = false;
            ImGui::PopStyleColor();
        }
        ImGui::EndGroup();

        ImGui::End();
    }

    if (m_GameState->getMode() == GameState::Mode::EVENT_PAUSED)
    {

        ImGui::SetNextWindowPos(ImVec2(m_Window.getSize().x * 0.5f, m_Window.getSize().y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(400, 250));


        ImGui::Begin("!!! WYDARZENIE !!!", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "%s", m_GameState->currentEventTitle.c_str());
        ImGui::Separator();
        

        ImGui::PushTextWrapPos(380.0f);
        ImGui::Text("%s", m_GameState->currentEventDescription.c_str());
        ImGui::PopTextWrapPos();

        ImGui::Spacing(); ImGui::Spacing(); ImGui::Separator();

        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Wcisnij [T] aby przyjac, lub [N] aby odrzucic.");

        if (ImGui::Button("PRZYJMIJ (T)", ImVec2(180, 40))) 
        {
            m_GameState->resolveRefugeeEvent(true);
        }
        ImGui::SameLine();
        if (ImGui::Button("ODRZUC (N)", ImVec2(180, 40))) 
        {
            m_GameState->resolveRefugeeEvent(false);
        }

            ImGui::End();
    }

    else if (m_AppState == AppState::GAME_OVER)
    {

        glClearColor(0.15f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);


        ImGui::SetNextWindowPos(ImVec2(m_Window.getSize().x * 0.5f, m_Window.getSize().y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(350, 260));

        ImGui::Begin("KONIEC GRY", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);


        auto windowWidth = ImGui::GetWindowSize().x;
        auto textWidth = ImGui::CalcTextSize("TWOJA OSADA UPADLA").x;
        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "TWOJA OSADA UPADLA");
        
        ImGui::Separator();
        ImGui::Spacing();


        if (m_GameState->m_villagers.empty()) 
            ImGui::TextWrapped("Wszyscy osadnicy zgineli z wycienczenia...");
        else 
            ImGui::TextWrapped("Morale spadlo do zera, a osadnicy uciekli...");

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1, 0.8f, 0, 1), "Przetrwano dni: %d", m_GameState->dayCounter);
        ImGui::Spacing(); ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

        if (ImGui::Button("Rozpocznij Nowa Gre", ImVec2(330, 40)))
        {
            m_GameState = std::make_unique<GameState>();
            m_Camera = std::make_unique<Camera2D>((float)m_Window.getSize().x, (float)m_Window.getSize().y);
            m_Camera->setPosition(glm::vec2(2000.0f, 2000.0f)); 
            m_AppState = AppState::GAME; 
        }

        if (ImGui::Button("Wroc do Menu Glownego", ImVec2(330, 40)))
        {
            m_AppState = AppState::MENU;
        }

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.1f, 0.1f, 1.0f));
        if (ImGui::Button("Opusc grę", ImVec2(330, 40)))
        {
            m_Running = false;
        }
        ImGui::PopStyleColor();

        ImGui::End();
    }
  
  
    ImGui::SFML::Render();

    m_Window.display(); 
}
