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

Application::~Application() {
    ImGui::SFML::Shutdown(); 
    // i teraz magia bo smart pointery same sie sprzataja ez 
}

void Application::init() {
    // konfigurowanie okna 
    sf::ContextSettings settings; 
    settings.depthBits = 24; 
    settings.stencilBits = 8; 
    settings.majorVersion = 3; 
    settings.minorVersion = 3; 
    //settings.attributeFlags = sf::ContextSettings::Core; 
    settings.attributeFlags = sf::ContextSettings::Default; 
    unsigned int windowWidth = 800; 
    unsigned int widnowHeight = 600; 
    std::srand(static_cast<unsigned int>(time(NULL))); 

    m_Window.create(sf::VideoMode(windowWidth, widnowHeight), "Echo Wojny", sf::Style::Default, settings); 
    m_Window.setVerticalSyncEnabled(true); 

    // -- inicjalizacja imgui -- 
    // Rzutujemy rozmiar na Vector2f
    if (!ImGui::SFML::Init(m_Window, static_cast<sf::Vector2f>(m_Window.getSize()))){
        std::cerr << "Nie udało się zainicjować ImGui!\n";
    }

    // -- inicjiowanie glew --
    if (glewInit() != GLEW_OK){
        std::cerr << "Nie udało się zainicjować GLEW!" << std::endl;
        m_Running = false; 
        return; 
    }
    // -- inicjalizacja oswietlenia -- 
    if (!m_lightMapTexture.create(windowWidth, windowWidth)) 
    {
        std::cerr << "Blad tworzenia LightMapy!\n";
    }

    // tworzenie obieku rdzenia 
    m_Camera = std::make_unique<Camera2D>((float)windowWidth, (float)widnowHeight); 
    m_Camera->setPosition(glm::vec2(2000.0f, 2000.0f));
    m_Renderer = std::make_unique<PrimitiveRenderer>(); 
    m_GameState = std::make_unique<GameState>(); 

    

    // ustawianie koloru tla
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}


void Application::run() {
    while(m_Running) {
        float deltaTime = m_DeltaClock.restart().asSeconds(); 

        pollEvents(); 
        update(deltaTime); 
        render(); 
    }
}

