# Minimalna wymagana wersja CMake
cmake_minimum_required(VERSION 3.10)

# Nazwa projektu
project(Echo_Wojny VERSION 1.0)

# Standars cpp 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# znajdz sfml
find_package(SFML 2.5 COMPONENTS window system REQUIRED)

# Znajdź OpenGL
find_package(OpenGL REQUIRED)

# Znajdź GLEW
find_package(GLEW REQUIRED)

# --- Tworzenie pliku wykonywalnego ---

# Tworzy plik wykonywalny o nazwie "gra" z pliku main.cpp
add_executable(gra src/main.cpp)

# --- Linkowanie bibliotek ---

# Połącz swoją "grę" ze znalezionymi bibliotekami
target_link_libraries(gra 
    PRIVATE
    sfml-window
    sfml-system
    OpenGL::GL  # Nowoczesny sposób linkowania OpenGL
    GLEW::GLEW  # Nowoczesny sposób linkowania GLEW
)