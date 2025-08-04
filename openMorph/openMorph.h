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
    0, 1, 0};

// 3x3 正方形结构元素
static const binary RECT_3X3[] = {
    1, 1, 1,
    1, 1, 1,
    1, 1, 1};

// 5x5 圆形结构元素
static const binary CIRCLE_5X5[] = {
    0, 0, 1, 0, 0,
    0, 1, 1, 1, 0,
    1, 1, 1, 1, 1,
    0, 1, 1, 1, 0,
    0, 0, 1, 0, 0};

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
void morphErode(binary* src, uint16_t width, uint16_t height, binary* kernel, uint16_t kWidth, uint16_t kHeight, binary* dst, binary background);

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
void morphDilate(binary* src, uint16_t width, uint16_t height, binary* kernel, uint16_t kWidth, uint16_t kHeight, binary* dst, binary background);

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
void morphOpen(binary* src, uint16_t width, uint16_t height, binary* kernel, uint16_t kWidth, uint16_t kHeight, binary* dst, binary background);

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
void morphClose(binary* src, uint16_t width, uint16_t height, binary* kernel, uint16_t kWidth, uint16_t kHeight, binary* dst, binary background);

/**
 * @brief 将二值图像转换为8位灰度图像
 *
 * 将 binary 类型的二值图像数组转换为 uint8_t 类型的8位灰度图像。
 * 前景像素（值为1）转换为255（白色），背景像素（值为0）转换为0（黑色）。
 *
 * @param src 源二值图像数组
 * @param width 图像宽度（像素）
 * @param height 图像高度（像素）
 * @param dst 目标8位灰度图像数组，需要预先分配 width*height 大小的内存
 *
 * @note 转换规则：
 *       - binary值为1（前景） → uint8_t值为255（白色）
 *       - binary值为0（背景） → uint8_t值为0（黑色）
 *
 * @warning 确保目标数组 dst 已分配足够的内存空间
 *
 * @example
 *   binary binImg[64*64];
 *   uint8_t grayImg[64*64];
 *   toUint8(binImg, 64, 64, grayImg);
 */
void toUint8(binary* src, uint16_t width, uint16_t height, uint8_t* dst);

/**
 * @brief 将8位灰度图像转换为二值图像
 *
 * 将 uint8_t 类型的8位灰度图像数组转换为 binary 类型的二值图像。
 * 任何非零像素值都被转换为前景（1），零像素值转换为背景（0）。
 *
 * @param src 源8位灰度图像数组
 * @param width 图像宽度（像素）
 * @param height 图像高度（像素）
 * @param dst 目标二值图像数组，需要预先分配 width*height 大小的内存
 *
 * @note 转换规则：
 *       - uint8_t值 > 0（任何非零值） → binary值为1（前景）
 *       - uint8_t值 = 0（零值） → binary值为0（背景）
 *
 * @warning 确保目标数组 dst 已分配足够的内存空间
 *
 * @example
 *   uint8_t grayImg[64*64];
 *   binary binImg[64*64];
 *   toBinary(grayImg, 64, 64, binImg);
 */
void toBinary(uint8_t* src, uint16_t width, uint16_t height, binary* dst);

#endif  // OPENMORPH_H