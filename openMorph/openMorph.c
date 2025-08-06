/*
 * openMorph.c
 *
 *  Created on: 2025年8月4日
 *      Author: Hecate
 */

#include "openMorph.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

// 全局分配大数组，避免栈溢出
static uint8_t g_morphBuffer[UNIFORMLY_PRE_ALLOCATED_BUFFER_SIZE];  // 形态学运算临时缓冲区
static uint16_t g_labMap[UNIFORMLY_PRE_ALLOCATED_BUFFER_SIZE];
static uint16_t g_labelEquivalence[1024];
static SBEA g_tempBea[100];  // 全局临时BEA数组，避免栈分配

void morphErode(CroppedImage8* src, CroppedImage8* kernel, CroppedImage8* dst, uint8_t background)
{
    uint16_t pixelCount = src->width * src->height;
    uint16_t kPixelCount = kernel->width * kernel->height;

    for (uint16_t i = 0; i < pixelCount; i++)
    {
        uint16_t y = i / src->width;
        uint16_t x = i % src->width;

        uint8_t result = cReverse(background);

        // 遍历内核，但减少不必要的计算
        for (uint16_t j = 0; j < kPixelCount && result == cReverse(background); j++)
        {
            if (kernel->data[j] == cReverse(background))
            {
                uint16_t kx = j % kernel->width;
                uint16_t ky = j / kernel->width;

                int32_t px = (int32_t) x + (int32_t) kx - (int32_t) (kernel->width >> 1);
                int32_t py = (int32_t) y + (int32_t) ky - (int32_t) (kernel->height >> 1);

                result = (px < 0 || px >= src->width || py < 0 || py >= src->height) ? background : (src->data[py * src->width + px] == background ? background : result);
            }
        }

        dst->data[i] = result;
    }
}

void morphDilate(CroppedImage8* src, CroppedImage8* kernel, CroppedImage8* dst, uint8_t background)
{
    uint16_t pixelCount = src->width * src->height;
    uint16_t kPixelCount = kernel->width * kernel->height;

    for (uint16_t i = 0; i < pixelCount; i++)
    {
        uint16_t y = i / src->width;
        uint16_t x = i % src->width;

        uint8_t result = background;

        // 遍历内核，但减少不必要的计算
        for (uint16_t j = 0; j < kPixelCount && result == background; j++)
        {
            if (kernel->data[j] == cReverse(background))
            {
                uint16_t kx = j % kernel->width;
                uint16_t ky = j / kernel->width;

                int32_t px = (int32_t) x + (int32_t) kx - (int32_t) (kernel->width >> 1);
                int32_t py = (int32_t) y + (int32_t) ky - (int32_t) (kernel->height >> 1);

                result = (px < 0 || px >= src->width || py < 0 || py >= src->height) ? background : (src->data[py * src->width + px] == background ? result : cReverse(background));
            }
        }

        dst->data[i] = result;
    }
}

void morphOpen(CroppedImage8* src, CroppedImage8* kernel, CroppedImage8* dst, uint8_t background)
{
    if (src->width * src->height > UNIFORMLY_PRE_ALLOCATED_BUFFER_SIZE) return;

    // 使用全局缓冲区，避免栈分配
    memset(g_morphBuffer, 0, src->width * src->height * sizeof(uint8_t));
    CroppedImage8 tempImg = {g_morphBuffer, src->width, src->height};

    morphErode(src, kernel, &tempImg, background);
    morphDilate(&tempImg, kernel, dst, background);
}

void morphClose(CroppedImage8* src, CroppedImage8* kernel, CroppedImage8* dst, uint8_t background)
{
    if (src->width * src->height > UNIFORMLY_PRE_ALLOCATED_BUFFER_SIZE) return;

    // 使用全局缓冲区，避免栈分配
    memset(g_morphBuffer, 0, src->width * src->height * sizeof(uint8_t));
    CroppedImage8 tempImg = {g_morphBuffer, src->width, src->height};

    morphDilate(src, kernel, &tempImg, background);
    morphErode(&tempImg, kernel, dst, background);
}

