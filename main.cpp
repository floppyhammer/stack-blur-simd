#include "src/stack_blur.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <chrono>
#include <iostream>

int main() {
    // Load image.
    int width, height, channels;
    unsigned char *img_data = stbi_load("../res/ferris.png", &width, &height, &channels, 0);

    // Image stride bytes.
    auto stride = width * channels * sizeof(unsigned char);

    // Image has to be 4-channel.
    if (img_data == nullptr || channels != 4) {
        std::cout << "Failed to load image or the number of channels isn't 4!" << std::endl;
        abort();
    }

    // Make a copy of the image data.
    auto *img_data_copy = new unsigned char[width * height * channels];
    memcpy(img_data_copy, img_data, stride * height);

    // Non SIMD.
    {
        auto start_time = std::chrono::steady_clock::now();

        StackBlur::do_stack_blur(img_data, width, height, stride, 16, 16);

        std::chrono::duration<double> elapsed_time = std::chrono::steady_clock::now() - start_time;
        std::cout << "Time cost " << std::round(elapsed_time.count() * 10000.0f) * 0.1f << " ms" << std::endl;
    }

    // SIMD.
    {
        auto start_time = std::chrono::steady_clock::now();

        StackBlur::do_stack_blur_simd(img_data_copy, width, height, stride, 16, 16);

        std::chrono::duration<double> elapsed_time = std::chrono::steady_clock::now() - start_time;
        std::cout << "Time cost (SIMD) " << std::round(elapsed_time.count() * 10000.0f) * 0.1f << " ms" << std::endl;
    }

    // Save results.
    stbi_write_png("../res/ferris_blur.png", width, height, channels, img_data, stride);
    stbi_write_png("../res/ferris_blur_simd.png", width, height, channels, img_data_copy, stride);

    // Clean up.
    stbi_image_free(img_data);
    delete[] img_data_copy;

    return 0;
}
