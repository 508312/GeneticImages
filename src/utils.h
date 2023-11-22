#ifndef UTILS_H
#define UTILS_H
#include <stdint.h>
#include <string>
#include <vector>

//TODO: do this better
#define PIXELS_PER_IMAGE 50000.0

/* Img utils */
typedef struct image_t {
    uint16_t width;
    uint16_t height;
    uint16_t pitch;
    uint32_t sad;
    std::string path;
    uint8_t* data;
};

void load_images(std::string path, std::vector<image_t>&images);

/* Math utils */
int compute_sad(uint8_t* image, uint8_t* target, size_t num_bytes);
int compute_sad_naive(uint8_t* image, uint8_t* target, size_t num_bytes);
void simd_memcpy(uint8_t* dst, uint8_t* src, size_t num_bytes);

#endif // UTILS_H
