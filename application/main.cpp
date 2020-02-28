#include <iostream>

// include all the things

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "lib/Initialization.h"
#include "lib/Texture.h"
#include "lib/Shader.h"
#include "lib/SimpleModel.h"
#include "lib/FloatCam.h"

// load custom level editor files
#include "lib/tool/LevelImport.h"
#include "lib/tool/FullInit.h"

//#include "main.h"

#define WIDTH 800
#define HEIGHT 600

using namespace std;

int main(int argc, char* argv[]) {

    auto gw = initGameWorld("test game engine", false);    

    btAlignedObjectArray<btCollisionShape*> engine_collision_shapes;

    // load the ground for the physics engine
    {
        // provide half-extents
        btCollisionShape* ground_shape = new btBoxShape(btVector3(12.5, 2.5, 12.5));
        engine_collision_shapes.push_back(ground_shape);

        // specify where the ground is in world coordinates
        btTransform ground_tf;
        ground_tf.setIdentity();
        ground_tf.setOrigin(btVector3( 12.5, -2.5, 12.5 ));

        btScalar  m(0.0);
        btVector3 local_inertia(0.0, 0.0, 0.0);
        btDefaultMotionState* motion_state = new btDefaultMotionState(ground_tf);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(m, motion_state, ground_shape, local_inertia);
        btRigidBody* body = new btRigidBody(rbInfo);
        body->setFriction(1.0);

        gw.dynamicsWorld->addRigidBody(body);
    }

    // load texture
    auto wall_texture = 
        new Texture(
            "../paint-tool/brick.txt", 
            TEXTURE_CUSTOM_PAINT_TOOL, 
            GL_TEXTURE0, 
            GL_REPEAT, 
            GL_LINEAR);

    // i do this the dumb, slow, easy way
    GLuint vertex_array_id;
    glGenVertexArrays(1, &vertex_array_id);
    glBindVertexArray(vertex_array_id);





    FloatCam camera({ 25.0, 20.0, 2.0 }, 6.0, 800, 600, 0.07, gw.window);

    glm::mat4 Model = glm::mat4(1.0f);

    glm::mat4 View =
        glm::lookAt(
            glm::vec3( 0.5, 1.0, 2.5 ),    // camera position
            glm::vec3( 0.0, 0.0, 0.0   ),  // looking at
            glm::vec3( 0.0, 1.0, 0.0   )); // 'up'

    glm::mat4 Projection =
        glm::perspective(
            glm::radians(66.0f), // FOV 66.0 degrees
            float(WIDTH)/float(HEIGHT),  // aspect ratio
            0.1f,                // near clipping plane
            100.0f);             // far clipping plane

    auto iter_time = glfwGetTime();
    double accumulated_time = 0.0;

    // ========================================================================
    // main game loop
    // ========================================================================
    while(!glfwWindowShouldClose(gw.window) && glfwGetKey(gw.window, GLFW_KEY_ESCAPE) != GLFW_PRESS) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto current_time = glfwGetTime();
        auto delta_time = current_time - iter_time;
        camera.update(float(delta_time));
        auto new_mvp = Projection * camera.getTf() * Model;
        accumulated_time += delta_time;

        // perform the physics simulation
        {
            const double timestep = 1.0/120.0;
            while(accumulated_time >= timestep) {
                gw.dynamicsWorld->stepSimulation(timestep);
                accumulated_time -= timestep;
            }
        }

        // render all the things!



        iter_time = current_time;
        glfwSwapBuffers(gw.window);
        glfwPollEvents();
    }

    glfwDestroyWindow(gw.window);
    glfwTerminate();
    return 0;
}
