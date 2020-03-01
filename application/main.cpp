#include <iostream>

// include all the things

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "lib/Initialization.h"
#include "lib/Texture.h"
#include "lib/Shader.h"
#include "lib/SimpleModel.h"
#include "lib/FloatCam.h"
#include "lib/range.h"

// load custom tool file importers
#include "lib/tool/LevelImport.h"
#include "lib/tool/FullInit.h"

#include "main.h"

#define WIDTH 800
#define HEIGHT 600

using namespace std;

vector<glm::mat4> transform_list;

void evaluateTransforms(GameWorld& gw) {
    transform_list.clear();
    transform_list.resize(gw.rigid_body_list.size());

    size_t sz = gw.rigid_body_list.size();
    for(unsigned int i = 0; i < sz; i++) {
        btTransform tf;
        gw.rigid_body_list[i]->getMotionState()->getWorldTransform(tf);
        tf.getOpenGLMatrix(glm::value_ptr(transform_list[i]));
    }
}

void addCube(GameWorld& gw, glm::vec3 pos, float _mass = 4.0) {
    btCollisionShape* box_shape = new btBoxShape(btVector3(0.15, 0.15, 0.15));
    //btCollisionShape* box_shape = new btSphereShape(0.5);
    gw.collision_shapes.push_back(box_shape);

    btTransform box_tf;
    box_tf.setIdentity();
    box_tf.setOrigin(btVector3(pos.x, pos.y, pos.z));

    btScalar m(_mass);
    btVector3 local_inertia(0.0, 0.0, 0.0);
    box_shape->calculateLocalInertia(m, local_inertia);
    btDefaultMotionState* motion_state = new btDefaultMotionState(box_tf);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(m, motion_state, box_shape, local_inertia);
    btRigidBody* body = new btRigidBody(rbInfo);
    body->setFriction(0.9);
    gw.dynamicsWorld->addRigidBody(body);
    gw.rigid_body_list.push_back(body);
}

