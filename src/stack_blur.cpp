#include "stack_blur.h"

#include "i32x4.h"

#include <algorithm>

namespace StackBlur {
    static unsigned short const stackblur_mul[255] = {
            512, 512, 456, 512, 328, 456, 335, 512, 405, 328, 271, 456, 388, 335, 292, 512,
            454, 405, 364, 328, 298, 271, 496, 456, 420, 388, 360, 335, 312, 292, 273, 512,
            482, 454, 428, 405, 383, 364, 345, 328, 312, 298, 284, 271, 259, 496, 475, 456,
            437, 420, 404, 388, 374, 360, 347, 335, 323, 312, 302, 292, 282, 273, 265, 512,
            497, 482, 468, 454, 441, 428, 417, 405, 394, 383, 373, 364, 354, 345, 337, 328,
            320, 312, 305, 298, 291, 284, 278, 271, 265, 259, 507, 496, 485, 475, 465, 456,
            446, 437, 428, 420, 412, 404, 396, 388, 381, 374, 367, 360, 354, 347, 341, 335,
            329, 323, 318, 312, 307, 302, 297, 292, 287, 282, 278, 273, 269, 265, 261, 512,
            505, 497, 489, 482, 475, 468, 461, 454, 447, 441, 435, 428, 422, 417, 411, 405,
            399, 394, 389, 383, 378, 373, 368, 364, 359, 354, 350, 345, 341, 337, 332, 328,
            324, 320, 316, 312, 309, 305, 301, 298, 294, 291, 287, 284, 281, 278, 274, 271,
            268, 265, 262, 259, 257, 507, 501, 496, 491, 485, 480, 475, 470, 465, 460, 456,
            451, 446, 442, 437, 433, 428, 424, 420, 416, 412, 408, 404, 400, 396, 392, 388,
            385, 381, 377, 374, 370, 367, 363, 360, 357, 354, 350, 347, 344, 341, 338, 335,
            332, 329, 326, 323, 320, 318, 315, 312, 310, 307, 304, 302, 299, 297, 294, 292,
            289, 287, 285, 282, 280, 278, 275, 273, 271, 269, 267, 265, 263, 261, 259
    };

    static unsigned char const stackblur_shr[255] = {
            9, 11, 12, 13, 13, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17,
            17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19,
            19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20,
            20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21,
            21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
            21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22,
            22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
            22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23,
            23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
            23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
            23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
            23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
            24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
            24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
            24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
            24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24
    };

