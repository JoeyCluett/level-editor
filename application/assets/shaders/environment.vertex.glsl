#version 330 core

layout(location = 0) in vec3 vertexPosition; // location must match glVertexAttribPointer() call

out vec3 fragmentColor;

// environment is just one big object with many smaller parts
uniform mat4 MVP; // model-view-projection matrix

void main() {

    gl_Position = MVP * vec4(vertexPosition, 1.0);

    vec3 mycolor = vec3(1.0f, 1.0f, 1.0f);

    // calculate the color of this vertex
    float mod_result = mod( gl_VertexID, 3 );
    if(mod_result < 0.1) {                          fragmentColor = mycolor * 0.4; }
    else if(mod_result > 0.9 && mod_result < 1.1) { fragmentColor = mycolor * 0.3; }
    else {                                          fragmentColor = mycolor * 0.2; }

}