void Application::pollEvents() {
    sf::Event event; 
    while (m_Window.pollEvent(event)) {
        
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

                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::N){
                    if (m_currentBuildMode == BuildMode::NONE){
                        m_currentBuildMode = BuildMode::WELL;
                        std::cout << "Tryb budowania: STUDNIA\n";
                        m_selectedVillager = nullptr; 
                    } else {
                        m_currentBuildMode = BuildMode::NONE; 
                        std::cout << "Wyłączono tryb budowania\n";
                    }
                }

                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::P){
                    if (m_currentBuildMode == BuildMode::NONE){
                        m_currentBuildMode = BuildMode::STOCKPILE; 
                        std::cout << "Tryb budowania: MAGAZYN\n";
                        m_selectedVillager = nullptr; 
                    } else {
                        m_currentBuildMode = BuildMode::NONE; 
                        std::cout << "Wyłączono tryb budowania\n";
                    }
                }
            }
        }
        
        if (event.type == sf::Event::MouseButtonPressed) {
            // pobbieranie funkcji myszy 
            glm::vec2 screenMousePos = {(float)event.mouseButton.x, (float)event.mouseButton.y};
            // konwersja na pozycje w swiecie 
            glm::vec2 worldMousePos = m_Camera->screenToWorld(screenMousePos, m_Window); 

            // --- POCZĄTEK POPRAWIONEJ LOGIKI ---

            if (event.mouseButton.button == sf::Mouse::Left){
                
                if (m_currentBuildMode != BuildMode::NONE){
                    // === JESTEŚMY W TRYBIE BUDOWANIA ===
                    // budowanie kuchni 
                    if (m_currentBuildMode == BuildMode::KITCHEN){
                        int woodCost = 50;
                        if (m_GameState->globalWood >= woodCost){
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
                        } else {
                            std::cout << "[BUDOWA] Potrzeba więcej drewna mój Panie, a dokładnie " << woodCost << std::endl; 
                        }
                    }
                    
                    // budowanie studni 
                    else if (m_currentBuildMode == BuildMode::WELL){
                        int woodCost = 25; 
                        if(m_GameState->globalWood >= woodCost){
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

                        } else {
                            std::cout << "[BUDOWA] Za mało drewna! Potrzeba " << woodCost << std::endl; 
                        }
                    }
                    
                    // budowanie magazynu 
                    else if (m_currentBuildMode == BuildMode::STOCKPILE){
                        int woodCost = 10; 
                        if (m_GameState->globalWood >= woodCost){
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

                        } else {
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

                } 
                else 
                { 
                    std::cout << "Kliknięto w świat: " << worldMousePos.x << ", " << worldMousePos.y << std::endl; 
                    m_selectedVillager = nullptr; 
                    for (Villager& v : m_GameState->m_villagers){
                        if (worldMousePos.x >= v.position.x && worldMousePos.x <= v.position.x + 10.0f &&
                             worldMousePos.y >= v.position.y && worldMousePos.y <= v.position.y + 10.0f)
                        {   
                            m_selectedVillager = &v; 
                            std::cout << "Zaznaczono: " << v.name << std::endl; 
                            break;
                        }
                    }
                }
            } 
            else if (event.mouseButton.button == sf::Mouse::Right){
                // === ROZKAZY ===
                if (m_selectedVillager != nullptr){
                    ResourceNode* clickedNode = nullptr; 
                    
                    // Szukanie zasobu
                    for (ResourceNode& node : m_GameState->m_resourceNodes){
                        if (node.amountLeft > 0 && glm::distance(node.position, worldMousePos) < 30.0f) {
                            clickedNode = &node; 
                            break; 
                        }
                    }
                    
 
                    glm::vec2 finalTargetPos;

                    if (clickedNode != nullptr){
                            std::cout << "Rozkaz: Zbieraj zasoby\n";
                            m_selectedVillager->targetNode = clickedNode; 
                            m_selectedVillager->currentState = Villager::State::MOVING_TO_WORK;
                            bool actionFound = false; 
                            // 1. Oblicz domyślny kierunek (od zasobu do osadnika)
                            glm::vec2 dir = glm::normalize(m_selectedVillager->position - clickedNode->position);
                            if(glm::length(m_selectedVillager->position - clickedNode->position) < 0.1f) dir = glm::vec2(1.0f, 0.0f); 

                            glm::vec2 bestSpot = clickedNode->position + (dir * 35.0f);
                            bool foundSpot = false;

                            glm::vec2 offsets[] = {
                                dir * 35.0f,                    // 1. Idealnie na wprost
                                glm::vec2(dir.y, -dir.x) * 35.0f, // 2. Bokiem
                                glm::vec2(-dir.y, dir.x) * 35.0f, // 3. Drugim bokiem
                                -dir * 35.0f,                   // 4. Od tyłu
                                glm::vec2(1,0)*35.0f, glm::vec2(-1,0)*35.0f, glm::vec2(0,1)*35.0f, glm::vec2(0,-1)*35.0f // 5. Standardowe kierunki
                            };

                            for (const auto& off : offsets) 
                            {
                                glm::vec2 testPos = clickedNode->position + off;
                                glm::ivec2 gridPos = m_GameState->m_worldMap->worldToGrid(testPos);

                                // Jeśli kratka jest wolna -> Bierzemy to miejsce!
                                if (!m_GameState->m_worldMap->isObstacle(gridPos.x, gridPos.y)) {
                                    bestSpot = testPos;
                                    foundSpot = true;
                                    break;
                                }
                            }

                            if (!foundSpot && glm::distance(m_selectedVillager->position, clickedNode->position) < 45.0f) {
                                bestSpot = m_selectedVillager->position;
                            }

                            finalTargetPos = bestSpot;
                            actionFound = true;
                        }
                        else {
                        std::cout << "Rozkaz: Ruch\n";
                        m_selectedVillager->targetNode = nullptr; 
                        m_selectedVillager->currentState = Villager::State::MOVING_TO_POINT;
                        finalTargetPos = worldMousePos;
                    }

                    m_selectedVillager->targetPosition = finalTargetPos;

                    m_selectedVillager->currentPath = Pathfinder::findPath(
                        m_selectedVillager->position,   // Start (vec2)
                        finalTargetPos,                 // Koniec (vec2)
                        *m_GameState->m_worldMap        // Mapa (WorldMap)
                    );
                    
                    m_selectedVillager->currentPathIndex = 0; // Reset indeksu
                }
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
    //glClear(GL_COLOR_BUFFER_BIT); 
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
           
            m_Renderer->drawSquare(*m_Camera, node.position, {10.0f, 10.0f}, color);
        }

        // 2 renderowanie mieszkancow 
        for (const auto& villager : m_GameState->m_villagers) {
            glm::vec4 color = {1.0f, 0.0f, 0.0f, 1.0f}; // czerwony 

            // zmienianie koloru kiedy zaznaczymy go 
            if (m_selectedVillager == &villager){
                color = {1.0f, 1.0f, 0.0f, 1.0f}; // zolty (jak zaznaczony)
            }

            m_Renderer->drawSquare(*m_Camera, villager.position, {10.0f, 10.0f}, color);
        }

        // renderowanie budynkow 
        for (const auto& b : m_GameState->m_buildings) {
            glm::vec4 color = {0.5f, 0.3f, 0.0f, 1.0f}; // brazowy 

            if (b.buildingType == Building::KITCHEN){
                color = {0.8f, 0.8f, 0.8f, 1.0f}; // jasnoszary 
            }

            else if (b.buildingType == Building::WELL){
                color = {0.0f, 0.5f, 1.0f, 1.0f}; // ciemnoniebieski 
            }

            else if (b.buildingType == Building::Type::STOCKPILE){
                color = {0.9f, 0.9f, 0.8f, 1.0f}; 
            }

            else if (b.buildingType == Building::CAMPFIRE) 
            {
                color = {1.0f, 0.5f, 0.0f, 1.0f}; 
            }

            else if (b.buildingType == Building::WALL) 
            {
                color = {0.4f, 0.4f, 0.4f, 1.0f}; // Szary
            }
            else if (b.buildingType == Building::STONE_WELL) 
            {
                color = {0.0f, 0.8f, 0.9f, 1.0f}; // Turkusowy
            }

            m_Renderer->drawSquare(*m_Camera, b.position, {20.0f, 20.0f}, color); 
        }


        // renderowanie ducha budynku 
        if (m_currentBuildMode == BuildMode::KITCHEN){
            // rysowanie polprzezroczystego kwadrata 
            glm::vec4 color = {0.8f, 0.8f, 0.8f, 0.5f}; 
            m_Renderer->drawSquare(*m_Camera, m_ghostBuildingPos, {20.0f, 20.0f}, color);
        }

        // budowanie studni
        else if (m_currentBuildMode == BuildMode::WELL){
            // polprzezroczysty niebieski kwadrat 
            glm::vec4 color = {0.0f, 0.5f, 1.0f, 0.5f}; 
            m_Renderer->drawSquare(*m_Camera, m_ghostBuildingPos, {15.0f, 15.0f}, color); 
        }

        // budowanie magazynu 
        else if (m_currentBuildMode == BuildMode::STOCKPILE){
            glm::vec4 color = {0.9f, 0.9f, 0.8f, 0.5f}; 
            m_Renderer->drawSquare(*m_Camera, m_ghostBuildingPos, {25.0f, 25.0f}, color);
        }

        // budowanie ogniska 
        else if (m_currentBuildMode == BuildMode::CAMPFIRE) 
        {
            glm::vec4 color = {1.0f, 0.5f, 0.0f, 0.5f}; 
            m_Renderer->drawSquare(*m_Camera, m_ghostBuildingPos, {20.0f, 20.0f}, color);
        }


        // -- dodanie cyklu dobowego i oswietlenia -- 
        // obliczanie jasnosci otoczenia 
        float time = m_GameState->timeOfDay; 
        sf::Uint8 darknessAlpha = 0; 

        // Logika obliczania ciemności (bez zmian)
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

            m_lightMapTexture.clear(sf::Color(0, 0, 0, darknessAlpha));
            m_lightMapTexture.setView(m_lightMapTexture.getDefaultView());

            sf::BlendMode eraser(sf::BlendMode::Zero, sf::BlendMode::One, sf::BlendMode::Add, sf::BlendMode::Zero, sf::BlendMode::OneMinusSrcAlpha, sf::BlendMode::Add); 
            sf::CircleShape lightShape; 
            lightShape.setFillColor(sf::Color(255, 255, 255, 255)); 

            glm::vec2 camPos = m_Camera->getPosition();
            sf::Vector2f camSize = m_Camera->getSize();
            sf::Vector2f winSize = static_cast<sf::Vector2f>(m_Window.getSize()); 

            // Skala (Zoom)
            float scaleX = winSize.x / camSize.x;
            float scaleY = winSize.y / camSize.y;

            for (const auto& b : m_GameState->m_buildings)
            {
                float radius = 0.0f; 
                if (b.buildingType == Building::CAMPFIRE) radius = 150.0f; 
                else if (b.buildingType == Building::KITCHEN) radius = 100.0f; 
                else if (b.buildingType == Building::WELL) radius = 60.0f;

                if (radius > 0.0f) 
                {
                    float screenX = (b.position.x - camPos.x) * scaleX;

                    float screenY = winSize.y - ((b.position.y - camPos.y) * scaleY); 

                    float screenRadius = radius * scaleX; 

                    lightShape.setRadius(screenRadius); 
                    lightShape.setOrigin(screenRadius, screenRadius); 
                    lightShape.setPosition(screenX, screenY); 

                    m_lightMapTexture.draw(lightShape, eraser); 
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

        // rysowanie ui 
        // definiowanie okna 
        // -- dolny pasek sterowania --
        // Obliczamy wymiary paska
        float windowW = (float)m_Window.getSize().x;
        float windowH = (float)m_Window.getSize().y;
        float barHeight = 160.0f;

        ImGui::SetNextWindowPos(ImVec2(0, windowH - barHeight));
        ImGui::SetNextWindowSize(ImVec2(windowW, barHeight));

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::Begin("MainBar", nullptr, windowFlags);

        // --- kolumna 1 zasoby ---
        ImGui::BeginGroup();
        {
            ImGui::TextColored(ImVec4(1, 0.8f, 0, 1), "Dzien: %d", m_GameState->dayCounter);
            ImGui::Text("Godzina: %02d:00", (int)m_GameState->timeOfDay);

            if (m_GameState->currentSeason == GameState::Season::SUMMER) 
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "LATO (%.1f C)", m_GameState->globalTemperature);
            else 
                ImGui::TextColored(ImVec4(0.4f, 0.6f, 1.0f, 1.0f), "ZIMA (%.1f C)", m_GameState->globalTemperature);

            ImGui::Separator();

            ImGui::Text("Drewno: %d", m_GameState->globalWood); ImGui::SameLine();
            ImGui::Text("Kamien: %d", m_GameState->globalStone);
            ImGui::Text("Woda:   %d", m_GameState->globalWater); ImGui::SameLine();
            ImGui::Text("Jedzenie: %d", m_GameState->globalFood);

            ImGui::Text("Ludzie: %lu", m_GameState->m_villagers.size());

            ImGui::Spacing(); 
            ImGui::Text("Morale Spolecznosci:");

            // Kolor paska zależny od poziomu
            if (m_GameState->globalMorale > 50.0f)
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.6f, 0.2f, 1.0f, 1.0f)); // Fioletowy (OK)
            else
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.3f, 0.3f, 0.3f, 1.0f)); // Szary (Depresja)

            ImGui::ProgressBar(m_GameState->globalMorale / 100.0f, ImVec2(180, 20), "");
            ImGui::PopStyleColor();
            
            if (m_GameState->globalMorale <= 0.0f) {
                ImGui::TextColored(ImVec4(1,0,0,1), "BUNT! KONIEC GRY!");
            }
        }
        ImGui::EndGroup();

        ImGui::SameLine(0, 20); 

        // --- kolumna 2 budowanie ---
        ImGui::BeginGroup();
        {
            ImGui::Text("BUDOWANIE:");

            auto DrawBuildButton = [&](const char* label, int cost, int& resourcePool, BuildMode mode, const char* resName) 
            {
                bool isActive = (m_currentBuildMode == mode);
                bool canAfford = (resourcePool >= cost); 

                if (isActive) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.7f, 0.0f, 1.0f)); 
                else if (!canAfford) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.2f, 0.2f, 1.0f)); 
                else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.3f, 0.2f, 1.0f)); 

                if (ImGui::Button(label, ImVec2(140, 40))) 
                {
                    if (isActive) 
                    {
                        m_currentBuildMode = BuildMode::NONE; 
                    } else 
                    {
                        m_currentBuildMode = mode; 
                        m_selectedVillager = nullptr; 
                    }
                }
                ImGui::PopStyleColor();

                if (ImGui::IsItemHovered()) 
                {
                    ImGui::SetTooltip("Koszt: %d %s", cost, resName);
                }
            };

            DrawBuildButton("KUCHNIA", 50, m_GameState->globalWood, BuildMode::KITCHEN, "Drewna"); 
            ImGui::SameLine(); 
            DrawBuildButton("STUDNIA", 25,  m_GameState->globalWood, BuildMode::WELL, "Drewna");

            DrawBuildButton("MAGAZYN",  10,  m_GameState->globalWood, BuildMode::STOCKPILE, "Drewna"); 
            ImGui::SameLine(); 
            DrawBuildButton("OGNISKO", 30,  m_GameState->globalWood, BuildMode::CAMPFIRE, "Drewna");
        
            DrawBuildButton("MUR", 10, m_GameState->globalStone, BuildMode::WALL, "Kamienia");
            ImGui::SameLine();
            DrawBuildButton("K. STUDNIA", 50, m_GameState->globalStone, BuildMode::STONE_WELL, "Kamienia");
        }
        ImGui::EndGroup();

        ImGui::SameLine(0, 20);

        // --- kolumna 3 inspektor, wielkosc itd  ---
        float currentX = ImGui::GetCursorPosX();
        float systemButtonsWidth = 150.0f; 
        float padding = 20.0f;
        float availableWidth = windowW - currentX - systemButtonsWidth - padding;

        float barWidth = availableWidth; 
        if (barWidth > 200.0f) barWidth = 200.0f; 
        if (barWidth < 100.0f) barWidth = 100.0f;

        ImGui::BeginGroup();
        {
            if (m_selectedVillager != nullptr) 
            {
                ImGui::TextColored(ImVec4(1,1,0,1), "WYBRANY: %s", m_selectedVillager->name.c_str());

                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.8f, 0.0f, 0.0f, 1.0f));
                ImGui::ProgressBar(m_selectedVillager->health / 100.0f, ImVec2(barWidth, 15), "Zdrowie");
                ImGui::PopStyleColor();

                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.8f, 0.5f, 0.0f, 1.0f));
                ImGui::ProgressBar(m_selectedVillager->hunger / 100.0f, ImVec2(barWidth, 15), "Glod");
                ImGui::PopStyleColor();

                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 0.5f, 1.0f, 1.0f));
                ImGui::ProgressBar(m_selectedVillager->thirst / 100.0f, ImVec2(barWidth, 15), "Woda");
                ImGui::PopStyleColor();

                ImGui::Text("Stan: %d", (int)m_selectedVillager->currentState);
            }
            else 
            {
                ImGui::TextDisabled("Nie wybrano nikogo.");
                ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + availableWidth);
                ImGui::Text("Kliknij LPM na mieszkanca aby zobaczyc status.");
                ImGui::PopTextWrapPos();

                ImGui::Spacing();

                ImGui::Text("Kontrola Czasu:");
                if (ImGui::Button("||", ImVec2(40, 30))) m_timeScale = 0.0f; ImGui::SameLine();
                if (ImGui::Button(">", ImVec2(40, 30))) m_timeScale = 1.0f; ImGui::SameLine();
                if (ImGui::Button(">>", ImVec2(40, 30))) m_timeScale = 5.0f;
            }
        }
        ImGui::EndGroup();

        // --- kolumna 4 systemowa --- 
        float targetX = windowW - systemButtonsWidth - 10.0f; 

        if (ImGui::GetCursorPosX() < targetX) {
            ImGui::SameLine();
            ImGui::SetCursorPosX(targetX);
        } else {
            ImGui::SameLine(0, 20); 
        }

        ImGui::BeginGroup();
            if (ImGui::Button("ZAPISZ", ImVec2(130, 25))) m_GameState->saveGame("save.txt");
            if (ImGui::Button("WCZYTAJ", ImVec2(130, 25))) m_GameState->loadGame("save.txt");
        
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
        
            if (ImGui::Button("WYJDZ", ImVec2(130, 25))) m_Running = false;
        
            ImGui::PopStyleColor();
        ImGui::EndGroup();

        ImGui::End();

        // rysowanie okna eventu 
        if (m_GameState->getMode() == GameState::Mode::EVENT_PAUSED){
            // ustawianie okna na srodku 
            ImGui::SetNextWindowPos(ImVec2(400, 300), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(400, 250)); 

            // rozpoczynanie okna 
            ImGui::Begin("WYDARZENIE", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

            // tytul i opis 
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", m_GameState->currentEventTitle.c_str()); // Czerwony tytuł
            ImGui::Separator();
            ImGui::TextWrapped("%s", m_GameState->currentEventDescription.c_str());

            ImGui::Spacing();
            ImGui::Spacing();

            if (ImGui::Button("PRZYJMIJ (Tak)", ImVec2(120, 30))) {
                m_GameState->resolveRefugeeEvent(true);
            }

            ImGui::SameLine();

            if (ImGui::Button("ODRZUĆ (Nie)", ImVec2(120, 30))) {
                m_GameState->resolveRefugeeEvent(false);
            }

            ImGui::End(); 
        }
    }
    
    else if (m_AppState == AppState::GAME_OVER)
    {
        glClearColor(0.2f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT); 

       ImGui::SetNextWindowPos(ImVec2(m_Window.getSize().x * 0.5f, m_Window.getSize().y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(400, 300));
        
        ImGui::Begin("Game Over", nullptr, ImGuiWindowFlags_NoDecoration); 


        ImGui::SetWindowFontScale(2.0f); // Duży tekst
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "     KONIEC GRY");
        ImGui::SetWindowFontScale(1.0f); // Powrót do normy
        ImGui::Separator();
        ImGui::Spacing();


        if (m_GameState->m_villagers.empty()) {
            ImGui::TextWrapped("Twoja wioska opustoszała. Wszyscy mieszkańcy zginęli z głodu, pragnienia lub zimna. Pamięć o nich przeminie.");
        } else {
            ImGui::TextWrapped("Wybuchł bunt! Mieszkańcy, doprowadzeni do ostateczności twoimi decyzjami, obalili władzę i uciekli z wioski.");
        }

        ImGui::Spacing(); ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();


        if (ImGui::Button("SPROBUJ PONOWNIE", ImVec2(380, 50)))
        {

            m_GameState = std::make_unique<GameState>(); 
            
  
            m_Camera->setPosition(glm::vec2(2000.0f, 2000.0f)); 
            

            m_AppState = AppState::GAME; 
        }

        ImGui::Spacing();


        if (ImGui::Button("WYJDZ Z GRY", ImVec2(380, 40)))
        {
            m_Running = false; 
        }

        ImGui::End();
    }
    // ------------------------------
    ImGui::SFML::Render();

    m_Window.display(); 
}