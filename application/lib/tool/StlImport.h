#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <utility>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "FullInit.h"
#include "../SimpleModel.h" // need the ModelInfo structure type

static std::vector<float>* readStlModel(std::string filename);
ModelInfo loadBinaryStlModel(std::string filename);
auto loadBinaryStlModelWithNormals(std::string filename) -> std::pair<ModelInfo, ModelInfo>;

// ==================================================================
// implementation
// ==================================================================

auto loadBinaryStlModelWithNormals(std::string filename) -> std::pair<ModelInfo, ModelInfo> {

    auto& v = *readStlModel(filename);
    std::vector<float> n;

    int sz = (int)v.size();
    for(int i = 0; i < sz; i += 9) {

        float
            x1 = v[i+0], y1 = v[i+1], z1 = v[i+2],
            x2 = v[i+3], y2 = v[i+4], z2 = v[i+5],
            x3 = v[i+6], y3 = v[i+7], z3 = v[i+8];

        glm::vec3 d1 = { x2-x1, y2-y1, z2-z1 };
        glm::vec3 d2 = { x3-x2, y3-y2, z3-z2 };

        glm::vec3 c = {
            d1.y*d2.z - d1.z*d2.y,
            d1.z*d2.x - d1.x*d2.z,
            d1.x*d2.y - d1.y*d2.x
        };

        for(int j = 0; j < 3; j++) {
            n.push_back(c.x);
            n.push_back(c.y);
            n.push_back(c.z);
        }

    }

    // place all of these vertices in the world
    ModelInfo mi_verts;
    ModelInfo mi_norms;

    // give this information to OpenGL
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, v.size() * 4, v.data(), GL_STATIC_DRAW);

    mi_verts.buffer_id = vbo;
    mi_verts.vertices  = v.size() / 3;

    GLuint vbo_n;
    glGenBuffers(1, &vbo_n);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_n);
    glBufferData(GL_ARRAY_BUFFER, n.size() * 4, n.data(), GL_STATIC_DRAW);

    mi_norms.buffer_id = vbo_n;
    mi_norms.vertices  = n.size() / 3;

    delete &v;

    return { mi_verts, mi_norms };
}

ModelInfo loadBinaryStlModel(std::string filename) {

    auto vptr = readStlModel(filename);

    // place all of these vertices in the world
    ModelInfo mi;

    // give this information to OpenGL
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vptr->size() * 4, vptr->data(), GL_STATIC_DRAW);

    mi.buffer_id = vbo;
    mi.vertices  = vptr->size() / 3;

    delete vptr;
    return mi;
}

static std::vector<float>* readStlModel(std::string filename) {

    auto vptr = new std::vector<float>;

    std::ifstream is(filename, std::ifstream::binary);

    char header[80];
    is.read(header, 80); // dont really care about the header but we need it

    union {
        unsigned int num;
        char buf[4];
    } facets;

    is.read(facets.buf, 4);

    union {
        float f32[12];
        char buf[sizeof(float)*12];
    } attrs;

    union {
        char buf[2];
        unsigned short u16;
    } attr_ct;

    for(unsigned int i = 0; i < facets.num; i++) {

        is.read(attrs.buf, sizeof(float)*12);
        vptr->insert(vptr->end(), attrs.f32 + 3, attrs.f32 + 12);

        is.read(attr_ct.buf, 2); // read attribute count
    }

    is.close();
    return vptr;
}