    void stack_blur(unsigned char *src, unsigned int w, unsigned int h, unsigned int stride,
                    unsigned int radius, int step, unsigned char *stack) {
        unsigned int x, y, xp, yp, i;
        unsigned int sp;
        unsigned int stack_start;
        unsigned char *stack_ptr;

        unsigned char *src_ptr;
        unsigned char *dst_ptr;

        unsigned long sum_r;
        unsigned long sum_g;
        unsigned long sum_b;
        unsigned long sum_a;
        unsigned long sum_in_r;
        unsigned long sum_in_g;
        unsigned long sum_in_b;
        unsigned long sum_in_a;
        unsigned long sum_out_r;
        unsigned long sum_out_g;
        unsigned long sum_out_b;
        unsigned long sum_out_a;

        unsigned int wm = w - 1;
        unsigned int hm = h - 1;
        unsigned int div = (radius * 2) + 1;
        unsigned int mul_sum = stackblur_mul[radius];
        unsigned char shr_sum = stackblur_shr[radius];

        if (step == 1) {
            for (y = 0; y < h; y++) {
                sum_r = sum_g = sum_b = sum_a =
                sum_in_r = sum_in_g = sum_in_b = sum_in_a =
                sum_out_r = sum_out_g = sum_out_b = sum_out_a = 0;

                // Start of line (0, y).
                src_ptr = src + stride * y;

                for (i = 0; i <= radius; i++) {
                    stack_ptr = &stack[4 * i];
                    stack_ptr[0] = src_ptr[0];
                    stack_ptr[1] = src_ptr[1];
                    stack_ptr[2] = src_ptr[2];
                    stack_ptr[3] = src_ptr[3];
                    sum_r += src_ptr[0] * (i + 1);
                    sum_g += src_ptr[1] * (i + 1);
                    sum_b += src_ptr[2] * (i + 1);
                    sum_a += src_ptr[3] * (i + 1);
                    sum_out_r += src_ptr[0];
                    sum_out_g += src_ptr[1];
                    sum_out_b += src_ptr[2];
                    sum_out_a += src_ptr[3];
                }

                for (i = 1; i <= radius; i++) {
                    if (i <= wm) src_ptr += 4;
                    stack_ptr = &stack[4 * (i + radius)];
                    stack_ptr[0] = src_ptr[0];
                    stack_ptr[1] = src_ptr[1];
                    stack_ptr[2] = src_ptr[2];
                    stack_ptr[3] = src_ptr[3];
                    sum_r += src_ptr[0] * (radius + 1 - i);
                    sum_g += src_ptr[1] * (radius + 1 - i);
                    sum_b += src_ptr[2] * (radius + 1 - i);
                    sum_a += src_ptr[3] * (radius + 1 - i);
                    sum_in_r += src_ptr[0];
                    sum_in_g += src_ptr[1];
                    sum_in_b += src_ptr[2];
                    sum_in_a += src_ptr[3];
                }

                sp = radius;
                xp = radius;
                if (xp > wm) xp = wm;

                dst_ptr = src + y * stride; // img.pix_ptr(0, y)
                src_ptr = dst_ptr + 4 * xp; // img.pix_ptr(xp, y)

                for (x = 0; x < w; x++) {
                    dst_ptr[0] = (sum_r * mul_sum) >> shr_sum;
                    dst_ptr[1] = (sum_g * mul_sum) >> shr_sum;
                    dst_ptr[2] = (sum_b * mul_sum) >> shr_sum;
                    dst_ptr[3] = (sum_a * mul_sum) >> shr_sum;
                    dst_ptr += 4;

                    sum_r -= sum_out_r;
                    sum_g -= sum_out_g;
                    sum_b -= sum_out_b;
                    sum_a -= sum_out_a;

                    stack_start = sp + div - radius;
                    if (stack_start >= div) stack_start -= div;
                    stack_ptr = &stack[4 * stack_start];

                    sum_out_r -= stack_ptr[0];
                    sum_out_g -= stack_ptr[1];
                    sum_out_b -= stack_ptr[2];
                    sum_out_a -= stack_ptr[3];

                    if (xp < wm) {
                        src_ptr += 4;
                        ++xp;
                    }

                    stack_ptr[0] = src_ptr[0];
                    stack_ptr[1] = src_ptr[1];
                    stack_ptr[2] = src_ptr[2];
                    stack_ptr[3] = src_ptr[3];

                    sum_in_r += src_ptr[0];
                    sum_in_g += src_ptr[1];
                    sum_in_b += src_ptr[2];
                    sum_in_a += src_ptr[3];
                    sum_r += sum_in_r;
                    sum_g += sum_in_g;
                    sum_b += sum_in_b;
                    sum_a += sum_in_a;

                    ++sp;
                    if (sp >= div) sp = 0;
                    stack_ptr = &stack[sp * 4];

                    sum_out_r += stack_ptr[0];
                    sum_out_g += stack_ptr[1];
                    sum_out_b += stack_ptr[2];
                    sum_out_a += stack_ptr[3];
                    sum_in_r -= stack_ptr[0];
                    sum_in_g -= stack_ptr[1];
                    sum_in_b -= stack_ptr[2];
                    sum_in_a -= stack_ptr[3];
                }
            }
        }

        // step 2
        if (step == 2) {
            for (x = 0; x < w; x++) {
                sum_r = sum_g = sum_b = sum_a =
                sum_in_r = sum_in_g = sum_in_b = sum_in_a =
                sum_out_r = sum_out_g = sum_out_b = sum_out_a = 0;

                src_ptr = src + 4 * x; // (x, 0)

                for (i = 0; i <= radius; i++) {
                    stack_ptr = &stack[i * 4];
                    stack_ptr[0] = src_ptr[0];
                    stack_ptr[1] = src_ptr[1];
                    stack_ptr[2] = src_ptr[2];
                    stack_ptr[3] = src_ptr[3];
                    sum_r += src_ptr[0] * (i + 1);
                    sum_g += src_ptr[1] * (i + 1);
                    sum_b += src_ptr[2] * (i + 1);
                    sum_a += src_ptr[3] * (i + 1);
                    sum_out_r += src_ptr[0];
                    sum_out_g += src_ptr[1];
                    sum_out_b += src_ptr[2];
                    sum_out_a += src_ptr[3];
                }

                for (i = 1; i <= radius; i++) {
                    if (i <= hm) src_ptr += stride; // +stride

                    stack_ptr = &stack[4 * (i + radius)];
                    stack_ptr[0] = src_ptr[0];
                    stack_ptr[1] = src_ptr[1];
                    stack_ptr[2] = src_ptr[2];
                    stack_ptr[3] = src_ptr[3];
                    sum_r += src_ptr[0] * (radius + 1 - i);
                    sum_g += src_ptr[1] * (radius + 1 - i);
                    sum_b += src_ptr[2] * (radius + 1 - i);
                    sum_a += src_ptr[3] * (radius + 1 - i);
                    sum_in_r += src_ptr[0];
                    sum_in_g += src_ptr[1];
                    sum_in_b += src_ptr[2];
                    sum_in_a += src_ptr[3];
                }

                sp = radius;
                yp = radius;
                if (yp > hm) yp = hm;

                dst_ptr = src + 4 * x; // img.pix_ptr(x, 0)
                src_ptr = dst_ptr + yp * stride; // img.pix_ptr(x, yp)

                for (y = 0; y < h; y++) {
                    dst_ptr[0] = (sum_r * mul_sum) >> shr_sum;
                    dst_ptr[1] = (sum_g * mul_sum) >> shr_sum;
                    dst_ptr[2] = (sum_b * mul_sum) >> shr_sum;
                    dst_ptr[3] = (sum_a * mul_sum) >> shr_sum;
                    dst_ptr += stride;

                    sum_r -= sum_out_r;
                    sum_g -= sum_out_g;
                    sum_b -= sum_out_b;
                    sum_a -= sum_out_a;

                    stack_start = sp + div - radius;
                    if (stack_start >= div) stack_start -= div;
                    stack_ptr = &stack[4 * stack_start];

                    sum_out_r -= stack_ptr[0];
                    sum_out_g -= stack_ptr[1];
                    sum_out_b -= stack_ptr[2];
                    sum_out_a -= stack_ptr[3];

                    if (yp < hm) {
                        src_ptr += stride; // +stride
                        ++yp;
                    }

                    stack_ptr[0] = src_ptr[0];
                    stack_ptr[1] = src_ptr[1];
                    stack_ptr[2] = src_ptr[2];
                    stack_ptr[3] = src_ptr[3];

                    sum_in_r += src_ptr[0];
                    sum_in_g += src_ptr[1];
                    sum_in_b += src_ptr[2];
                    sum_in_a += src_ptr[3];
                    sum_r += sum_in_r;
                    sum_g += sum_in_g;
                    sum_b += sum_in_b;
                    sum_a += sum_in_a;

                    ++sp;
                    if (sp >= div) sp = 0;
                    stack_ptr = &stack[sp * 4];

                    sum_out_r += stack_ptr[0];
                    sum_out_g += stack_ptr[1];
                    sum_out_b += stack_ptr[2];
                    sum_out_a += stack_ptr[3];
                    sum_in_r -= stack_ptr[0];
                    sum_in_g -= stack_ptr[1];
                    sum_in_b -= stack_ptr[2];
                    sum_in_a -= stack_ptr[3];
                }
            }
        }
    }

