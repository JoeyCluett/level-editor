#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <SDL/SDL.h>
#include "event_core.h"

using namespace std;

#define TILEWIDTH 24

struct Tile_t {
    int type;
    int tracked;

    static const int DEFAULT = 0;
    static const int BARRIER = 1;
};

typedef vector<vector<Tile_t>> TileArray_t;

// we want to have as few of these as possible for a given map
struct CollisionGeometry {    
    // basically just a SDL_Rect but with larger data type
    int x;
    int y;
    int w;
    int h;
};

void initTileArray(TileArray_t& ta);
void render(SDL_Surface* scr, TileArray_t& ta, bool render_collision_data);
void saveFile(std::string filename, TileArray_t& ta);
void readFile(std::string filename, TileArray_t& ta);

vector<CollisionGeometry> optimize_collision_entities(TileArray_t& ta);

int main(int argc, char* argv[]) {

    if(argc < 3) {
        cout << "Options:\n\n";
        cout << 
            " -n <new map file>\n"
            " -i <existing map file>\n"
            " -o <where to save map file>\n\n";

        return 1;
    }

    // each tile is 24x24 pixels
    TileArray_t tile_array;
    initTileArray(tile_array);

    string infile, outfile;

    string flag = argv[1];
    if(flag == "-n") {
        outfile = argv[2];
    }
    else if(flag == "-i") {
        infile = argv[2];
        ::readFile(infile, tile_array);
    }
    else if(flag == "-o") {
        outfile = argv[2];
    }

    if(argc >= 5) {
        flag = argv[3];
        if(flag == "-i") {
            infile = argv[4];
            ::readFile(infile, tile_array);
        }
        else if(flag == "-o") {
            outfile = argv[4];
        }
    }

    SDL_Init(SDL_INIT_EVERYTHING);
    auto* scr = SDL_SetVideoMode(800, 600, 32, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN);

/*
    for(int y : {0, 24})
        for(int x = 0; x < 25; x++)
            tile_array[y][x].type = Tile_t::BARRIER;
    for(int y = 0; y < 25; y++)
        for(int x : {0, 24})
            tile_array[y][x].type = Tile_t::BARRIER;
*/

    bool loop_running = true;
    bool render_collision_data = false;

    sdl_event_map_t eventmap = {
        {
            SDL_KEYDOWN,
            [
                    &loop_running,&outfile,
                    &tile_array,&render_collision_data](void* ptr) {

                auto* key_event = (SDL_KeyboardEvent*)ptr;
                auto sym = key_event->keysym.sym;

                if(sym == SDLK_ESCAPE)
                    loop_running = false;
                else if(sym == SDLK_s)
                    ::saveFile(outfile, tile_array);
                else if(sym == SDLK_q)
                    render_collision_data = !render_collision_data;

            }
        },
        {
            SDL_MOUSEBUTTONDOWN,
            [&tile_array](void* ptr) {
                auto* mouse_button_event = (SDL_MouseButtonEvent*)ptr;
                auto x = mouse_button_event->x;
                auto y = mouse_button_event->y;

                x /= TILEWIDTH;
                y /= TILEWIDTH;

                if(x > 24)
                    return;

                Tile_t& tref = tile_array[y][x];
                if(tref.type == Tile_t::DEFAULT)
                    tref.type = Tile_t::BARRIER;
                else if(tref.type == Tile_t::BARRIER)
                    tref.type = Tile_t::DEFAULT;
            }
        }
    };

    while(loop_running) {
        sdl_evaluate_events(eventmap);
        render(scr, tile_array, render_collision_data);
        SDL_Flip(scr);
        SDL_Delay(16);
    }

    SDL_Quit();
    return 0;
}

vector<CollisionGeometry> optimize_collision_entities(TileArray_t& ta) {

    //cout << "optimizing collision geometry...\n" << flush;

    vector<CollisionGeometry> vcollide;

    auto track_h = [&ta](int y, int x) -> int {

        //cout << "tracking h-length(x=" << x << ",y=" << y << ")...\n" << flush;

        int len = 0;
        while(x < 25 && ta[y][x].type == Tile_t::BARRIER) {
            len++;
            x++;
        }

        return len;
    };

    auto track_v = [&ta](int y, int x) -> int {

        //cout << "tracking v-length(x=" << x << ",y=" << y << ")...\n" << flush;

        int len = 0;
        while(y < 25 && ta[y][x].type == Tile_t::BARRIER) {
            len++;
            y++;
        }

        return len;
    };

    for(int y = 0; y < 25; y++) {
        for(int x = 0; x < 25; x++) {

            if(ta[y][x].type == Tile_t::BARRIER && ta[y][x].tracked == 0) {

                CollisionGeometry cg;
                cg.x = x;
                cg.y = y;

                int hlen = track_h(y, x);
                int vlen = track_v(y, x);

                if(hlen >= vlen) {
                    // equal length favors hlen
                    for(int i = 0; i < hlen; i++)
                        ta[y][x+i].tracked++;
                
                    cg.h = 1;
                    cg.w = hlen;

                }
                else {

                    for(int i = 0; i < vlen; i++)
                        ta[y+i][x].tracked++;

                    cg.h = vlen;
                    cg.w = 1;

                }

                vcollide.push_back(cg);
            }
        }
    }

    // reset all tracking data
    for(int y = 0; y < 25; y++)
        for(int x = 0; x < 25; x++)
            ta[y][x].tracked = 0;

    return vcollide;
}

