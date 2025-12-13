cmake_minimum_required(VERSION 3.10)
project(EchoWojny VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- PAKIETY ---
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

# --- ŹRÓDŁA IMGUI ---
set(IMGUI_SOURCES
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_widgets.cpp
    vendor/imgui/imgui_demo.cpp
    vendor/imgui-sfml/imgui-SFML.cpp
)

# --- ŹRÓDŁA GRY ---
set(APP_SOURCES
    src/main.cpp
    src/Core/Application.cpp
    src/Rendering/Camera2D.cpp
    src/Rendering/Shader.cpp
    src/Rendering/PrimitiveRenderer.cpp
    src/Game/GameState.cpp
    src/Game/WorldMap.cpp
    src/Game/Pathfinder.cpp
    ${IMGUI_SOURCES}
)

add_executable(gra ${APP_SOURCES})

# --- LINKOWANIE (WERSJA PANCERNA) ---
# Zamiast zmiennych, podajemy nazwy bibliotek, które linker (ld) znajdzie w systemie.
target_link_libraries(gra 
    PRIVATE
    sfml-system
    sfml-window
    sfml-graphics
    
    OpenGL::GL
    GLEW::GLEW
)

# --- INCLUDE ---
target_include_directories(gra 
    PRIVATE 
    /usr/include # Standardowa ścieżka nagłówków na Fedorze
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/vendor 
    ${PROJECT_SOURCE_DIR}/vendor/imgui 
    ${PROJECT_SOURCE_DIR}/vendor/imgui-sfml
)

# Definicja dla ImGui-SFML
target_compile_definitions(gra PRIVATE IMGUI_SFML_USE_DEFAULT_CONFIG)

file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
