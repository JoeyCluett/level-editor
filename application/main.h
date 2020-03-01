#pragma once

#include <map>
#include <unordered_map>
#include <vector>
#include <set>
#include <iostream>
#include <string>
#include <fstream>

struct GraphNode {

    int flags;

    std::pair<int, int> up;
    std::pair<int, int> down;
    std::pair<int, int> left;
    std::pair<int, int> right;

    // init everything to -1 (an invalid index into the node_map)
    GraphNode(void) : up(-1, -1), down(-1,-1), left(-1, -1), right(-1, -1) {}

    void resetFlags(void) { 
        this->flags = 0x00; }

    bool visited(void) { 
        return flags & 0x01; }

    void visit() {
        this->flags |= 0x01; }

    bool hasNorth(void) { return this->up    != std::pair<int,int>{ -1, -1 }; }
    bool hasSouth(void) { return this->down  != std::pair<int,int>{ -1, -1 }; }
    bool hasEast(void)  { return this->right != std::pair<int,int>{ -1, -1 }; }
    bool hasWest(void)  { return this->left  != std::pair<int,int>{ -1, -1 }; }

};

struct Graph {
private:
    std::vector<std::pair<int,int>> ai_spawn_points;
    std::map<std::pair<int, int>, GraphNode> node_map;

public:

    void insertNewNode(int y, int x) {
        auto iter = this->node_map.find({ y, x });

        if(iter == this->node_map.end()) {
            this->node_map.insert({{ y, x }, GraphNode()});

            // update neighbors
            {
                // search left
                auto niter = this->node_map.find({ y, x-1 });
                if(niter != this->node_map.end()) {
                    this->node_map[std::pair<int,int>{ y, x-1 }].right = {y,x};
                    this->node_map[std::pair<int,int>{ y, x }].left = { y, x-1 };
                }

                // search right
                niter = this->node_map.find({ y, x+1 });
                if(niter != this->node_map.end()) {
                    this->node_map[std::pair<int,int>{ y, x+1 }].left = { y, x };
                    this->node_map[std::pair<int,int>{ y, x }].right = { y, x+1 };
                }

                // search up
                niter = this->node_map.find({ y-1, x });
                if(niter != this->node_map.end()) {
                    this->node_map[std::pair<int,int>{ y-1, x }].down = { y, x };
                    this->node_map[std::pair<int,int>{ y, x }].up = { y-1, x };
                }

                // search down
                niter = this->node_map.find({ y+1, x });
                if(niter != this->node_map.end()) {
                    this->node_map[std::pair<int,int>{ y+1, x }].up = { y, x };
                    this->node_map[std::pair<int,int>{ y, x }].down = { y+1, x };
                }

            }
        }

    }

    auto searchFor(std::pair<int,int> from, std::pair<int,int> to) 
            -> std::vector<std::pair<int,int>> {

        auto iter = this->node_map.find(from);
        if(iter == this->node_map.end())
            return {};

        iter = this->node_map.find(to);
        if(iter == this->node_map.end())
            return {};

        for(auto& gn : this->node_map)
            gn.second.resetFlags();

        // both endpoints exist
        std::vector<std::pair<int,int>> s;

        s.push_back(from);
        while(s.size() > 0) {
            auto p = s.back();
            s.pop_back();
            auto& gn = this->node_map[p];

            // does end at some point
            if(p == to)
                return s;

            if(!this->node_map[p].visited()) {
                this->node_map[p].visit();

                if(gn.hasNorth())
                    s.push_back(gn.up);

                if(gn.hasSouth())
                    s.push_back(gn.down);

                if(gn.hasWest())
                    s.push_back(gn.left);

                if(gn.hasEast())
                    s.push_back(gn.right);
            }
        }

    }

    void insertSpawnPoint(int y, int x) {
        this->ai_spawn_points.push_back({ y, x });
    }

};

Graph* gen_ai_graph(std::string filename) {

    Graph* gr = new Graph;

    std::ifstream is(filename);

    std::string token;
    if(!(is >> token)) {
        std::cout << "error reading level input file...\n";
        exit(1);
    }

    for(int y = 0; y < 25; y++) {
        for(int x = 0; x < 25; x++) {

            is >> token;

            if(token != "1") {
                // not a barrier type
                gr->insertNewNode(y, x);

                // insert spawn points separately
                if(token == "2")
                    // ai spawn point
                    gr->insertSpawnPoint(y, x);

            }
        }
    }

    return gr;
}
