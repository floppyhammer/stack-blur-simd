#ifndef STACK_BLUR_H
#define STACK_BLUR_H

// The stack blur algorithm was invented by Mario Klingemann (mario@quasimondo.com)
// http://incubator.quasimondo.com/processing/fast_blur_deluxe.php

// The C++ RGBA (32-bit color) version by Victor Laskin (victor.laskin@gmail.com)
// http://vitiy.info/stackblur-algorithm-multi-threaded-blur-for-cpp

// The SIMD implementation by floppyhammer (tannhauser_chen@outlook.com)
// https://github.com/floppyhammer/stack-blur-simd

namespace StackBlur {
    /**
     * Do stack blur.
     * @param src Input image data
     * @param w Image width
     * @param h Image height
     * @param stride Row stride of the image data
     * @param blur_x Blur size in X direction
     * @param blur_y Blur size in Y direction
     */
    void do_stack_blur(unsigned char *image_data, unsigned int width, unsigned int height,
                       unsigned int stride, unsigned int blur_x, unsigned int blur_y);

    /**
     * Do stack blur (utilizing SIMD).
     * @param src Input image data
     * @param w Image width
     * @param h Image height
     * @param stride Row stride of the image data
     * @param blur_x Blur size in X direction
     * @param blur_y Blur size in Y direction
     */
    void do_stack_blur_simd(unsigned char *image_data, unsigned int width, unsigned int height,
                            unsigned int stride, unsigned int blur_x, unsigned int blur_y);
}

#endif //STACK_BLUR_H
