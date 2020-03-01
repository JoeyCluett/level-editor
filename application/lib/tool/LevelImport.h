#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <utility>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "FullInit.h"
#include "../SimpleModel.h" // need the ModelInfo structure type

auto ImportLevelFile(std::string filename, GameWorld& gw, bool gen_uv = false) -> std::pair<ModelInfo, ModelInfo>;

// ==================================================================
// implementation
// ==================================================================
auto ImportLevelFile(std::string filename, GameWorld& gw, bool gen_uv) -> std::pair<ModelInfo, ModelInfo> {
    std::ifstream is(filename);
    std::string token;

    if(!(is >> token)) {
        std::cout << "ImportLevelFile : invalid file format\n" << std::flush;
        exit(1);
    }

    std::vector<GLfloat> verts;
    std::vector<GLfloat> uv;
    //std::array<int, 625> raw;

    // need to get past the actual tile data
    for(int i = 0; i < 25*25; i++) {
        int t;
        //is >> raw[i];
        is >> t;
    }

    //                                                    z-len    x-len
    auto gl_gen_box = [&gen_uv](float xstart, float zstart, float l, float w) -> std::pair<std::vector<float>, std::vector<float>> {
        std::vector<float> verts;
        std::vector<float> uv;

        const std::vector<std::array<float, 3>> tips = {
            { 0.5f, -0.5f, -0.5f },
            { 0.5f, -0.5f,  0.5f },
            { -0.5f, -0.5f,  0.5f },
            { -0.5f, -0.5f, -0.5f },
            { 0.5f,  0.5f, -0.5f },
            { 0.5f,  0.5f,  0.5f },
            { -0.5f,  0.5f,  0.5f },
            { -0.5f,  0.5f, -0.5f }
        };

        const std::vector<std::array<int, 3>> lut = {
            { 0, 1, 5 },
            { 0, 5, 4 },
            { 3, 0, 4 },
            { 3, 4, 7 },
            { 2, 3, 7 },
            { 2, 7, 6 },
            { 1, 2, 6 },
            { 1, 6, 5 }
        };

        xstart += (w / 2.0f);
        zstart += (l / 2.0f);

        for(auto& a : lut) {
            for(int idx : a) {
                auto tp = tips[idx];

                verts.push_back(xstart + ( tp[0] * w ));
                verts.push_back(tp[1]);
                verts.push_back(zstart + ( tp[2] * l ));

            }
        }

        if(gen_uv) {

            for(unsigned int i = 0; i < verts.size(); i += 36) {
                uv.insert(uv.end(), { 
                    0.0f, 0.0f, 
                    l,    0.0f, 
                    l,    1.0f, 
                    
                    0.0f, 0.0f,
                    l,    1.0f,
                    0.0f, 1.0f,
                    


                    0.0f, 0.0f, 
                    w,    0.0f, 
                    w,    1.0f, 
                    
                    0.0f, 0.0f,
                    w,    1.0f,
                    0.0f, 1.0f  });
            }

            const float scale = 2.0f;
            for(auto& f : uv)
                f *= scale;
        }

        return { verts, uv };
    };

    // actual collision entity data
    int collids;
    is >> collids;

    for(int i = 0; i < collids; i++) {
        float x, y, h, w;        
        is >> x >> y >> h >> w;

        x -= 0.5f;
        y -= 0.5f;

        auto newbox = gl_gen_box(x, y, h, w);
        verts.insert(verts.end(), newbox.first.begin(), newbox.first.end());

        if(gen_uv)
            uv.insert(uv.end(), newbox.second.begin(), newbox.second.end());

        // need a midpoint to make collision entity from
        x += ( w / 2.0f );
        y += ( h / 2.0f );

        // provide half-extents
        btCollisionShape* ground_shape = new btBoxShape(btVector3(w/2.0, 0.5, h/2.0));
        gw.collision_shapes.push_back(ground_shape);

        // specify where the ground is in world coordinates
        btTransform ground_tf;
        ground_tf.setIdentity();
        ground_tf.setOrigin(btVector3( x, 0.0, y ));

        btScalar  m(0.0);
        btVector3 local_inertia(0.0, 0.0, 0.0);
        btDefaultMotionState* motion_state = new btDefaultMotionState(ground_tf);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(m, motion_state, ground_shape, local_inertia);
        btRigidBody* body = new btRigidBody(rbInfo);
        body->setFriction(1.0);

        gw.dynamicsWorld->addRigidBody(body);

    }

    is.close();

    // place all of these vertices in the world
    ModelInfo mi_verts, mi_uv;

    // give this information to OpenGL
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * 4, verts.data(), GL_STATIC_DRAW);

    mi_verts.buffer_id = vbo;
    mi_verts.vertices  = verts.size() / 3;

    if(gen_uv) {
        GLuint vbo_uv;
        glGenBuffers(1, &vbo_uv);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_uv);
        glBufferData(GL_ARRAY_BUFFER, uv.size() * 4, uv.data(), GL_STATIC_DRAW);

        mi_uv.buffer_id = vbo_uv;
        mi_uv.vertices  = uv.size() / 3;
    }

    return { mi_verts, mi_uv };
}
