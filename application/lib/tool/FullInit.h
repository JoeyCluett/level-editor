#pragma once

#include "../Initialization.h"

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <bullet/btBulletDynamicsCommon.h>

// initialization stuff needed for game engine
struct GameWorld {
    GLFWwindow*      window;
    btDynamicsWorld* dynamicsWorld;  
    std::vector<btCollisionShape*> collision_shapes;
    std::vector<btRigidBody*> rigid_body_list;
};

auto initGameWorld(std::string game_name = "", bool fullscreen = false) -> GameWorld;
auto init_bt(void) -> btDiscreteDynamicsWorld*;

// ============================================================================
// implementation
// ============================================================================
auto initGameWorld(std::string game_name, bool fullscreen) -> GameWorld {
    GameWorld gw;

    // initialize glfw
    gw.window = GLFWINITWINDOW(800, 600, game_name.c_str(), {3, 3}, 4, fullscreen);
    glfwSetInputMode(gw.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(gw.window, GLFW_STICKY_KEYS, GL_TRUE);

    // initialize Bullet
    gw.dynamicsWorld = init_bt();

    // some housekeeping stuff for OpenGL
    //glClearColor(0.0f, 0.0f, 0.35f, 0.0f);
    glClearColor(0.25f, 0.25f, 0.25f, 0.0f);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);

    return gw;
}

auto init_bt(void) -> btDiscreteDynamicsWorld* {

    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
	btCollisionDispatcher* dispatcher                       = new btCollisionDispatcher(collisionConfiguration);
	btBroadphaseInterface* overlappingPairCache             = new btDbvtBroadphase();
	btSequentialImpulseConstraintSolver* solver             = new btSequentialImpulseConstraintSolver;
	btDiscreteDynamicsWorld* dynamicsWorld                  = new btDiscreteDynamicsWorld(
                                                                   dispatcher, overlappingPairCache, solver, collisionConfiguration);
	//dynamicsWorld->setGravity(btVector3(0, -10, 0));
	//dynamicsWorld->setGravity(btVector3(0, -10, 0));
	dynamicsWorld->setGravity(btVector3(0, 0.0f, 0));


    return dynamicsWorld;
}
