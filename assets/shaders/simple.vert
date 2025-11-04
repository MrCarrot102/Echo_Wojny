#version 330 core
layout (location = 0) in vec2 aPos;

uniform mat4 u_model;
uniform mat4 u_projectionView;

void main() {
    gl_Position = u_projectionView * u_model * vec4(aPos.x, aPos.y, 0.0, 1.0);
}