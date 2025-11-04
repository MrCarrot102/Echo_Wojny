#version 330 core 

// wyjscie ostateczny kolo piksela 
out vec4 FragColor; 

// wejscie zmienna uniform wysylana z cpp 
uniform vec4 u_color; 

void main() {
    FragColor = u_color; 
}