    void stack_blur_simd(unsigned char *src, unsigned int w, unsigned int h, unsigned int stride,
                         unsigned int radius, int step, I32x4 *stack) {
        unsigned int x, y, xp, yp, i;
        unsigned int sp;
        unsigned int stack_start;
        I32x4 *stack_ptr;

        unsigned char *src_ptr;
        unsigned char *dst_ptr;

        unsigned int wm = w - 1;
        unsigned int hm = h - 1;
        unsigned int div = (radius * 2) + 1;
        auto mul_sum = I32x4::splat(stackblur_mul[radius]);
        unsigned char shr_sum = stackblur_shr[radius];

        uint32_t val[4];

        // Step 1.
        if (step == 1) {
            for (y = 0; y < h; y++) {
                auto sum = I32x4::splat(0);
                auto sum_in = I32x4::splat(0);
                auto sum_out = I32x4::splat(0);

                // Start of line (0, y).
                src_ptr = src + stride * y;

                for (i = 0; i <= radius; i++) {
                    stack_ptr = &stack[i];

                    *stack_ptr = I32x4(src_ptr[0], src_ptr[1], src_ptr[2], src_ptr[3]);

                    sum += *stack_ptr * I32x4::splat(i + 1);

                    sum_out += *stack_ptr;
                }

                for (i = 1; i <= radius; i++) {
                    if (i <= wm) {
                        src_ptr += 4;
                    }
                    stack_ptr = &stack[i + radius];

                    *stack_ptr = I32x4(src_ptr[0], src_ptr[1], src_ptr[2], src_ptr[3]);

                    sum += *stack_ptr * I32x4::splat(radius + 1 - i);
                    sum_in += *stack_ptr;
                }

                sp = radius;
                xp = radius;
                if (xp > wm) {
                    xp = wm;
                }

                dst_ptr = src + y * stride; // img.pix_ptr(0, y)
                src_ptr = dst_ptr + 4 * xp; // img.pix_ptr(xp, y)

                for (x = 0; x < w; x++) {
                    auto temp = sum * mul_sum;
                    temp = temp.shift_r(shr_sum);
                    memcpy(val, &temp, sizeof(val));

                    dst_ptr[0] = val[0];
                    dst_ptr[1] = val[1];
                    dst_ptr[2] = val[2];
                    dst_ptr[3] = val[3];

                    dst_ptr += 4;

                    sum -= sum_out;

                    stack_start = sp + div - radius;
                    if (stack_start >= div) {
                        stack_start -= div;
                    }
                    stack_ptr = &stack[stack_start];

                    sum_out -= *stack_ptr;

                    if (xp < wm) {
                        src_ptr += 4;
                        ++xp;
                    }

                    *stack_ptr = I32x4(src_ptr[0], src_ptr[1], src_ptr[2], src_ptr[3]);

                    sum_in += *stack_ptr;
                    sum += sum_in;

                    ++sp;
                    if (sp >= div) {
                        sp = 0;
                    }
                    stack_ptr = &stack[sp];

                    sum_out += *stack_ptr;
                    sum_in -= *stack_ptr;
                }
            }
        }

        // Step 2.
        if (step == 2) {
            for (x = 0; x < w; x++) {
                auto sum = I32x4::splat(0);
                auto sum_in = I32x4::splat(0);
                auto sum_out = I32x4::splat(0);

                src_ptr = src + 4 * x; // (x, 0)

                for (i = 0; i <= radius; i++) {
                    stack_ptr = &stack[i];

                    *stack_ptr = I32x4(src_ptr[0], src_ptr[1], src_ptr[2], src_ptr[3]);

                    sum += *stack_ptr * I32x4::splat(i + 1);
                    sum_out += *stack_ptr;
                }

                for (i = 1; i <= radius; i++) {
                    if (i <= hm) {
                        src_ptr += stride;
                    }

                    stack_ptr = &stack[i + radius];

                    *stack_ptr = I32x4(src_ptr[0], src_ptr[1], src_ptr[2], src_ptr[3]);

                    sum += *stack_ptr * I32x4::splat(radius + 1 - i);
                    sum_in += *stack_ptr;
                }

                sp = radius;
                yp = radius;
                if (yp > hm) {
                    yp = hm;
                }

                dst_ptr = src + 4 * x; // img.pix_ptr(x, 0)
                src_ptr = dst_ptr + yp * stride; // img.pix_ptr(x, yp)

                for (y = 0; y < h; y++) {
                    auto temp = sum * mul_sum;
                    temp = temp.shift_r(shr_sum);
                    memcpy(val, &temp, sizeof(val));

                    dst_ptr[0] = val[0];
                    dst_ptr[1] = val[1];
                    dst_ptr[2] = val[2];
                    dst_ptr[3] = val[3];

                    dst_ptr += stride;

                    sum -= sum_out;

                    stack_start = sp + div - radius;
                    if (stack_start >= div) {
                        stack_start -= div;
                    }
                    stack_ptr = &stack[stack_start];

                    sum_out = sum_out - *stack_ptr;

                    if (yp < hm) {
                        src_ptr += stride;
                        ++yp;
                    }

                    *stack_ptr = I32x4(src_ptr[0], src_ptr[1], src_ptr[2], src_ptr[3]);

                    sum_in += *stack_ptr;
                    sum += sum_in;

                    ++sp;
                    if (sp >= div) {
                        sp = 0;
                    }
                    stack_ptr = &stack[sp];

                    sum_out += *stack_ptr;
                    sum_in -= *stack_ptr;
                }
            }
        }
    }

