#include "Core/Application.h" 
#include <iostream> 

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

    m_Window.create(sf::VideoMode(windowWidth, widnowHeight), "Echo Wojny", sf::Style::Default, settings); 
    m_Window.setVerticalSyncEnabled(true); 

    // --- INICJALIZACJA IMGUI --- 
    // Rzutujemy rozmiar na Vector2f
    if (!ImGui::SFML::Init(m_Window, static_cast<sf::Vector2f>(m_Window.getSize()))){
        std::cerr << "Nie udało się zainicjować ImGui!\n";
    }


    // inicjiowanie glew 
    if (glewInit() != GLEW_OK){
        std::cerr << "Nie udało się zainicjować GLEW!" << std::endl;
        m_Running = false; 
        return; 
    }

    // tworzenie obieku rdzenia 
    m_Camera = std::make_unique<Camera2D>((float)windowWidth, (float)widnowHeight); 
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
                    
                    if (m_currentBuildMode == BuildMode::KITCHEN){
                        int woodCost = 50;
                        if (m_GameState->globalWood >= woodCost){
                            m_GameState->globalWood -= woodCost;
                            m_GameState->m_buildings.emplace_back(Building::KITCHEN, m_ghostBuildingPos); 
                            std::cout << "[BUDOWA] Zbudowano Kuchnię! Pozostało drewna: " << m_GameState->globalWood << std::endl; 
                            m_currentBuildMode = BuildMode::NONE; 
                        } else {
                            std::cout << "[BUDOWA] Potrzeba więcej drewna mój Panie, a dokładnie " << woodCost << std::endl; 
                        }
                    }
                    
                    else if (m_currentBuildMode == BuildMode::WELL){
                        int woodCost = 25; 
                        if (m_GameState->globalWood >= woodCost){
                            m_GameState->globalWood -= woodCost; 
                            m_GameState->m_buildings.emplace_back(Building::WELL, m_ghostBuildingPos); 
                            std::cout << "[BUDOWA] Zbudowano Studnię! Pozostało drewna: " << m_GameState->globalWood << std::endl; 
                            m_currentBuildMode = BuildMode::NONE; 
                        } else {
                            std::cout << "[BUDOWA] Za mało drewna! Potrzeba " << woodCost << std::endl; 
                        }
                    }

                    else if (m_currentBuildMode == BuildMode::STOCKPILE){
                        int woodCost = 10; 
                        if (m_GameState->globalWood >= woodCost){
                            m_GameState->globalWood -= woodCost; 
                            m_GameState->m_buildings.emplace_back(Building::STOCKPILE, m_ghostBuildingPos); 
                            std::cout << "[BUDOWA] Wyznaczono Magazyn! Pozostało drewna: " << m_GameState->globalWood << std::endl; 
                            m_currentBuildMode = BuildMode::NONE; 
                        } else {
                            std::cout << "[BUDOWA] Za mało drewna na magazyn! Potrzeba " << woodCost << std::endl; 
                        }
                    }

                } else { // BŁĄD LOGICZNY BYŁ TUTAJ: Ten 'else if' musi być W ŚRODKU
                    // === JESTEŚMY W TRYBIE ZAZNACZANIA (nie budowania) ===
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
            // BŁĄD LOGICZNY BYŁ TUTAJ: 'else if (Right)' musi być tutaj,
            // a nie po 'else' od 'if (Left)'
            else if (event.mouseButton.button == sf::Mouse::Right){
                // === LOGIKA PRAWGO PRZYCISKU (ROZKAZY) ===
                if (m_selectedVillager != nullptr){
                    ResourceNode* clickedNode = nullptr; 
                    for (ResourceNode& node : m_GameState->m_resourceNodes){
                        if (worldMousePos.x >= node.position.x && worldMousePos.x <= node.position.x + 15.0f &&
                            worldMousePos.y >= node.position.y && worldMousePos.y <= node.position.y + 15.0f)
                        {
                            clickedNode = &node; 
                            break; 
                        }
                    }
                    
                    if (clickedNode != nullptr){
                        std::cout << "Rozkaz pracy dla " << m_selectedVillager->name << " przy zasobie" << std::endl; 
                        m_selectedVillager->targetNode = clickedNode; 
                        m_selectedVillager->targetPosition = clickedNode->position; 
                        m_selectedVillager->currentState = Villager::State::MOVING_TO_WORK; 
                    } else {
                        std::cout << "Rozkaz ruchu dla " << m_selectedVillager->name << std::endl;
                        m_selectedVillager->targetNode = nullptr; 
                        m_selectedVillager->targetPosition = worldMousePos; 
                        m_selectedVillager->currentState = Villager::State::MOVING_TO_POINT;
                    }
                }
            }
        }   
    }
}


