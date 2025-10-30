#include <GL/glew.h> // GLEW musi być pierwszy!
#include <SFML/Window.hpp>
#include <iostream>

int main() {
    // 1. Poproś o kontekst OpenGL 3.3 (Core Profile)
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.majorVersion = 3; // Chcemy OpenGL 3.3
    settings.minorVersion = 3;
    settings.attributeFlags = sf::ContextSettings::Core;

    // 2. Stwórz okno za pomocą SFML
    sf::Window window(sf::VideoMode(800, 600), "Moja Inżynierka (Fedora!)", sf::Style::Default, settings);
    window.setVerticalSyncEnabled(true);

    // 3. Zainicjuj GLEW (MUSI być po stworzeniu okna!)
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Nie udało się zainicjować GLEW!" << std::endl;
        return -1;
    }

    // 4. Ustaw kolor czyszczenia ekranu (np. na niebieski)
    glClearColor(0.1f, 0.3f, 0.6f, 1.0f);

    // 5. Główna pętla gry
    bool running = true;
    while (running) {
        // 6. Obsługa zdarzeń (wejście)
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                running = false;
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                running = false;
            }
        }

        // 7. Logika (na razie pusto)
        // ...

        // 8. Rysowanie (Renderowanie)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- Tutaj w Kroku 3 pojawi się kod rysujący ---

        // 9. Zamiana buforów i wyświetlenie
        window.display();
    }

    return 0;
}