    void do_stack_blur(unsigned char *image_data, unsigned int width, unsigned int height,
                       unsigned int stride, unsigned int blur_x, unsigned int blur_y) {
        unsigned char stack_buffer[4 * (254 * 2 + 1)] = {0};

        if (blur_x > 0) {
            blur_x = std::clamp(blur_x, 1u, 254u);

            stack_blur(image_data, width, height, stride, blur_x, 1, stack_buffer);
        }

        if (blur_y > 0) {
            blur_y = std::clamp(blur_y, 1u, 254u);

            stack_blur(image_data, width, height, stride, blur_y, 2, stack_buffer);
        }
    }

    void do_stack_blur_simd(unsigned char *image_data, unsigned int width, unsigned int height,
                            unsigned int stride, unsigned int blur_x, unsigned int blur_y) {
        I32x4 stack_buffer_i32[254 * 2 + 1];

        if (blur_x > 0) {
            blur_x = std::clamp(blur_x, 1u, 254u);

            stack_blur_simd(image_data, width, height, stride, blur_x, 1, stack_buffer_i32);
        }

        if (blur_y > 0) {
            blur_y = std::clamp(blur_y, 1u, 254u);

            stack_blur_simd(image_data, width, height, stride, blur_y, 2, stack_buffer_i32);
        }
    }
}