void toBinary(CroppedImage8* src, CroppedImage8* dst, uint8_t threshold)
{
    for (uint16_t i = 0; i < src->width * src->height; i++) dst->data[i] = (src->data[i] > threshold) ? WHITE : BLACK;  // 将8位灰度图像转换为二值图像
}

void toBinaryDoubleThreshold(CroppedImage8* src, CroppedImage8* dst, uint8_t thresholdUp, uint8_t thresholdDown)
{
    for (uint16_t i = 0; i < src->width * src->height; i++) dst->data[i] = ((src->data[i] < thresholdDown) ? BLACK : ((src->data[i] < thresholdUp) ? WHITE : BLACK));  // 根据上下阈值转换为二值图像
}

void rgb565toBinary(CroppedImage16* src, CroppedImage8* dst, uint8_t background, uint8_t r_min, uint8_t r_max, uint8_t g_min, uint8_t g_max, uint8_t b_min, uint8_t b_max)
{
    for (uint16_t i = 0; i < src->width * src->height; i++)
    {
        uint16_t pixel = src->data[i];

        // 从RGB565格式中提取RGB分量
        uint8_t r = (pixel >> 11) << 3;          // 红色分量 (5位 -> 8位)
        uint8_t g = ((pixel >> 5) & 0x3F) << 2;  // 绿色分量 (6位 -> 8位)
        uint8_t b = (pixel & 0x1F) << 3;         // 蓝色分量 (5位 -> 8位)

        // 根据范围判断是否为前景
        dst->data[i] = (r >= r_min && r <= r_max && g >= g_min && g <= g_max && b >= b_min && b <= b_max) ? cReverse(background) : background;
    }
}

void twoPassConnectedAreaInit(BEAINF* obj, uint8_t background, CroppedImage8* src, CroppedImage8* sho, uint16_t areaMin, uint16_t areaMax, float leftDeadZone, float rightDeadZone, float topDeadZone, float bottomDeadZone)
{
    obj->background = background;

    obj->data = src;
    obj->show = sho;

    obj->areaRange[0] = areaMin;
    obj->areaRange[1] = areaMax;

    obj->xRange[0] = (uint16_t) (leftDeadZone * src->width);
    obj->xRange[1] = (uint16_t) ((1.0f - rightDeadZone) * src->width);

    obj->yRange[0] = (uint16_t) (topDeadZone * src->height);
    obj->yRange[1] = (uint16_t) ((1.0f - bottomDeadZone) * src->height);

    obj->beaCount = 0;
    obj->lastFrameBeaExist = 0;  // 初始化为无有效区域
    obj->lastFrameSelectedBeaXY[0] = 0;
    obj->lastFrameSelectedBeaXY[1] = 0;
    obj->selectedIndex = 100;
}

