#version 330 core 

// wejscie pozycja wierzcholka vbo 
layout (location = 0) in vec2 aPos; 

// wejscie zmienne uniformy wyslane z cpp 
// macierz modelu pozycja/rozmiar kwadratu 
uniform mat4 u_model;

// macierz kamery 
uniform mat u_projectionView; 

void main(){
    // obliczanie ostatecznej pozycji wierzchołka na ekranie 
    gl_Position = u_projectionView * u_model * vec4(aPos.x, aPos.y, 0.0, 1.0); 
}