void saveFile(std::string filename, TileArray_t& ta) {
    ofstream os(filename);
    
    os << "MAPDATA\n";

    for(int y = 0; y < 25; y++) {
        for(int x = 0; x < 25; x++) {
            os << ta[y][x].type << ' ';
        }
        os << '\n';
    }

    auto vcollide = optimize_collision_entities(ta);

    os << vcollide.size() << '\n';
    for(auto cg : vcollide) {
        os << cg.x << ' ' << cg.y << ' ' << cg.h << ' ' << cg.w << '\n';
    }

    os.close();
}

void readFile(std::string filename, TileArray_t& ta) {
    ifstream is(filename);
    string token;

    if(!(is >> token)) {
        cout << "map file is empty...\n";
        exit(1);
    }

    for(int y = 0; y < 25; y++) {
        for(int x = 0; x < 25; x++) {

            is >> token;

            if(token == "0") {
                ta[y][x].type = Tile_t::DEFAULT;
            }
            else if(token == "1") {
                ta[y][x].type = Tile_t::BARRIER;
            }
            else {
                cout << "map contains invalid data: " << token << endl;
                exit(1);
            }

        }
    }

}

void initTileArray(TileArray_t& ta) {
    for(int y = 0; y < 25; y++) {
        ta.push_back({}); // push back empty vector<Tile_t>

        for(int x = 0; x < 25; x++) {
            ta.back().push_back({}); // push back empty Tile_t
            ta.back().back().type = Tile_t::DEFAULT;
            ta.back().back().tracked = 0;
        }
    }
}

void render(SDL_Surface* scr, TileArray_t& ta, bool render_collision_data) {

    // clear the screen
    SDL_FillRect(scr, NULL, 0x00);

    for(int y = 0; y < 25; y++) {
        for(int x = 0; x < 25; x++) {

            Tile_t t = ta[y][x];
            //t.type = Tile_t::BARRIER;

            SDL_Rect r;
            r.x = TILEWIDTH * x + 2;
            r.y = TILEWIDTH * y + 2;
            r.h = TILEWIDTH - 4;
            r.w = TILEWIDTH - 4;

            if(t.type == Tile_t::DEFAULT) {

                SDL_FillRect(scr, &r, SDL_MapRGB(scr->format, 80, 80, 80));

            }
            else if(t.type == Tile_t::BARRIER) {

                SDL_FillRect(scr, &r, SDL_MapRGB(scr->format, 220, 0, 0));
                r.y += 6;
                r.h = 1;
                SDL_FillRect(scr, &r, SDL_MapRGB(scr->format, 255, 255, 255));
                r.y += 8;
                SDL_FillRect(scr, &r, SDL_MapRGB(scr->format, 255, 255, 255));
                r.y -= 8;
                r.h = 8;
                r.w = 1;
                r.x += 4;
                SDL_FillRect(scr, &r, SDL_MapRGB(scr->format, 255, 255, 255));
                r.h = 8;
                r.w = 1;
                r.x += 12;
                SDL_FillRect(scr, &r, SDL_MapRGB(scr->format, 255, 255, 255));
                r.x -= 6;
                r.y -= 6;
                r.h -= 1;
                SDL_FillRect(scr, &r, SDL_MapRGB(scr->format, 255, 255, 255));
                r.y += 14;
                SDL_FillRect(scr, &r, SDL_MapRGB(scr->format, 255, 255, 255));

            }

        }
    }

    if(render_collision_data) {
        auto vcollide = optimize_collision_entities(ta);
        for(auto cg : vcollide) {

            if(cg.h == 1) {

                // horizontal entity
                SDL_Rect r;
                r.x = TILEWIDTH*cg.x;
                r.y = TILEWIDTH*cg.y + 6;
                r.h = 12;
                r.w = cg.w * TILEWIDTH;
            
                SDL_FillRect(scr, &r, SDL_MapRGB(scr->format, 255, 255, 0));

            }
            else {

                // vertical entity
                SDL_Rect r;
                r.x = TILEWIDTH*cg.x + 6;
                r.y = TILEWIDTH*cg.y;
                r.h = cg.h * TILEWIDTH;
                r.w = 12;

                SDL_FillRect(scr, &r, SDL_MapRGB(scr->format, 255, 255, 0));

            }

        }
    }

}
