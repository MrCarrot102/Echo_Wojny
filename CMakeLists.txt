cmake_minimum_required(VERSION 3.10)
project(EchoWojny VERSION 1.0)

# standard 
set(SFML_DIR "/usr/local/lib/cmake/SFML")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# zaleznosci 
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(SFML 2.6 COMPONENTS system window graphics audio REQUIRED)

# headery 
set(APP_HEADERS
    src/Core/Application.h
    src/Core/ResourceManager.h
    src/Rendering/Camera2D.h
    src/Rendering/Shader.h
    src/Rendering/PrimitiveRenderer.h
    src/Game/GameState.h
    src/Game/WorldMap.h
    src/Game/Pathfinder.h
    src/Game/Building.h
    src/Game/Enemy.h
    src/Game/ResourceNode.h
    src/Game/Villager.h
)

set(IMGUI_SOURCES
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_widgets.cpp
    vendor/imgui/imgui_demo.cpp
    vendor/imgui-sfml/imgui-SFML.cpp
)

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
    ${APP_HEADERS}
)

add_executable(gra ${APP_SOURCES})

target_link_libraries(gra 
    PRIVATE
    sfml-system
    sfml-window
    sfml-graphics
    sfml-audio
    OpenGL::GL
    GLEW::GLEW
    glm::glm 
)

target_include_directories(gra 
    PRIVATE 
    /usr/include
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/vendor 
    ${PROJECT_SOURCE_DIR}/vendor/imgui 
    ${PROJECT_SOURCE_DIR}/vendor/imgui-sfml
    ${PROJECT_SOURCE_DIR}/vendor/stb
)

target_compile_definitions(gra PRIVATE IMGUI_SFML_USE_DEFAULT_CONFIG)
find_package(SFML 2.6 COMPONENTS system window graphics audio REQUIRED)

file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
