cmake_minimum_required(VERSION 3.10)
project(EchoWojny VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- Znajdź Pakiety (Używamy "starego" stylu, który Ci działał) ---

# Ta komenda znajdzie SFML i stworzy zmienną ${SFML_INCLUDE_DIR}
find_package(SFML 2.5 REQUIRED COMPONENTS system window graphics) 
# (Możesz dodać 'audio' i 'network', jeśli chcesz, ale 'system' i 'window' są kluczowe)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
# GLM mamy w folderze 'vendor', więc nie musimy go szukać

# --- Pliki Źródłowe (.cpp) ---
set(APP_SOURCES
    src/main.cpp
    src/Core/Application.cpp
    src/Core/Camera2D.cpp
    src/Rendering/Shader.cpp
    src/Rendering/PrimitiveRenderer.cpp
)
add_executable(gra ${APP_SOURCES})

# --- Linkowanie i Dołączanie (NAJWAŻNIEJSZA SEKCJA) ---

# To jest sekcja, która naprawi Twoje błędy 'undefined reference'
# Używamy DOKŁADNIE tych nazw, które podałeś w swoim działającym pliku:
target_link_libraries(gra 
    PRIVATE
    # Te linie mówią linkerowi, aby znalazł biblioteki
    # libsfml-system.so i libsfml-window.so
    sfml-system  
    sfml-window
    sfml-graphics # Na razie niepotrzebne, ale nie szkodzi

    OpenGL::GL
    GLEW::GLEW
)

# Mówimy kompilatorowi, gdzie są pliki nagłówkowe (.h)
target_include_directories(gra 
    PRIVATE 
    # Ścieżka do nagłówków SFML (z find_package)
    ${SFML_INCLUDE_DIR} 
    
    # Nasze własne ścieżki
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/vendor # Dla GLM
)

# --- Kopiowanie Zasobów ---
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})