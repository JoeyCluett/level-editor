#version 330 core

out vec3 color;
flat in vec3 fragmentColor;

void main() {
    
    color = fragmentColor;

}