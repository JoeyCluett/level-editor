#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "FullInit.h"
#include "../SimpleModel.h" // need the ModelInfo structure type

void ImportLevelFile(std::string filename, GameWorld& gw);

// ==================================================================
// implementation
// ==================================================================
void ImportLevelFile(std::string filename, GameWorld& gw) {
    std::ifstream is(filename);
    std::string token;

    if(!(is >> token)) {
        cout << "ImportLevelFile : invalid file format\n";
        exit(1);
    }

    std::vector<GLfloat> verts;
    //std::array<int, 625> raw;

    // need to get past the actual tile data
    for(int i = 0; i < 25*25; i++) {
        int t;
        //is >> raw[i];
        is >> t;
    }

    //                                      y-len    x-len
    auto gl_gen_box = [](float x, float y, float l, float w) -> std::vector<float> {
        std::vector<float> v;

        

        return v;
    };

    // actual collision entity data
    int collids;
    is >> collids;

    for(int i = 0; i < collids; i++) {
        float x, y, z, h, w;        
        is >> x >> y >> h >> w;

        x -= 0.5f;
        y -= 0.5f;
        z = -0.5f;



    }

    is.close();
}
