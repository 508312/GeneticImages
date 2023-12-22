#include "utils.h"
#include <SDL.h>
#include <SDL_image.h>

#include <immintrin.h>
#include <filesystem>
#include <execution>
#include <iostream>
#include <math.h>

void load_images(int px_per_image, std::string path, std::vector<SrcImage>& images) {
    int ind = 0;

    for (const auto & entry : std::filesystem::directory_iterator(path)) {
        SrcImage img;
        img.data = NULL;
        img.path = entry.path().string();
        images.push_back(img);
        ind++;
    }

    std::for_each(std::execution::par_unseq, images.begin(), images.end(), [px_per_image](SrcImage& image){
        std::cout << "loading " << image.path << "\n";
        SDL_Surface* surf = IMG_Load(image.path.c_str());
        if (surf == NULL) {
            std::cout << "encountering error on " << image.path << " skipping " << SDL_GetError() << std::endl;
            return;
        }
        double scale = std::sqrt(std::min(px_per_image/(double)(surf->w*surf->h), (double)1.0));
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

uint64_t compute_sse_naive(uint8_t* image, uint8_t* target, size_t num_bytes) {
    uint64_t sum = 0;

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

// ARGB
uint8_t* SobelSimd(uint8_t* inputImage, int width, int height, int pitch) {
    uint8_t* outputImage = new uint8_t[height * pitch];
    const int border = 8;

    __m128i p1, p2, p3, p4, p5, p6, p7, p8, p9;
    __m128i gx, gy, temp, G;

    uint8_t* outputPointer = outputImage;
    uint8_t* inputPointer = inputImage;

    for (int i = (border - 1); i < height - (border + 1); i += 1)
    {
        for (int j = (border - 1); j < pitch - (2 * border - 1); j += 8)
        {

            /*
            Sobel operator input matrix
            +~~~~~~~~~~~~~~+
            | p1 | p2 | p3 |
            |~~~~+~~~~+~~~~+
            | p4 | p5 | p6 |
            |~~~~+~~~~+~~~~+
            | p7 | p8 | p9 |
            +~~~~+~~~~+~~~~+
            */

            /*
            __m128i _mm_loadu_si128 (__m128i const* mem_addr)
            Load 128-bits of integer data from memory into dst. mem_addr does not need to be aligned on any particular boundary.
            */

            p1 = _mm_loadu_si128((__m128i *)(inputPointer + i * pitch + j));
            p2 = _mm_loadu_si128((__m128i *)(inputPointer + i * pitch + j + 1));
            p3 = _mm_loadu_si128((__m128i *)(inputPointer + i * pitch + j + 2));

            p4 = _mm_loadu_si128((__m128i *)(inputPointer + (i + 1) * pitch + j));
            p5 = _mm_loadu_si128((__m128i *)(inputPointer + (i + 1) * pitch + j + 1));
            p6 = _mm_loadu_si128((__m128i *)(inputPointer + (i + 1) * pitch + j + 2));

            p7 = _mm_loadu_si128((__m128i *)(inputPointer + (i + 2) * pitch + j));
            p8 = _mm_loadu_si128((__m128i *)(inputPointer + (i + 2) * pitch + j + 1));
            p9 = _mm_loadu_si128((__m128i *)(inputPointer + (i + 2) * pitch + j + 2));

            /*
            __m128i _mm_srli_epi16 (__m128i a, int imm8)
            Shift packed 16-bit integers in a right by imm8 while shifting in zeros, and store the Gs in dst.

            __m128i _mm_unpacklo_epi8 (__m128i a, __m128i b)
            Unpack and interleave 8-bit integers from the low half of a and b, and store the Gs in dst.
            */

            // convert image 8-bit unsigned integer data to 16-bit signed integers to use in arithmetic operations
            p1 = _mm_srli_epi16(_mm_unpacklo_epi8(p1, p1), 8);
            p2 = _mm_srli_epi16(_mm_unpacklo_epi8(p2, p2), 8);
            p3 = _mm_srli_epi16(_mm_unpacklo_epi8(p3, p3), 8);
            p4 = _mm_srli_epi16(_mm_unpacklo_epi8(p4, p4), 8);
            p5 = _mm_srli_epi16(_mm_unpacklo_epi8(p5, p5), 8);
            p6 = _mm_srli_epi16(_mm_unpacklo_epi8(p6, p6), 8);
            p7 = _mm_srli_epi16(_mm_unpacklo_epi8(p7, p7), 8);
            p8 = _mm_srli_epi16(_mm_unpacklo_epi8(p8, p8), 8);
            p9 = _mm_srli_epi16(_mm_unpacklo_epi8(p9, p9), 8);

            /*
            __m128i _mm_add_epi16 (__m128i a, __m128i b)
            Add packed 16-bit integers in a and b, and store the Gs in dst.

            __m128i _mm_sub_epi16 (__m128i a, __m128i b)
            Subtract packed 16-bit integers in b from packed 16-bit integers in a, and store the Gs in dst.
            */

            // Calculating Gx = (p3 + 2 * p6 + p9) - (p1 + 2 * p4 + p7)
            gx = _mm_add_epi16(p6, p6);   // 2*p6
            gx = _mm_add_epi16(gx, p3);   // p3 + 2*p6
            gx = _mm_add_epi16(gx, p9);   // p3 + 2*p6 + p9
            gx = _mm_sub_epi16(gx, p1);   // p3 + 2*p6 + p9 - p1
            temp = _mm_add_epi16(p4, p4); // 2*p4
            gx = _mm_sub_epi16(gx, temp); // p3 + 2*p6 + p9 - (p1 + 2*p4)
            gx = _mm_sub_epi16(gx, p7);   // p3 + 2*p6 + p9 - (p1 + 2*p4 + p7)

            // Calculating Gy = (p1 + 2 * p2 + p3) - (p7 + 2 * p8 + p9)
            gy = _mm_add_epi16(p2, p2);   // 2*p2
            gy = _mm_add_epi16(gy, p1);   // p1 + 2*p2
            gy = _mm_add_epi16(gy, p3);   // p1 + 2*p2 + p3
            gy = _mm_sub_epi16(gy, p7);   // p1 + 2*p2 + p3 - p7
            temp = _mm_add_epi16(p8, p8); // 2*p8
            gy = _mm_sub_epi16(gy, temp); // p1 + 2*p2 + p3 - (p7 + 2*p8)
            gy = _mm_sub_epi16(gy, p9);   // p1 + 2*p2 + p3 - (p7 + 2*p8 + p9)

            /*
            __m128i _mm_abs_epi16 (__m128i a)
            Compute the absolute value of packed 16-bit integers in a, and store the unsigned Gs in dst.
            */

            gx = _mm_abs_epi16(gx); // |Gx|
            gy = _mm_abs_epi16(gy); // |Gy|

            // G = |Gx| + |Gy|
            G = _mm_add_epi16(gx, gy);

            /*
            __m128i _mm_packus_epi16 (__m128i a, __m128i b)
            Convert packed 16-bit integers from a and b to packed 8-bit integers using unsigned saturation, and store the results in dst.
            */
            G = _mm_packus_epi16(G, G);

            /*
            void _mm_storeu_si128 (__m128i* mem_addr, __m128i a)
            Store 128-bits of integer data from a into memory. mem_addr does not need to be aligned on any particular boundary.
            */
            _mm_storeu_si128((__m128i *)(outputPointer + (i + 1) * pitch + j + 1), G);
        }
    }
    return outputImage;
}


uint8_t* to_greyscale(uint8_t* inputImage, int width, int height, int pitch) {
    uint8_t* outputImage = new uint8_t[height * width];

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < pitch - 4; j+=4) {
            outputImage[i * width + j/4] = (inputImage[i * pitch + j + 0]
                            + inputImage[i * pitch + j + 1]
                            + inputImage[i * pitch + j + 2])/3;
        }
    }
    return outputImage;
}
