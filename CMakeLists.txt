cmake_minimum_required(VERSION 3.10)
project(EchoWojny VERSION 1.0) # Zmieniłem nazwę projektu

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- Znajdź Pakiety ---
find_package(SFML 2.5 REQUIRED system window)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

# --- Zdefiniuj nasz plik wykonywalny i jego pliki źródłowe ---
# Zbieramy wszystkie pliki .cpp do jednej listy
set(APP_SOURCES
    src/main.cpp
    src/Core/Application.cpp
    src/Core/Camera2D.cpp
    src/Rendering/Shader.cpp
    src/Rendering/PrimitiveRenderer.cpp
)

add_executable(gra ${APP_SOURCES}) # Tworzymy "gra" z tych plików

# --- Linkowanie i Dołączanie ---
target_link_libraries(gra 
    PRIVATE
    ${SFML_LIBRARIES}
    OpenGL::GL
    GLEW::GLEW
)

# Powiedz CMake, żeby szukał nagłówków (.h) w całym folderze src
# To pozwoli nam robić #include "Core/Application.h"
target_include_directories(gra 
    PRIVATE 
    ${SFML_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/vendor # Dla GLM
)

# --- Kopiowanie Zasobów ---
# To jest BARDZO WAŻNE: Kopiuje folder 'assets'
# do folderu 'build', aby program mógł znaleźć shadery.
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})