void twoPassFourConnectedAreaProcess(BEAINF* obj)
{
    // first pass
    uint16_t pixCount = obj->data->width * obj->data->height;
    if (pixCount > UNIFORMLY_PRE_ALLOCATED_BUFFER_SIZE) return;
    uint16_t lab = 0;

    // 清零全局数组
    memset(g_labMap, 0, pixCount * sizeof(uint16_t));
    memset(g_labelEquivalence, 0, 1024 * sizeof(uint16_t));

    for (uint16_t i = 0; i < pixCount; i++)
    {
        if (lab >= 1024) continue;

        if (obj->data->data[i] == obj->background)
        {
            g_labMap[i] = 0;  // 背景像素标记为0
            continue;
        }

        uint16_t left = (i % obj->data->width == 0) ? 0 : g_labMap[i - 1];
        uint16_t up = (i < obj->data->width) ? 0 : g_labMap[i - obj->data->width];

        if (!left && !up)
        {
            g_labMap[i] = ++lab;  // 新标签
            g_labelEquivalence[lab] = lab;
        }
        else if (left && !up)
        {
            g_labMap[i] = left;  // 使用左边标签
        }
        else if (!left && up)
        {
            g_labMap[i] = up;  // 使用上边标签
        }
        else
        {
            // 两个标签合并 - 使用并查集防止环形引用
            uint16_t minLabel = (left < up) ? left : up;
            g_labMap[i] = minLabel;
            g_labelEquivalence[(left < up) ? up : left] = minLabel;
        }
    }

    // second pass
    // 清零全局临时BEA数组
    memset(g_tempBea, 0, 100 * sizeof(SBEA));
    uint16_t tempBeaCount = 0;

    // 路径压缩优化标签等价表
    for (uint16_t i = 1; i <= lab; i++)
    {
        uint16_t root = i;
        while (root != g_labelEquivalence[root]) root = g_labelEquivalence[root];
        g_labelEquivalence[i] = root;
    }

    for (uint16_t i = 0; i < pixCount; i++)
    {
        uint16_t curr_label = g_labMap[i];

        if (!curr_label) continue;

        // 使用优化后的等价表直接查找
        curr_label = g_labelEquivalence[curr_label];

        if (curr_label > 100) continue;  // 防止数组越界

        tempBeaCount = (curr_label > tempBeaCount) ? curr_label : tempBeaCount;

        g_tempBea[curr_label - 1].beaX += (i % obj->data->width);
        g_tempBea[curr_label - 1].beaY += (i / obj->data->width);
        g_tempBea[curr_label - 1].beaArea++;
    }

    // filter pass
    memcpy(obj->show->data, obj->data->data, pixCount * sizeof(uint8_t));
    obj->beaCount = 0;          // 重置计数器
    obj->selectedIndex = 100;   // 100表示未选中任何区域
    float minDistance = -1.0f;  // 使用-1表示未初始化

    for (uint16_t i = 0; i < tempBeaCount; i++)
    {
        // 跳过空区域
        if (g_tempBea[i].beaArea == 0 || obj->beaCount >= 100) continue;

        // 计算质心
        g_tempBea[i].beaX = (uint32_t) ((float) g_tempBea[i].beaX / (float) g_tempBea[i].beaArea);
        g_tempBea[i].beaY = (uint32_t) ((float) g_tempBea[i].beaY / (float) g_tempBea[i].beaArea);

        // 检查面积和位置约束
        if (g_tempBea[i].beaX < obj->xRange[0] || g_tempBea[i].beaX >= obj->xRange[1] ||
            g_tempBea[i].beaY < obj->yRange[0] || g_tempBea[i].beaY >= obj->yRange[1] ||
            g_tempBea[i].beaArea < obj->areaRange[0] || g_tempBea[i].beaArea > obj->areaRange[1])
            continue;

        // 将有效信标添加到结果数组
        obj->sbea[obj->beaCount].beaX = g_tempBea[i].beaX;
        obj->sbea[obj->beaCount].beaY = g_tempBea[i].beaY;
        obj->sbea[obj->beaCount].beaArea = g_tempBea[i].beaArea;

        // 选择逻辑：分两种情况处理
        if (obj->lastFrameBeaExist)
        {
            // 上帧有灯：选择距离上帧选中的灯距离最近的信标
            float distance = sqrtf(powf((float) obj->sbea[obj->beaCount].beaX - (float) obj->lastFrameSelectedBeaXY[0], 2) +
                                   powf((float) obj->sbea[obj->beaCount].beaY - (float) obj->lastFrameSelectedBeaXY[1], 2));

            if (minDistance < 0.0f || distance < minDistance)
            {
                obj->selectedIndex = obj->beaCount;
                minDistance = distance;
            }
        }
        // 上帧无灯：选择面积最大的信标
        else if (obj->selectedIndex == 100 || obj->sbea[obj->beaCount].beaArea > obj->sbea[obj->selectedIndex].beaArea) obj->selectedIndex = obj->beaCount;

        // 边界检查，防止数组越界
        if (obj->sbea[obj->beaCount].beaX < obj->data->width && obj->sbea[obj->beaCount].beaY < obj->data->height)
        {
            // 标记水平线
            for (uint32_t x = 0; x < obj->data->width; x++)
            {
                obj->show->data[obj->sbea[obj->beaCount].beaY * obj->data->width + x] = cReverse(obj->background);
            }

            // 标记垂直线
            for (uint32_t y = 0; y < obj->data->height; y++)
            {
                obj->show->data[y * obj->data->width + obj->sbea[obj->beaCount].beaX] = cReverse(obj->background);
            }
        }

        obj->beaCount++;
    }

    if (obj->beaCount)
    {
        obj->lastFrameBeaExist = 1;  // 标记当前帧有有效区域
        obj->lastFrameSelectedBeaXY[0] = obj->sbea[obj->selectedIndex].beaX;
        obj->lastFrameSelectedBeaXY[1] = obj->sbea[obj->selectedIndex].beaY;
    }
    else obj->lastFrameBeaExist = 0;
}

