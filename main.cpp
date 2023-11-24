#include <SDL.h>
#include <SDL_image.h>

#include <iostream>
#include <cmath>

#include <windows.h>
#include <winuser.h>

#include <random>

#include "Timer.h"
#include <rotate.h>
#include <math.h>
#include <execution>

#include <Habitat.h>

#include <opencv2/core/core.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/mat.hpp>
#include "opencv2/opencv.hpp"
#include <opencv2/imgcodecs.hpp>

#include "utils.h"

bool init_sdl(SDL_Window** window_ptr, SDL_Surface** surface_ptr, SDL_Renderer** renderer_ptr,
                   int x_win_res, int y_win_res);
void sdl_draw(SDL_Renderer* renderer, const SrcImage& img, void* data);

int main( int argc, char* args[] ) {
    SDL_Window* window = NULL;
    SDL_Surface* screenSurface = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Event event;
    init_sdl(&window, &screenSurface, &renderer, 400, 400);
    IMG_Init(IMG_INIT_PNG);

    std::vector<SrcImage> src_images;
    load_images("hemtai", src_images);
    int recInd = src_images.size()-4;
    SrcImage reconstructed = src_images[recInd];
    std::cout << "RECONSTRUCTING " << reconstructed.path << std::endl;
    src_images.erase(src_images.begin() + recInd);
    Habitat hbsim = Habitat(&reconstructed, &src_images);

    Timer t;
    t.start();
    for (int i = 0; i < 100000000; i++) {
        hbsim.step();
        while (SDL_PollEvent(&event)) {
        }
        if (i % 500 == 0) {
            sdl_draw(renderer, reconstructed, (void*)hbsim.getBestGroup().pastedData);
            std::cout << "currently at " << i << " 500 gens took " << t.get() <<
             "\n" << "sad is " << hbsim.getBestGroup().fitness <<
              " num images :" << hbsim.getBestGroup().individuals.size() << "\n";

            t.start();
        }
    }

    return 0;
}


bool init_sdl(SDL_Window** window_ptr, SDL_Surface** surface_ptr, SDL_Renderer** renderer_ptr,
            int x_win_res, int y_win_res) {
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        return false;
    }

    *window_ptr = SDL_CreateWindow( "imageVimage", SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED, x_win_res,
                                y_win_res, SDL_WINDOW_SHOWN );
    if( *window_ptr == NULL ) {
        printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
        return false;
    }

    *renderer_ptr = SDL_CreateRenderer( *window_ptr, -1, SDL_RENDERER_ACCELERATED );

    SDL_SetRenderDrawColor( *renderer_ptr, 0xFF, 0xFF, 0xFF, 0xFF );

    *surface_ptr = SDL_GetWindowSurface( *window_ptr );

    return true;
}

void sdl_draw(SDL_Renderer* renderer, const SrcImage& img, void* data) {
    SDL_Texture *intermediate = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STREAMING, img.width, img.height);
    SDL_UpdateTexture(intermediate, NULL, data, img.pitch);

    SDL_SetRenderDrawColor(renderer, 88, 88, 88, 255);
    SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, intermediate, NULL, NULL);
	SDL_RenderPresent(renderer);

    SDL_Delay(0);
    SDL_DestroyTexture(intermediate);
}
