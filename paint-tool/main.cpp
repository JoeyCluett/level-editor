#include <iostream>
#include <vector>
#include <array>
#include <fstream>
#include <SDL/SDL.h>
#include "../event_core.h"

#define MAX_Y (64*9)
#define MAX_X (64*9)
#define COLOR_BLOCK_SIZE 35
#define COLOR_SEL_HEIGHT (35*17)
#define PIXEL_SIZE 9 // 64x64 grid of pixels

using namespace std;

typedef array<array<int, 64>, 64> DrawSurface_t;

void readFile(string filename, DrawSurface_t& ds);
void saveFile(string filename, DrawSurface_t& ds);
void fillArea(DrawSurface_t& ds, int x, int y, int exist, int new_color);

int main(int argc, char* argv[]) {
    
    if(argc < 3) {
        cout << "Options:\n\n";
        cout << 
            " -n <new map file>\n"
            " -i <existing map file>\n"
            " -o <where to save map file>\n\n";

        return 1;
    }

    DrawSurface_t drawSurface;
    for(int y = 0; y < 64; y++)
        for(int x = 0; x < 64; x++)
            drawSurface[y][x] = 14; // black

    string infile, outfile;

    string flag = argv[1];
    {
        if(flag == "-n") {
            outfile = argv[2];
        }
        else if(flag == "-i") {
            infile = argv[2];
            ::readFile(infile, drawSurface);
        }
        else if(flag == "-o") {
            outfile = argv[2];
        }

        if(argc >= 5) {
            flag = argv[3];
            if(flag == "-i") {
                infile = argv[4];
                ::readFile(infile, drawSurface);
            }
            else if(flag == "-o") {
                outfile = argv[4];
            }
        }
    }

    SDL_Init(SDL_INIT_EVERYTHING);
    auto* scr = SDL_SetVideoMode(800, 600, 32, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN);
    
    auto screen_format = scr->format;

    array<uint32_t, 17> colors = {
        SDL_MapRGB(screen_format, 0x80, 0x00, 0x00),
        SDL_MapRGB(screen_format, 0xFF, 0x00, 0x00),
        SDL_MapRGB(screen_format, 0xFF, 0xA5, 0x00),
        SDL_MapRGB(screen_format, 0xFF, 0xFF, 0x00),
        SDL_MapRGB(screen_format, 0x80, 0x80, 0x00),
        SDL_MapRGB(screen_format, 0x80, 0x00, 0x80),
        SDL_MapRGB(screen_format, 0xFF, 0x00, 0xFF),
        SDL_MapRGB(screen_format, 0xFF, 0xFF, 0xFF),
        SDL_MapRGB(screen_format, 0x00, 0xFF, 0x00),
        SDL_MapRGB(screen_format, 0x00, 0x80, 0x00),
        SDL_MapRGB(screen_format, 0x00, 0x00, 0x80),
        SDL_MapRGB(screen_format, 0x00, 0x00, 0xFF),
        SDL_MapRGB(screen_format, 0x00, 0xFF, 0xFF),
        SDL_MapRGB(screen_format, 0x00, 0x80, 0x80),
        SDL_MapRGB(screen_format, 0x00, 0x00, 0x00),
        SDL_MapRGB(screen_format, 0xC0, 0xC0, 0xC0),
        SDL_MapRGB(screen_format, 0x80, 0x80, 0x08)
    };

    bool loop_running = true;
    int color_index = 0;
    bool is_drawing = false;
    bool using_large_select = false;
    int last_x = -1, last_y = -1;

    sdl_event_map_t eventmap = {
        {
            SDL_KEYDOWN,
            [&loop_running,&outfile,&drawSurface](void* ptr) {

                auto* key_event = (SDL_KeyboardEvent*)ptr;
                auto sym = key_event->keysym.sym;

                if(sym == SDLK_ESCAPE)
                    loop_running = false;
                else if(sym == SDLK_s)
                    ::saveFile(outfile, drawSurface);

            }
        },
        {
            SDL_MOUSEMOTION,
            [&drawSurface,&is_drawing,&color_index,&last_x,&last_y](void* ptr) {
                auto* mouse_motion_event = (SDL_MouseMotionEvent*)ptr;
                auto x = mouse_motion_event->x;
                auto y = mouse_motion_event->y;

                if(is_drawing && x < MAX_X && y < MAX_Y)
                    drawSurface[y/PIXEL_SIZE][x/PIXEL_SIZE] = color_index;
            }
        },
        {
            SDL_MOUSEBUTTONDOWN,
            [&is_drawing, &color_index, &drawSurface, &last_x, &last_y](void* ptr) {
                auto* mouse_button_event = (SDL_MouseButtonEvent*)ptr;
                auto x = mouse_button_event->x;
                auto y = mouse_button_event->y;
                auto but = mouse_button_event->button;

                if(x < MAX_X && y < MAX_Y) {
                    if(but == SDL_BUTTON_LEFT) {
                        is_drawing = true;
                        last_x = x;
                        last_y = y;
                        drawSurface[y/PIXEL_SIZE][x/PIXEL_SIZE] = color_index;
                    }
                    else if(but == SDL_BUTTON_RIGHT) {
                        //cout << "Callback: " << x << ", " << y << endl;

                        fillArea(drawSurface, 
                            x/PIXEL_SIZE, 
                            y/PIXEL_SIZE, 
                            drawSurface[y/PIXEL_SIZE][x/PIXEL_SIZE], 
                            color_index);
                    }
                }
                else if(x > 800-COLOR_BLOCK_SIZE && y < COLOR_SEL_HEIGHT) {
                    // selecting a different color
                    color_index = y / COLOR_BLOCK_SIZE;
                }

            }
        },
        {
            SDL_MOUSEBUTTONUP,
            [&is_drawing, &color_index](void* ptr) {
                //auto* mouse_button_event = (SDL_MouseButtonEvent*)ptr;
                //auto x = mouse_button_event->x;
                //auto y = mouse_button_event->y;

                is_drawing = false;

            }
        }
    };

    while(loop_running) {
        sdl_evaluate_events(eventmap);
        //render(scr, tile_array, render_collision_data);

        // place render stuff here
        SDL_FillRect(scr, NULL, colors[color_index]);

        // image should ALWAYS have a thing black edge
        SDL_Rect e;
        e.x = 0;
        e.y = 0;
        e.h = MAX_X + 10;
        e.w = MAX_Y + 10;

        SDL_FillRect(scr, &e, 0x00);

        // draw all of the color selector boxes
        for(int i = 0; i < 17; i++) {
            SDL_Rect r;
            r.x = 800-COLOR_BLOCK_SIZE;
            r.y = i * COLOR_BLOCK_SIZE;

            r.w = COLOR_BLOCK_SIZE;
            r.h = COLOR_BLOCK_SIZE;

            SDL_FillRect(scr, &r, colors[i]);
        }

        for(int y = 0; y < 64; y++) {
            for(int x = 0; x < 64; x++) {

                SDL_Rect r;
                r.x = PIXEL_SIZE * x;
                r.y = PIXEL_SIZE * y;

                r.w = PIXEL_SIZE;
                r.h = PIXEL_SIZE;

                SDL_FillRect(scr, &r, colors[drawSurface[y][x]]);

            }
        }

        SDL_Flip(scr);
        SDL_Delay(16);
    }

    SDL_Quit();
    return 0;
}

