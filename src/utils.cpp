#include "utils.h"
#include <SDL.h>
#include <SDL_image.h>

#include <immintrin.h>
#include <filesystem>
#include <execution>
#include <iostream>
#include <math.h>

void load_images(std::string path, std::vector<SrcImage>& images) {
    int ind = 0;

    for (const auto & entry : std::filesystem::directory_iterator(path)) {
        SrcImage img;
        img.data = NULL;
        img.path = entry.path().string();
        images.push_back(img);
        ind++;
    }

    std::for_each(std::execution::par_unseq, images.begin(), images.end(), [](SrcImage& image){
        std::cout << "loading " << image.path << "\n";
        SDL_Surface* surf = IMG_Load(image.path.c_str());
        if (surf == NULL) {
            std::cout << "encountering error on " << image.path << " skipping " << SDL_GetError() << std::endl;
            return;
        }
        float scale = std::sqrt(PIXELS_PER_IMAGE/(surf->w*surf->h));
        SDL_Surface* scaled_surf = SDL_CreateRGBSurface(0,surf->w * scale,surf->h * scale,32,0,0,0,0);
        SDL_Surface* formatted_surf = SDL_ConvertSurface(surf, scaled_surf->format, NULL);
        if (scaled_surf == NULL || formatted_surf == NULL) {
            std::cout << "encountering error on " << image.path << " skipping " << SDL_GetError() << std::endl;
            return;
        }

        int k = SDL_BlitScaled(formatted_surf, NULL, scaled_surf, NULL);
        if (k) {
            std::cout << "encountering error on " << image.path << " skipping " << SDL_GetError() << std::endl;
            return;
        }
        image.width = scaled_surf->w;
        image.height = scaled_surf->h;
        image.pitch = image.width * 4;
        image.sad = -1;
        image.data = new uint8_t[image.width * image.height * 4];
        simd_memcpy(image.data, (uint8_t*)scaled_surf->pixels, image.width * image.height * 4);

        SDL_FreeSurface(scaled_surf);
        SDL_FreeSurface(formatted_surf);
        SDL_FreeSurface(surf);

    });

    for (auto it = images.begin(); it != images.end(); ++it) {
        if ((*it).data == NULL) {
            images.erase(it);
            it -= 1;
        }
    }

}

int compute_sad(uint8_t* image, uint8_t* target, size_t num_bytes) {
    int sum = 0;
    __m512i a;
    __m512i b;
    __m512i sad_vec;
    int i;
    int bb = num_bytes - 64;
    for (i = 0; i < bb; i += 64) {
        a = _mm512_loadu_epi8(image + i);
        b = _mm512_loadu_epi8(target + i);
        sad_vec = _mm512_sad_epu8(a, b);

        sum += _mm512_reduce_add_epi64(sad_vec);
    }

    for (;i < num_bytes; i++) {
        if (image[i] > target[i])
            sum += image[i] - target[i];
        else
            sum += target[i] - image[i];
    }

    return sum;
}

int compute_sse_naive(uint8_t* image, uint8_t* target, size_t num_bytes) {
    int sum = 0;

    for (int i = 0; i < num_bytes; i++) {
        int dif = image[i] - target[i];
        sum += dif * dif;
    }

    return sum;
}

void simd_memcpy(uint8_t* dst, uint8_t* src, size_t num_bytes) {
    // should i convert this to uint32_t ?
    // I think benchmarks did not show significant improvement
    __m512i a;
    int i;
    int bb = num_bytes - 64;
    for (i = 0; i < bb; i += 64) {
        a = _mm512_loadu_epi8(src + i);
        _mm512_storeu_epi8(dst + i, a);
    }

    for (;i < num_bytes; i++) {
        dst[i] = src[i];
    }
}

int compute_sad_naive(uint8_t* image, uint8_t* target, size_t num_bytes) {
    int sum = 0;
    int i;
    for (i = 0; i < num_bytes; i++) {
        if (image[i] > target[i])
            sum += image[i] - target[i];
        else
            sum += target[i] - image[i];
    }
    return sum;
}
