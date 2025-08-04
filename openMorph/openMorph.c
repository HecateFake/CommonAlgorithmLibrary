/*
 * openMorph.c
 *
 *  Created on: 2025年8月4日
 *      Author: Hecate
 */

#include "openMorph.h"
#include <stdlib.h>

void morphErode(binary* src, uint16_t width, uint16_t height, binary* kernel, uint16_t kWidth, uint16_t kHeight, binary* dst, binary background)
{
    uint16_t pixelCount = width * height;
    uint16_t kPixelCount = kWidth * kHeight;

    for (uint16_t i = 0; i < pixelCount; i++)
    {
        uint16_t y = i / width;
        uint16_t x = i % width;

        binary result = !background;

        // 遍历内核，但减少不必要的计算
        for (uint16_t j = 0; j < kPixelCount && result == !background; j++)
        {
            if (kernel[j] == !background)
            {
                uint16_t ky = j / kWidth;
                uint16_t kx = j % kWidth;
                int px = x + kx - kWidth / 2;
                int py = y + ky - kHeight / 2;

                result = (px < 0 || px >= width || py < 0 || py >= height) ? background : (src[py * width + px] == background ? background : result);
            }
        }

        dst[i] = result;
    }
}

void morphDilate(binary* src, uint16_t width, uint16_t height, binary* kernel, uint16_t kWidth, uint16_t kHeight, binary* dst, binary background)
{
    uint16_t pixelCount = width * height;
    uint16_t kPixelCount = kWidth * kHeight;

    for (uint16_t i = 0; i < pixelCount; i++)
    {
        uint16_t y = i / width;
        uint16_t x = i % width;

        binary result = background;

        // 遍历内核，但减少不必要的计算
        for (uint16_t j = 0; j < kPixelCount && result == background; j++)
        {
            if (kernel[j] == !background)
            {
                uint16_t ky = j / kWidth;
                uint16_t kx = j % kWidth;
                int px = x + kx - kWidth / 2;
                int py = y + ky - kHeight / 2;

                result = (px < 0 || px >= width || py < 0 || py >= height) ? background : (src[py * width + px] == background ? result : !background);
            }
        }

        dst[i] = result;
    }
}

void morphOpen(binary* src, uint16_t width, uint16_t height, binary* kernel, uint16_t kWidth, uint16_t kHeight, binary* dst, binary background)
{
    binary* temp = (binary*) malloc(width * height * sizeof(binary));

    if (!temp) return;

    morphErode(src, width, height, kernel, kWidth, kHeight, temp, background);
    morphDilate(temp, width, height, kernel, kWidth, kHeight, dst, background);

    free(temp);
}

void morphClose(binary* src, uint16_t width, uint16_t height, binary* kernel, uint16_t kWidth, uint16_t kHeight, binary* dst, binary background)
{
    binary* temp = (binary*) malloc(width * height * sizeof(binary));

    if (!temp) return;

    morphDilate(src, width, height, kernel, kWidth, kHeight, temp, background);
    morphErode(temp, width, height, kernel, kWidth, kHeight, dst, background);

    free(temp);
}

void toUint8(binary* src, uint16_t width, uint16_t height, uint8_t* dst)
{
    for (uint16_t i = 0; i < width * height; i++)
    {
        dst[i] = src[i] ? 255 : 0;  // 将二值图像转换为8位灰度图像
    }
}

void toBinary(uint8_t* src, uint16_t width, uint16_t height, binary* dst)
{
    for (uint16_t i = 0; i < width * height; i++)
    {
        dst[i] = (src[i] > 0) ? 1 : 0;  // 将8位灰度图像转换为二值图像
    }
}