void fillArea(DrawSurface_t& ds, int x, int y, int exist, int new_color) {

    if(exist == new_color)
        return;

    //cout << x << ", " << y << endl;
    //cout << "fa\n";

    if(x < 0 || x > 63 || y < 0 || y > 63) {
        return;
    }

    else if(ds[y][x] == exist) {
        ds[y][x] = new_color;

        //cout << "  rec\n";

        fillArea(ds, x, y-1, exist, new_color); // up
        fillArea(ds, x, y+1, exist, new_color); // down
        fillArea(ds, x-1, y, exist, new_color); // left
        fillArea(ds, x+1, y, exist, new_color); // right

    }
}

void readFile(string filename, DrawSurface_t& ds) {

    ifstream is(filename);
    string s;
    
    if(!(is >> s)) {
        cout << "error reading input file...\n";
        exit(1);
    }

    const vector<array<int,3>> color_lut = {
        { 0x80, 0x00, 0x00 },
        { 0xFF, 0x00, 0x00 },
        { 0xFF, 0xA5, 0x00 },
        { 0xFF, 0xFF, 0x00 },

        { 0x80, 0x80, 0x00 },
        { 0x80, 0x00, 0x80 },
        { 0xFF, 0x00, 0xFF },
        { 0xFF, 0xFF, 0xFF },
        
        { 0x00, 0xFF, 0x00 },
        { 0x00, 0x80, 0x00 },
        { 0x00, 0x00, 0x80 },
        { 0x00, 0x00, 0xFF },
        
        { 0x00, 0xFF, 0xFF },
        { 0x00, 0x80, 0x80 },
        { 0x00, 0x00, 0x00 },
        { 0xC0, 0xC0, 0xC0 },
        
        { 0x80, 0x80, 0x08 },
    };

    for(int y = 0; y < 64; y++) {
        for(int x = 0; x < 64; x++) {
            // have to convert back to format used internally by paint-tool
            int r,g,b;
            is >> r >> g >> b;
            
            for(int i = 0; i < color_lut.size(); i++)
                if(color_lut[i] == array<int,3>{ r,g,b }) {
                    ds[y][x] = i;
                    break;
                }
        }
    }
    is.close();

}

void saveFile(string filename, DrawSurface_t& ds) {

    ofstream os(filename);

    os << "IMAGEDATA\n";

    const vector<array<int,3>> color_lut = {
        { 0x80, 0x00, 0x00 },
        { 0xFF, 0x00, 0x00 },
        { 0xFF, 0xA5, 0x00 },
        { 0xFF, 0xFF, 0x00 },

        { 0x80, 0x80, 0x00 },
        { 0x80, 0x00, 0x80 },
        { 0xFF, 0x00, 0xFF },
        { 0xFF, 0xFF, 0xFF },
        
        { 0x00, 0xFF, 0x00 },
        { 0x00, 0x80, 0x00 },
        { 0x00, 0x00, 0x80 },
        { 0x00, 0x00, 0xFF },
        
        { 0x00, 0xFF, 0xFF },
        { 0x00, 0x80, 0x80 },
        { 0x00, 0x00, 0x00 },
        { 0xC0, 0xC0, 0xC0 },
        
        { 0x80, 0x80, 0x08 },
    };

    for(int y = 0; y < 64; y++) {
        for(int x = 0; x < 64; x++) {
            int idx = ds[y][x];
            auto& arr = color_lut[idx];
            os << arr[0] << ' ' << arr[1] << ' ' << arr[2] << ' ';
        }
        os << endl;
    }

    os.close();
}
