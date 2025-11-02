#include "Core/Application.h"
#include <iostream> 

int main(){
    try {
        Application app; 
        app.run(); 
    } catch (const std::exception& e) {
        std::cerr << "Wystąpił nieoczekiwany błąd: " << e.what() << std::endl; 
        return -1; 
    }

    return 0; 
}