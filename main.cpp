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

#include <opencv2/core/core.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/mat.hpp>
#include "opencv2/opencv.hpp"
#include <opencv2/imgcodecs.hpp>

#include "utils.h"

bool init_sdl(SDL_Window** window_ptr, SDL_Surface** surface_ptr, SDL_Renderer** renderer_ptr,
                   int x_win_res, int y_win_res);
void sdl_draw(SDL_Renderer* renderer, const image& img);

int main( int argc, char* args[] ) {
    SDL_Window* window = NULL;
    SDL_Surface* screenSurface = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Event event;
    init_sdl(&window, &screenSurface, &renderer, 400, 400);
    IMG_Init(IMG_INIT_PNG);

    std::vector<image> src_images;
    load_images("hemtai", src_images);
    image reconstructed = src_images[0];
    reconstructed.data = new uint8_t[reconstructed.width * reconstructed.height * 4];
    memset(reconstructed.data, 0xFFFFFF00, reconstructed.width * reconstructed.height);
    RotatePixel_t *pPstBase = static_cast<RotatePixel_t*>((void*)reconstructed.data);
    RotatePixel_t *pDstBase = static_cast<RotatePixel_t*>((void*)src_images[0].data);

    Timer t;
    t.start();
    for (int i = 0; i < 100000000; i++) {
        int index = rand()%(src_images.size() - 1) + 1;
        RotatePixel_t *pSrcBase = static_cast<RotatePixel_t*>((void*)src_images[index].data);
        int best_sad = INT_MAX;
        int bestX;
        int bestY;
        float bestA;
        float bestS;
        for (int j = 0; j < 100; j++) {
            int fDstCX = rand()%(src_images[0].width - 10) + 5;
            int fDstCY = rand()%(src_images[0].height - 10) + 5;
            float fAngle = rand()%(314) / 100.0;
            float fScale = (rand()%(30) + 3) / 10.0;
            sadPair genQuality = RotateDrawClipSad(
                    pDstBase, src_images[0].width, src_images[0].height, src_images[0].pitch,
                    pSrcBase, src_images[index].width, src_images[index].height, src_images[index].pitch,
                    pPstBase,
                    fDstCX, fDstCY,
                    0, 0,
                    fAngle, fScale );
            if (genQuality.sad1 < genQuality.sad2 && genQuality.sad1 < best_sad) {
                best_sad = genQuality.sad1;
                bestX = fDstCX;
                bestY = fDstCY;
                bestA = fAngle;
                bestS = fScale;
            }
        }

        if (best_sad != INT_MAX) {
            RotateDrawClip(
                    pPstBase, src_images[0].width, src_images[0].height, src_images[0].pitch,
                    pSrcBase, src_images[index].width, src_images[index].height, src_images[index].pitch,
                    bestX, bestY,
                    0, 0,
                    bestA, bestS );
        }
        if (i % 100 == 0) {
            while (SDL_PollEvent(&event)) {
                }
            sdl_draw(renderer, reconstructed);
            std::cout << "currently at " << i << " 100 gens took " << t.get() <<
             "\n" << "sad is " << compute_sad(src_images[0].data,
                                                    reconstructed.data,
                                                    reconstructed.width * reconstructed.height * 4) << "\n";

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

void sdl_draw(SDL_Renderer* renderer, const image& img) {
    SDL_Texture *intermediate = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STREAMING, img.width, img.height);
    SDL_UpdateTexture(intermediate, NULL, (void*)img.data, 4 * img.width);

    SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, intermediate, NULL, NULL);
	SDL_RenderPresent(renderer);

    SDL_Delay(0);
    SDL_DestroyTexture(intermediate);
}
