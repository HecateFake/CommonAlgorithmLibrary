/*
 * lowPassFilter.c
 *
 *  Created on: 2025年3月19日
 *      Author: Hecate
 */
#include "lowPassFilter.h"

#include <math.h>

/**
 * @brief 初始化低通滤波器数据结构
 *
 * @param obj 指向要初始化的低通滤波器数据结构的指针
 * @param alpha 滤波器系数 (0.0 到 1.0)，决定截止频率。
 *              较小的值产生更多滤波（响应较慢）。
 *              较大的值产生较少滤波（响应较快）。
 *
 * @details 此函数通过以下方式设置低通滤波器的初始状态：
 *          - 设置滤波器系数 (alpha)
 *          - 清除滤波值
 *          - 设置首次采样标志
 *          - 清除上次输出值
 */
void lowPassFilterInit(LowPassFilterData *obj, float alpha)
{
    obj->alpha = alpha;

    obj->filteredValue = 0;
    obj->firstFlag = 1;
    obj->lastOutput = 0;
}

/**
 * @brief 对输入值应用低通滤波
 *
 * 此函数使用指数移动平均公式实现一阶无限脉冲响应（IIR）低通滤波器。
 * 在首次调用时，它使用输入值初始化滤波器状态。
 *
 * @param obj 指向包含滤波器状态和参数的低通滤波器数据结构的指针
 * @param value 要被滤波的输入值
 *
 * @note 滤波后的输出存储在 obj->filteredValue 中
 * @note obj 结构中的 alpha 参数决定滤波器截止频率
 *       (0 < alpha < 1，其中较小的值产生更多滤波)
 */
void lowPassFilter(LowPassFilterData *obj, float value)
{
    if (obj->firstFlag == 1)
    {
        obj->firstFlag = 0;
        obj->lastOutput = value;
    }

    obj->filteredValue = obj->alpha * value + (1.0f - obj->alpha) * obj->lastOutput;
    obj->lastOutput = obj->filteredValue;
}