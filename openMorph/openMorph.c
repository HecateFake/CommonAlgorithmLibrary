/*
 * openMorph.c
 *
 *  Created on: 2025年8月4日
 *      Author: Hecate
 */


#include "openMorph.h"
#include <stdlib.h>

static inline binary getPixel(binary* img, uint8_t width, uint8_t height, int x, int y, binary background)
{
    if (x < 0 || x >= width || y < 0 || y >= height)
        return background;
    return img[y * width + x];
}

static inline void setPixel(binary* img, uint8_t width, int x, int y, binary value)
{
    img[y * width + x] = value;
}

void morphErode(binary* src, uint8_t width, uint8_t height, binary* kernel, uint8_t kWidth, uint8_t kHeight, binary* dst, binary background)
{
    uint8_t kCenterX = kWidth / 2;
    uint8_t kCenterY = kHeight / 2;

    for (uint8_t y = 0; y < height; y++)
    {
        for (uint8_t x = 0; x < width; x++)
        {
            binary result = !background;

            for (uint8_t ky = 0; ky < kHeight && result == !background; ky++)
            {
                for (uint8_t kx = 0; kx < kWidth && result == !background; kx++)
                {
                    if (kernel[ky * kWidth + kx] == !background)
                    {
                        int px = x + kx - kCenterX;
                        int py = y + ky - kCenterY;

                        if (getPixel(src, width, height, px, py, background) == background)
                        {
                            result = background;
                        }
                    }
                }
            }

            setPixel(dst, width, x, y, result);
        }
    }
}

void morphDilate(binary* src, uint8_t width, uint8_t height, binary* kernel, uint8_t kWidth, uint8_t kHeight, binary* dst, binary background)
{
    uint8_t kCenterX = kWidth / 2;
    uint8_t kCenterY = kHeight / 2;

    for (uint8_t y = 0; y < height; y++)
    {
        for (uint8_t x = 0; x < width; x++)
        {
            binary result = background;  // 膨胀初始为背景色

            for (uint8_t ky = 0; ky < kHeight && result == background; ky++)
            {
                for (uint8_t kx = 0; kx < kWidth && result == background; kx++)
                {
                    if (kernel[ky * kWidth + kx] == !background)
                    {
                        int px = x + kx - kCenterX;
                        int py = y + ky - kCenterY;

                        if (getPixel(src, width, height, px, py, background) == !background)
                        {
                            result = !background;
                        }
                    }
                }
            }

            setPixel(dst, width, x, y, result);
        }
    }
}

void morphOpen(binary* src, uint8_t width, uint8_t height, binary* kernel, uint8_t kWidth, uint8_t kHeight, binary* dst, binary background)
{
    // 动态分配临时缓冲区
    binary* temp = (binary*) malloc(width * height * sizeof(binary));
    if (temp == NULL)
    {
        return;  // 内存分配失败，直接返回
    }

    // 先腐蚀
    morphErode(src, width, height, kernel, kWidth, kHeight, temp, background);
    // 后膨胀
    morphDilate(temp, width, height, kernel, kWidth, kHeight, dst, background);

    // 释放临时缓冲区
    free(temp);
}

void morphClose(binary* src, uint8_t width, uint8_t height, binary* kernel, uint8_t kWidth, uint8_t kHeight, binary* dst, binary background)
{
    // 动态分配临时缓冲区
    binary* temp = (binary*) malloc(width * height * sizeof(binary));
    if (temp == NULL)
    {
        return;  // 内存分配失败，直接返回
    }

    // 先膨胀
    morphDilate(src, width, height, kernel, kWidth, kHeight, temp, background);
    // 后腐蚀
    morphErode(temp, width, height, kernel, kWidth, kHeight, dst, background);

    // 释放临时缓冲区
    free(temp);
}