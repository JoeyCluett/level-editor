#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <utility>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "FullInit.h"
#include "../SimpleModel.h" // need the ModelInfo structure type


auto loadXYGrid(int x_len, int y_len, int z_len) -> std::pair<ModelInfo, ModelInfo> {

    ModelInfo mi_verts;
    ModelInfo mi_colors;

    std::vector<float> v;
    std::vector<float> colors;

    // first is the x axis
    v.insert(v.end(), 
        {
            float(-x_len), 0.0f, 0.0f,
            float(x_len),  0.0f, 0.0f
        });

    // second is the y axis
    v.insert(v.end(),
        {
            0.0f, float(y_len), 0.0f,
            0.0f, float(-y_len), 0.0f 
        });

    // third is the z axis
    v.insert(v.end(),
        {
            0.0f, 0.0f, float(z_len),
            0.0f, 0.0f, float(-z_len) 
        });



    for(int x = 1; x < x_len; x++) {

        v.insert(v.end(),
            {
                float(x), 0.0f, float(z_len),
                float(x), 0.0f, float(-z_len)   
            });

        v.insert(v.end(),
            {
                float(-x), 0.0f, float(z_len),
                float(-x), 0.0f, float(-z_len)   
            });
    }

    for(int z = 1; z < z_len; z++) {

        v.insert(v.end(),
            {
                float(x_len),  0.0f, float(z),
                float(-x_len), 0.0f, float(z),
            });

        v.insert(v.end(),
            {
                float(x_len),  0.0f, float(-z),
                float(-x_len), 0.0f, float(-z),
            });
    }

    // red
    colors.insert(colors.end(), { 1.0f, 0.0f, 0.0f });
    colors.insert(colors.end(), { 1.0f, 0.0f, 0.0f });

    // green
    colors.insert(colors.end(), { 0.0f, 1.0f, 0.0f });
    colors.insert(colors.end(), { 0.0f, 1.0f, 0.0f });

    // blue
    colors.insert(colors.end(), { 0.0f, 0.0f, 1.0f });
    colors.insert(colors.end(), { 0.0f, 0.0f, 1.0f });

    // all of the lines are grey
    for(unsigned int i = 18; i < v.size(); i += 3)
        colors.insert(colors.end(), { 0.0f, 0.0f, 0.0f });

    glGenBuffers(1, &mi_verts.buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, mi_verts.buffer_id);
    glBufferData(GL_ARRAY_BUFFER, v.size() * 4, v.data(), GL_STATIC_DRAW);
    mi_verts.vertices = v.size() / 3;

    glGenBuffers(1, &mi_colors.buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, mi_colors.buffer_id);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * 4, colors.data(), GL_STATIC_DRAW);
    mi_colors.vertices = colors.size() / 3;

    return { mi_verts, mi_colors };
}

