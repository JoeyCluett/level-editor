#version 330 core

layout(location = 0) in vec3 vertexPosition; // location must match glVertexAttribPointer() call

out vec3 fragmentColor;

//uniform mat4 Model; // model - view - projection matrix
//uniform mat4 View;
//uniform mat4 Projection;

// environment is just one big object with many smaller parts
uniform mat4 MVP; // model-view-projection matrix

void main() {

    //gl_Position = MVP * instance_tf * vec4(vertexPosition, 1.0);

    gl_Position = MVP * vec4(vertexPosition, 1.0);

    // calculate the color of this vertex
    float mod_result = mod( gl_VertexID, 3 );
    if(mod_result < 0.1) {                          fragmentColor = vec3( 0.0, 0.0, 0.0 ); }
    else if(mod_result > 0.9 && mod_result < 1.1) { fragmentColor = vec3( 0.5, 0.0, 0.0 ); }
    else {                                          fragmentColor = vec3( 0.25, 0.0, 0.0 ); }

    // shades of input color
    //float mod_result = mod( gl_VertexID, 3 );
    //if(mod_result < 0.1) {                          fragmentColor = input_color * 0.0; }
    //else if(mod_result > 0.9 && mod_result < 1.1) { fragmentColor = input_color * 0.5; }
    //else {                                          fragmentColor = input_color * 1.0; }

}