#include "Core/Application.h" 
#include <iostream> 

Application::Application()
    : m_Window(), m_Running(true), m_selectedVillager(nullptr)
{
    init(); 
}

Application::~Application() {
    // i teraz magia bo smart pointery same sie sprzataja ez 
}

void Application::init() {
    // konfigurowanie okna 
    sf::ContextSettings settings; 
    settings.depthBits = 24; 
    settings.stencilBits = 8; 
    settings.majorVersion = 3; 
    settings.minorVersion = 3; 
    settings.attributeFlags = sf::ContextSettings::Core; 

    unsigned int windowWidth = 800; 
    unsigned int widnowHeight = 600; 

    m_Window.create(sf::VideoMode(windowWidth, widnowHeight), "Echo Wojny", sf::Style::Default, settings); 
    m_Window.setVerticalSyncEnabled(true); 

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
        if (event.type == sf::Event::Closed){
            m_Running = false; 
        }
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

        if (event.type == sf::Event::MouseButtonPressed) {
            // pobbieranie funkcji myszy 
            glm::vec2 screenMousePos = {(float)event.mouseButton.x, (float)event.mouseButton.y};
            // konwersja na pozycje w swiecie 
            glm::vec2 worldMousePos = m_Camera->screenToWorld(screenMousePos, m_Window); 

            if (event.mouseButton.button == sf::Mouse::Left){

                if (m_currentBuildMode != BuildMode::NONE){
                    // tryb budowania 
                    int woodCost = 50; // kuchnia potrzebuje 50 drewna do zbudowania 

                    // sprawdzanie odpowiedniej ilosci 
                    if (m_GameState->globalWood >= woodCost){
                        // jesli nas stac 
                        m_GameState->globalWood -= woodCost; 

                        // tworzenie budynku 
                        m_GameState->m_buildings.emplace_back(Building::KITCHEN, m_ghostBuildingPos); 
                        std::cout << "[BUDOWA] Zbudowano Kuchnię! Pozostało drewna: " << m_GameState->globalWood << std::endl; 

                        // wylaczanie trybu budowania 
                        m_currentBuildMode = BuildMode::NONE; 
                    } else {
                        // kiedy nie mamy odpowiedniej ilosci drewna moj panie 
                        std::cout << "[BUDOWA] Potrzeba więcej drewna mój Panie, a dokładnie " << woodCost << std::endl; 
                    }
                } else { 
                    // zaznaczanie 
                    std::cout << "Kliknięto w świat: " << worldMousePos.x << ", " << worldMousePos.y << std::endl; 

                    // szukanie mieszkanca w tym miejscu 
                    // nie jest optymalna ta petla na razie 
                    m_selectedVillager = nullptr; 
                    for (Villager& v : m_GameState->m_villagers) {
                        // test kolizjii 
                        if (worldMousePos.x >= v.position.x && worldMousePos.x <= v.position.x + 10.0f &&
                            worldMousePos.y >= v.position.y && worldMousePos.y <= v.position.y + 10.0f)
                            {
                                m_selectedVillager = &v; // zaznacz 
                                std::cout << "Zaznaczono: " << v.name << std::endl; 
                                break; // konczenie petli 
                            }
                        }
                }
            }
            else if (event.mouseButton.button == sf::Mouse::Right){
                // wydanie rozkazu prawy guzior 
                if (m_selectedVillager != nullptr){
                    // sprawdzanie klikniecia na zasob 
                    ResourceNode* clickedNode = nullptr; 
                    for (ResourceNode& node : m_GameState->m_resourceNodes){
                        // testowanie kolizji 
                        if (worldMousePos.x >= node.position.x && worldMousePos.x <= node.position.x + 15.0f &&
                            worldMousePos.y >= node.position.y && worldMousePos.y <= node.position.y + 15.0f)
                        {
                            clickedNode = &node; 
                            break; 
                        }
                    }
                    
                    if (clickedNode != nullptr){
                        // rozkazywanie pracy 
                        std::cout << "Rozkaz pracy dla " << m_selectedVillager->name << " przy zasobie" << std::endl; 
                        m_selectedVillager->targetNode = clickedNode; 
                        m_selectedVillager->targetPosition = clickedNode->position; // ustawianie celu 
                        m_selectedVillager->currentState = Villager::State::MOVING_TO_WORK; 
                    } else {
                        // dawanie rozkazu tylko do ruchu 
                        std::cout << "Rozkaz ruchu dla " << m_selectedVillager->name << std::endl;
                        m_selectedVillager->targetNode = nullptr; // zaznaczenie ze idzie w puste miejsce 
                        m_selectedVillager->targetPosition = worldMousePos; 
                        m_selectedVillager->currentState = Villager::State::MOVING_TO_POINT;
                    }
                }
            }
        }

        // dodac jakies inne eventy zmiana rozdzielczosci itd; 
    }
}


void Application::update(float deltaTime){
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
        m_Renderer->drawSquare(*m_Camera, b.position, {20.0f, 20.0f}, color); 
    }


    // renderowanie ducha budynku 
    if (m_currentBuildMode == BuildMode::KITCHEN){
        // rysowanie polprzezroczystego kwadrata 
        glm::vec4 color = {0.8f, 0.8f, 0.8f, 0.5f}; 
        m_Renderer->drawSquare(*m_Camera, m_ghostBuildingPos, {20.0f, 20.0f}, color);
    }


    
    //m_Renderer->drawSquare(*m_Camera, {100.0f, 50.0f}, {30.0f, 30.0f}, {1.0f, 0.0f, 0.0f, 1.0f});
    //m_Renderer->drawSquare(*m_Camera, {200.0f, 150.0f}, {50.0f, 50.0f}, {0.0f, 1.0f, 0.0f, 1.0f});

    // koniec renderowania 

    m_Window.display(); 
}