void twoPassEightConnectedAreaProcess(BEAINF* obj)
{
    // first pass
    uint16_t pixCount = obj->data->width * obj->data->height;
    if (pixCount > UNIFORMLY_PRE_ALLOCATED_BUFFER_SIZE) return;
    uint16_t lab = 0;

    // 清零全局数组
    memset(g_labMap, 0, pixCount * sizeof(uint16_t));
    memset(g_labelEquivalence, 0, 1024 * sizeof(uint16_t));

    for (uint16_t i = 0; i < pixCount; i++)
    {
        if (lab >= 1024) continue;

        if (obj->data->data[i] == obj->background)
        {
            g_labMap[i] = 0;  // 背景像素标记为0
            continue;
        }

        // 收集所有标签
        uint16_t labels[4];
        labels[0] = (i % obj->data->width == 0) ? 0 : g_labMap[i - 1];
        labels[1] = (i % obj->data->width == 0 || i < obj->data->width) ? 0 : g_labMap[i - obj->data->width - 1];
        labels[2] = (i < obj->data->width) ? 0 : g_labMap[i - obj->data->width];
        labels[3] = (i % obj->data->width == obj->data->width - 1 || i < obj->data->width) ? 0 : g_labMap[i - obj->data->width + 1];
        uint16_t validLabels[4] = {0};
        uint16_t validCount = 0;

        for (uint16_t k = 0; k < 4; k++)
            if (labels[k] != 0) validLabels[validCount++] = labels[k];

        if (!validCount)
        {
            g_labMap[i] = ++lab;  // 新标签
            g_labelEquivalence[lab] = lab;
        }
        else if (validCount == 1)
        {
            g_labMap[i] = validLabels[0];
        }
        else
        {
            // 找到最小标签
            uint16_t minLabel = validLabels[0];
            for (uint16_t k = 0; k < validCount; k++) minLabel = (validLabels[k] < minLabel) ? validLabels[k] : minLabel;

            // 使用最小标签
            g_labMap[i] = minLabel;

            // 将所有其他标签指向最小标签
            for (uint16_t k = 0; k < validCount; k++) g_labelEquivalence[validLabels[k]] = minLabel;
        }
    }

    // second pass
    // 清零全局临时BEA数组（复用同一个全局数组）
    memset(g_tempBea, 0, 100 * sizeof(SBEA));
    uint16_t tempBeaCount = 0;

    // 路径压缩优化标签等价表
    for (uint16_t i = 1; i <= lab; i++)
    {
        uint16_t root = i;
        while (root != g_labelEquivalence[root]) root = g_labelEquivalence[root];
        g_labelEquivalence[i] = root;
    }

    for (uint16_t i = 0; i < pixCount; i++)
    {
        uint16_t curr_label = g_labMap[i];

        if (!curr_label) continue;

        // 使用优化后的等价表直接查找
        curr_label = g_labelEquivalence[curr_label];

        if (curr_label > 100) continue;  // 防止数组越界

        tempBeaCount = (curr_label > tempBeaCount) ? curr_label : tempBeaCount;

        g_tempBea[curr_label - 1].beaX += (i % obj->data->width);
        g_tempBea[curr_label - 1].beaY += (i / obj->data->width);
        g_tempBea[curr_label - 1].beaArea++;
    }

    // filter pass
    memcpy(obj->show->data, obj->data->data, pixCount * sizeof(uint8_t));
    obj->beaCount = 0;          // 重置计数器
    obj->selectedIndex = 100;   // 100表示未选中任何区域
    float minDistance = -1.0f;  // 使用-1表示未初始化

    for (uint16_t i = 0; i < tempBeaCount; i++)
    {
        // 跳过空区域
        if (g_tempBea[i].beaArea == 0 || obj->beaCount > 99) continue;

        // 计算质心
        g_tempBea[i].beaX = (uint32_t) ((float) g_tempBea[i].beaX / (float) g_tempBea[i].beaArea);
        g_tempBea[i].beaY = (uint32_t) ((float) g_tempBea[i].beaY / (float) g_tempBea[i].beaArea);

        // 检查面积和位置约束
        if (g_tempBea[i].beaX < obj->xRange[0] || g_tempBea[i].beaX >= obj->xRange[1] ||
            g_tempBea[i].beaY < obj->yRange[0] || g_tempBea[i].beaY >= obj->yRange[1] ||
            g_tempBea[i].beaArea < obj->areaRange[0] || g_tempBea[i].beaArea > obj->areaRange[1])
            continue;

        // 将有效信标添加到结果数组
        obj->sbea[obj->beaCount].beaX = g_tempBea[i].beaX;
        obj->sbea[obj->beaCount].beaY = g_tempBea[i].beaY;
        obj->sbea[obj->beaCount].beaArea = g_tempBea[i].beaArea;

        // 选择逻辑：分两种情况处理
        if (obj->lastFrameBeaExist)
        {
            // 上帧有灯：选择距离上帧选中的灯距离最近的信标
            float distance = sqrtf(powf((float) obj->sbea[obj->beaCount].beaX - (float) obj->lastFrameSelectedBeaXY[0], 2) +
                                   powf((float) obj->sbea[obj->beaCount].beaY - (float) obj->lastFrameSelectedBeaXY[1], 2));

            if (minDistance < 0.0f || distance < minDistance)
            {
                obj->selectedIndex = obj->beaCount;
                minDistance = distance;
            }
        }
        // 上帧无灯：选择面积最大的信标
        else if (obj->selectedIndex == 100 || obj->sbea[obj->beaCount].beaArea > obj->sbea[obj->selectedIndex].beaArea) obj->selectedIndex = obj->beaCount;


        // 边界检查，防止数组越界
        if (obj->sbea[obj->beaCount].beaX < obj->data->width && obj->sbea[obj->beaCount].beaY < obj->data->height)
        {
            // 标记水平线
            for (uint16_t x = 0; x < obj->data->width; x++)
            {
                obj->show->data[obj->sbea[obj->beaCount].beaY * obj->data->width + x] = cReverse(obj->background);
            }

            // 标记垂直线
            for (uint16_t y = 0; y < obj->data->height; y++)
            {
                obj->show->data[y * obj->data->width + obj->sbea[obj->beaCount].beaX] = cReverse(obj->background);
            }
        }

        obj->beaCount++;
    }

    if (obj->beaCount)
    {
        obj->lastFrameBeaExist = 1;  // 标记当前帧有有效区域
        obj->lastFrameSelectedBeaXY[0] = obj->sbea[obj->selectedIndex].beaX;
        obj->lastFrameSelectedBeaXY[1] = obj->sbea[obj->selectedIndex].beaY;
    }
    else obj->lastFrameBeaExist = 0;
}
