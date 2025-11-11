#include "Core/Application.h" 
#include <iostream> 

Application::Application()
    : m_Window(), m_Running(true)
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

        // dodac jakies inne eventy zmiana rozdzielczosci itd; 
    }
}


void Application::update(float deltaTime){
    // aktualizacja logiki gry 
    m_Camera->update(deltaTime, m_Window); 

    // tick w symulacji 
    m_GameState->update(deltaTime); 
}

void Application::render(){
    glClear(GL_COLOR_BUFFER_BIT); 

    // renderowanie gry 

    // 1 renderowanie zasobow 
    for (const auto& node : m_GameState->getResourceNodes()) { 
        glm::vec4 color; 
        if (node.resourceType == ResourceNode::TREE){
            color = {0.0f, 0.5f, 0.0f, 1.0f}; // ciemnozielony na drzewo 
        } else if (node.resourceType == ResourceNode::ROCK) {
            color = {0.5f, 0.5f, 0.5f, 1.0f}; // szary na kamienie 
        }
        m_Renderer->drawSquare(*m_Camera, node.position, {10.0f, 10.0f}, color);
    }

    // 2 renderowanie mieszkancow 
    for (const auto& villager : m_GameState->getVillagers()) {
        glm::vec4 color = {1.0f, 0.0f, 0.0f, 1.0f}; // czerwony 
        m_Renderer->drawSquare(*m_Camera, villager.position, {10.0f, 10.0f}, color);
    }








    //m_Renderer->drawSquare(*m_Camera, {100.0f, 50.0f}, {30.0f, 30.0f}, {1.0f, 0.0f, 0.0f, 1.0f});
    //m_Renderer->drawSquare(*m_Camera, {200.0f, 150.0f}, {50.0f, 50.0f}, {0.0f, 1.0f, 0.0f, 1.0f});

    // koniec renderowania 

    m_Window.display(); 
}