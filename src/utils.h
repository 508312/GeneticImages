#ifndef UTILS_H
#define UTILS_H
#include <stdint.h>
#include <string>
#include <vector>

/* Img utils */
typedef struct SrcImage {
    uint16_t width;
    uint16_t height;
    uint16_t pitch;
    uint32_t sad;
    std::string path;
    uint8_t* data;
};

void load_images(int px_per_image, std::string path, std::vector<SrcImage>&images);

/* Math utils */
int compute_sad(uint8_t* image, uint8_t* target, size_t num_bytes);
int compute_sad_naive(uint8_t* image, uint8_t* target, size_t num_bytes);
uint64_t compute_sse_naive(uint8_t* image, uint8_t* target, size_t num_bytes);
void simd_memcpy(uint8_t* dst, uint8_t* src, size_t num_bytes);

uint8_t* SobelSimd(uint8_t* inputImage, int width, int height, int pitch);
uint8_t* to_greyscale(uint8_t* inputImage, int width, int height, int pitch);

#endif // UTILS_H