int main(int argc, char* argv[]) {

    auto gw = initGameWorld("test game engine", true);    
    
    btCollisionShape* user_shape;
    btRigidBody*      user_body;

    { // load the ground for the physics engine

        // provide half-extents
        btCollisionShape* ground_shape = new btBoxShape(btVector3(12.5, 2.5, 12.5));
        gw.collision_shapes.push_back(ground_shape);

        // specify where the ground is in world coordinates
        btTransform ground_tf;
        ground_tf.setIdentity();
        ground_tf.setOrigin(btVector3( 12.5, -3.0, 12.5 ));

        btScalar  m(0.0);
        btVector3 local_inertia(0.0, 0.0, 0.0);
        btDefaultMotionState* motion_state = new btDefaultMotionState(ground_tf);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(m, motion_state, ground_shape, local_inertia);
        btRigidBody* body = new btRigidBody(rbInfo);
        body->setFriction(1.0);

        gw.dynamicsWorld->addRigidBody(body);
    }

    { // create user object to follow

        //btCollisionShape* box_shape = new btBoxShape(btVector3(0.15, 0.15, 0.15));
        user_shape = new btSphereShape(0.25);
        //gw.collision_shapes.push_back(box_shape);

        btTransform box_tf;
        box_tf.setIdentity();
        box_tf.setOrigin(btVector3(1.0f, 0.1f, 1.0f));

        btScalar m(2.0f);
        btVector3 local_inertia(0.0, 0.0, 0.0);
        user_shape->calculateLocalInertia(m, local_inertia);
        btDefaultMotionState* motion_state = new btDefaultMotionState(box_tf);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(m, motion_state, user_shape, local_inertia);
        btRigidBody* body = new btRigidBody(rbInfo);
        body->setFriction(0.0f);
        gw.dynamicsWorld->addRigidBody(body);
        //gw.rigid_body_list.push_back(body);

        //body->getCenterOfMassPosition();
        user_body = body;
        user_body->setFriction(0.0);
        user_body->forceActivationState(DISABLE_DEACTIVATION);
    }

    for(int y : range(0, 25))
        for(int x : range(0, 25)) {
            ::addCube(gw, { x, 5.0f, y });
        }

    // i do this the dumb, slow, easy way
    GLuint vertex_array_id;
    glGenVertexArrays(1, &vertex_array_id);
    glBindVertexArray(vertex_array_id);

    // a bunch of data that needs to be read from various asset files
    Texture*    brick_texture;
    Texture*    dirt_texture;
    Shader*     environment_shader;
    Shader*     texture_shader;
    ModelInfo   levelWalls;
    ModelInfo   levelWallsUV;
    ModelInfo   box_info;
    ModelInfo   ground_texels;
    ModelInfo   ground_uv_texels;
    ModelInfo   ball_model;

    // set global location data for asset loaders
    Shader::setVertexShaderDirectory("./assets/shaders/");
    Shader::setFragmentShaderDirectory("./assets/shaders/");
    SimpleModelParser::setFileLocation("./assets/models/");

    // ============================================================
    // load assets
    // ============================================================
    brick_texture = 
        new Texture(
            "./assets/textures/sanbrick.txt", 
            TEXTURE_CUSTOM_PAINT_TOOL, 
            GL_TEXTURE0 + 0, 
            GL_REPEAT, 
            GL_LINEAR);

    dirt_texture = 
        new Texture(
            "./assets/textures/dirt.txt",
            TEXTURE_CUSTOM_PAINT_TOOL,
            GL_TEXTURE0 + 0,
            GL_REPEAT,
            GL_LINEAR);

    environment_shader = new Shader( "environment" ); // shaders/environment.*.glsl
    texture_shader     = new Shader( "texture" );     // shaders/texture.*.glsl

    {
        auto levelWallsPair = ImportLevelFile("../map.txt", gw, true); // turns out, we dont actually need to pass gw :(
        levelWalls   = levelWallsPair.first;
        levelWallsUV = levelWallsPair.second;
    }

    SimpleModelParser::loadModelList({
        { "box.txt", "box.box", &box_info },
        { "ball.txt", "toplevel.ball", &ball_model }
    });

    // load the ground info into OpenGL
    {
        vector<GLfloat> texture_triangles = {
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 25.0f,
            25.0f, 0.0f, 0.0f,

            25.0f, 0.0f, 0.0f,
            25.0f, 0.0f, 25.0f,
            0.0f, 0.0f, 25.0f };

        for(int i = 0; i < (int)texture_triangles.size(); i += 3) {
            texture_triangles[i+0] -= 0.5f;
            texture_triangles[i+1] -= 0.5f;
            texture_triangles[i+2] -= 0.5f;
        }
        vector<GLfloat> texture_coords = {
            0.0f,      0.0f,
            0.0f,      25.0f,
            25.0f, 0.0f, //1.0f, 0.0f,

            25.0f, 0.0f,
            25.0f, 25.0f,
            0.0f,  25.0f
        };

        ground_texels = SimpleModelParser::loadForeignModelIntoRuntime(texture_triangles);
        ground_uv_texels = SimpleModelParser::loadForeignModelIntoRuntime(texture_coords);
    }

    // ============================================================
    // done loading assets
    // ============================================================

    FloatCam camera({ 0.5, 1.0, 0.5 }, 6.0, 800, 600, 0.07, gw.window);

    glm::mat4 Model = glm::mat4(1.0f);

    //glm::mat4 View =
    //    glm::lookAt(
    //        glm::vec3( 0.5, 1.0, 2.5 ),    // camera position
    //        glm::vec3( 0.0, 0.0, 0.0   ),  // looking at
    //        glm::vec3( 0.0, 1.0, 0.0   )); // 'up'

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
        accumulated_time += delta_time;

        if(glfwGetKey(gw.window, GLFW_KEY_W) == GLFW_PRESS) {
            auto cam_dir = camera.getDirection();
            auto current_vel = user_body->getLinearVelocity();
            user_body->setLinearVelocity(
                btVector3( cam_dir.x * 3.0f, current_vel.y(), cam_dir.z*3.0f ));
        }
        else {
            auto current_vel = user_body->getLinearVelocity();
            user_body->setLinearVelocity(btVector3(0.0f, current_vel.y(), 0.0f));
        }

        // perform the physics simulation
        {
            const double timestep = 1.0/120.0;
            while(accumulated_time >= timestep) {
                user_body->setAngularVelocity(btVector3( 0.0f, 0.0f, 0.0f ));
                user_body->forceActivationState(DISABLE_DEACTIVATION);
                gw.dynamicsWorld->stepSimulation(timestep);
                accumulated_time -= timestep;
            }
        }

        auto up = user_body->getCenterOfMassPosition();
        camera.setPosition({ up.x(), up.y(), up.z() });

        // render the physics blocks here
        if(false) {
            evaluateTransforms(gw);
            glUseProgram(environment_shader->getShaderId());
            glBindBuffer(GL_ARRAY_BUFFER, box_info.buffer_id);
            const auto pv = Projection * camera.getTf();
            for(auto& m4f : transform_list) {
                auto mvp = pv * m4f;
                auto mvp_uniform_location = environment_shader->getUniformLocation("MVP");
                glUniformMatrix4fv(mvp_uniform_location, 1, GL_FALSE, reinterpret_cast<float*>(&mvp));
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
                glDrawArrays(GL_TRIANGLES, 0, box_info.vertices);
            }
        }

        // render ball that moves
        if(false) {

            glm::mat4 gltf;
            btTransform tf;
            user_body->getMotionState()->getWorldTransform(tf);
            tf.getOpenGLMatrix(glm::value_ptr(gltf));

            auto new_mvp = Projection * camera.getTf() * gltf;
            glUseProgram(environment_shader->getShaderId());
            glBindBuffer(GL_ARRAY_BUFFER, ball_model.buffer_id);
            auto uniform_location = environment_shader->getUniformLocation("MVP");
            glUniformMatrix4fv(
                uniform_location,
                1,
                GL_FALSE,
                reinterpret_cast<float*>(&new_mvp));
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
            glDrawArrays(GL_TRIANGLES, 0, ball_model.vertices);
        }

        if(true) {
            auto new_mvp = Projection * camera.getTf() * Model;
            glUseProgram(texture_shader->getShaderId());

            auto uniform_location = texture_shader->getUniformLocation("MVP");
            glUniformMatrix4fv(
                uniform_location,
                1,
                GL_FALSE,
                reinterpret_cast<float*>(&new_mvp));

            brick_texture->use(*texture_shader, "texture_sampler");

            // vertex position
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, levelWalls.buffer_id);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

            // uv coordinates
            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, levelWallsUV.buffer_id);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

            glDrawArrays(GL_TRIANGLES, 0, levelWalls.vertices);
        }

        // render the textured ground
        if(true) {
            auto new_mvp = Projection * camera.getTf() * Model;
            glUseProgram(texture_shader->getShaderId());

            auto uniform_location = texture_shader->getUniformLocation("MVP");
            glUniformMatrix4fv(
                uniform_location,
                1,
                GL_FALSE,
                reinterpret_cast<float*>(&new_mvp));

            //brick_texture->use(*texture_shader, "texture_sampler");
            dirt_texture->use(*texture_shader, "texture_sampler");

            // vertex position
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, ground_texels.buffer_id);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

            // uv coordinates
            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, ground_uv_texels.buffer_id);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

            glDrawArrays(GL_TRIANGLES, 0, ground_texels.vertices);
        }

        iter_time = current_time;
        glfwSwapBuffers(gw.window);
        glfwPollEvents();
        glfwPollEvents();
    }

    glfwDestroyWindow(gw.window);
    glfwTerminate();
    return 0;
}
