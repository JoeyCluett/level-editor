#version 330 core

layout(location = 0) in vec3 vertexPosition; // location must match glVertexAttribPointer() call
layout(location = 1) in vec3 vertexColor;

out vec3 fragmentColor;

//uniform mat4 Model; // model - view - projection matrix
//uniform mat4 View;
//uniform mat4 Projection;

// environment is just one big object with many smaller parts
uniform mat4 MVP; // model-view-projection matrix

void main() {

    gl_Position = MVP * vec4(vertexPosition, 1.0);

    // calculate the color of this vertex
    fragmentColor = vertexColor;
}
