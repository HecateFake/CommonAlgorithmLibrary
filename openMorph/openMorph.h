/*
 * openMorph.h
 *
 *  Created on: 2025年8月4日
 *      Author: Hecate
 */

#ifndef OPENMORPH_H
#define OPENMORPH_H

#include <stdint.h>
#include <stdbool.h>

// 颜色宏定义
#define BLACK 0
#define WHITE 1

// 二值化图像数组类型定义
typedef bool binary;

/**
 * @brief 常用结构元素定义
 *
 * 提供一些常用的形态学结构元素，可直接使用或作为参考。
 */

// 3x3 十字形结构元素
static const binary CROSS_3X3[] = {
    0, 1, 0,
    1, 1, 1,
    0, 1, 0
};

// 3x3 正方形结构元素  
static const binary RECT_3X3[] = {
    1, 1, 1,
    1, 1, 1,
    1, 1, 1
};

// 5x5 圆形结构元素
static const binary CIRCLE_5X5[] = {
    0, 0, 1, 0, 0,
    0, 1, 1, 1, 0,
    1, 1, 1, 1, 1,
    0, 1, 1, 1, 0,
    0, 0, 1, 0, 0
};

// 结构元素尺寸宏定义
#define CROSS_3X3_SIZE 3
#define RECT_3X3_SIZE 3  
#define CIRCLE_5X5_SIZE 5

// 形态学运算函数声明

/**
 * @brief 图像腐蚀运算
 *
 * 腐蚀运算通过结构元素对二值图像进行腐蚀，用于去除小的噪声点，
 * 收缩前景对象，分离连接的对象。
 *
 * @param src 源二值图像数组
 * @param width 图像宽度
 * @param height 图像高度  
 * @param kernel 结构元素（核）数组
 * @param kWidth 结构元素宽度
 * @param kHeight 结构元素高度
 * @param dst 目标图像数组（存储结果）
 * @param background 背景色值（0或1）
 *
 * @note 腐蚀算法：只有当结构元素覆盖的所有像素都为前景色时，
 *       中心像素才设为前景色，否则设为背景色
 * @warning 源图像和目标图像不能是同一块内存区域
 */
void morphErode(binary* src, uint8_t width, uint8_t height, binary* kernel, uint8_t kWidth, uint8_t kHeight, binary* dst, binary background);

/**
 * @brief 图像膨胀运算
 *
 * 膨胀运算通过结构元素对二值图像进行膨胀，用于填充小的空洞，
 * 扩张前景对象，连接断开的对象。
 *
 * @param src 源二值图像数组
 * @param width 图像宽度
 * @param height 图像高度
 * @param kernel 结构元素（核）数组
 * @param kWidth 结构元素宽度
 * @param kHeight 结构元素高度
 * @param dst 目标图像数组（存储结果）
 * @param background 背景色值（0或1）
 *
 * @note 膨胀算法：只要结构元素覆盖的任意一个像素为前景色，
 *       中心像素就设为前景色
 * @warning 源图像和目标图像不能是同一块内存区域
 */
void morphDilate(binary* src, uint8_t width, uint8_t height, binary* kernel, uint8_t kWidth, uint8_t kHeight, binary* dst, binary background);

/**
 * @brief 开运算 (先腐蚀后膨胀)
 *
 * 开运算可以去除小的噪声点，平滑对象边界，分离细小的连接。
 * 开运算倾向于保持对象的面积和形状，同时去除细节。
 *
 * @param src 源二值图像数组
 * @param width 图像宽度
 * @param height 图像高度
 * @param kernel 结构元素（核）数组
 * @param kWidth 结构元素宽度
 * @param kHeight 结构元素高度
 * @param dst 目标图像数组（存储结果）
 * @param background 背景色值（0或1）
 *
 * @note 开运算 = 腐蚀 + 膨胀，主要用于去噪和分离对象
 */
void morphOpen(binary* src, uint8_t width, uint8_t height, binary* kernel, uint8_t kWidth, uint8_t kHeight, binary* dst, binary background);

/**
 * @brief 闭运算 (先膨胀后腐蚀)
 *
 * 闭运算可以填充对象内部的小洞，平滑对象边界，连接邻近的对象。
 * 闭运算倾向于保持对象的面积和形状，同时填充内部空洞。
 *
 * @param src 源二值图像数组
 * @param width 图像宽度
 * @param height 图像高度
 * @param kernel 结构元素（核）数组
 * @param kWidth 结构元素宽度
 * @param kHeight 结构元素高度
 * @param dst 目标图像数组（存储结果）
 * @param background 背景色值（0或1）
 *
 * @note 闭运算 = 膨胀 + 腐蚀，主要用于填洞和连接对象
 */
void morphClose(binary* src, uint8_t width, uint8_t height, binary* kernel, uint8_t kWidth, uint8_t kHeight, binary* dst, binary background);

#endif  // OPENMORPH_H