void Application::update(float deltaTime){
    // --- imgui update ---
    ImGui::SFML::Update(sf::Mouse::getPosition(m_Window), static_cast<sf::Vector2f>(m_Window.getSize()), sf::seconds(deltaTime));
    // -------------------- 

    // aktualizacja logiki gry 
    m_Camera->update(deltaTime, m_Window); 

    // tick w symulacji 
    m_GameState->update(deltaTime); 

    if (m_currentBuildMode != BuildMode::NONE){
        // aktualizowanie pozycji ducha na podstawie myszki 
        sf::Vector2i screenMousePos = sf::Mouse::getPosition(m_Window); 
        m_ghostBuildingPos = m_Camera->screenToWorld({(float)screenMousePos.x, (float)screenMousePos.y}, m_Window);

    }
}

void Application::render(){
    glClear(GL_COLOR_BUFFER_BIT); 

    // renderowanie gry 

    // 1 renderowanie zasobow 
    for (const auto& node : m_GameState->m_resourceNodes) { 
        glm::vec4 color; 
        if (node.resourceType == ResourceNode::TREE){
            color = {0.0f, 0.5f, 0.0f, 1.0f}; // ciemnozielony na drzewo 
        } else if (node.resourceType == ResourceNode::ROCK) {
            color = {0.5f, 0.5f, 0.5f, 1.0f}; // szary na kamienie 
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

    // rysowanie ui 
    // definiowanie okna 
    ImGui::Begin("Panel Zarzadzania");
    
    ImGui::Text("Dzien: %d", m_GameState->dayCounter); 
    ImGui::Text("Godzina: %d:00", (int)m_GameState->timeOfDay); 
    ImGui::Separator();
    ImGui::Text("Drewno: %d", m_GameState->globalWood);
    ImGui::Text("Woda: %d", m_GameState->globalWater);
    ImGui::Text("Jedzenie: %d", m_GameState->globalFood);
    ImGui::Separator();
    ImGui::Text("Mieszkancy: %lu", m_GameState->m_villagers.size());
    
    ImGui::Separator();
    ImGui::Text("BUDOWANIE:");

    // logika przyciskow 
    bool isKitchenActive = (m_currentBuildMode == BuildMode::KITCHEN); 
    if (isKitchenActive) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.5f, 0.0f, 1.0f)); 

    // przycisk odpowiedzialny za kuchnie 
    if(ImGui::Button("Kuchnia (50 Drewna)")){
        if (isKitchenActive){
            m_currentBuildMode = BuildMode::NONE; 
        } else {
            m_currentBuildMode = BuildMode::KITCHEN; 
            m_selectedVillager = nullptr; 
        }
    }

    if (isKitchenActive) ImGui::PopStyleColor(); 

    // przycisk odpowiedzialny za studnie 
    bool isWellActivate = (m_currentBuildMode == BuildMode::WELL); 
    if (isWellActivate) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.5f, 0.0f, 1.0f)); 

    if (ImGui::Button("Studnia (25 Drewna)")){
        if (isWellActivate){
            m_currentBuildMode = BuildMode::NONE; 
        } else {
            m_currentBuildMode = BuildMode::WELL; 
            m_selectedVillager = nullptr; 
        }
    }

    if (isWellActivate) ImGui::PopStyleColor(); 

    // przycisk odpowiedzialny za magazyn 
    bool isStockpileActive = (m_currentBuildMode == BuildMode::STOCKPILE);
    if (isStockpileActive) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.5f, 0.0f, 1.0f));

    if (ImGui::Button("Magazyn (10 Drewna)")){
        if (isStockpileActive){
            m_currentBuildMode = BuildMode::NONE; 
        } else {
            m_currentBuildMode = BuildMode::STOCKPILE; 
            m_selectedVillager = nullptr; 
        }
    }
    if (isStockpileActive) ImGui::PopStyleColor(); 

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

        // --- PRZYCISKI DECYZJI ---
        // Button zwraca 'true' jeśli został kliknięty w tej klatce
        
        if (ImGui::Button("PRZYJMIJ (Tak)", ImVec2(120, 30))) {
            m_GameState->resolveRefugeeEvent(true); // Logika z GameState
        }
        
        ImGui::SameLine(); // Rysuj następny element w tej samej linii
        
        if (ImGui::Button("ODRZUĆ (Nie)", ImVec2(120, 30))) {
            m_GameState->resolveRefugeeEvent(false);
        }

        ImGui::End(); 
    }


    
    // ------------------------------
    ImGui::SFML::Render();

    m_Window.display(); 
}