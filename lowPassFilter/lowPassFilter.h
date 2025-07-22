/*
 * lowPassFilter.h
 *
 *  创建时间: 2025年3月19日
 *      作者: Hecate
 */

#ifndef CODE_LOWPASSFILTER_H_
#define CODE_LOWPASSFILTER_H_

#include <stdint.h>

/**
 * @brief 低通滤波器数据结构
 *
 * 该结构包含实现带变化率限制的低通滤波器所需的
 * 所有参数和状态变量。
 *
 * @param alpha         滤波系数 (0.0 到 1.0) - 决定截止频率
 * @param firstFlag     初始化标志 - 指示是否为首次滤波迭代
 * @param lastOutput    上一次输出值 - 用于滤波器状态保持
 * @param filteredValue 当前滤波值
 * @param changeRateMax 最大允许变化率 - 用于变化率限制
 */
typedef struct
{
        float alpha;
        uint8_t firstFlag;
        float lastOutput;
        float filteredValue;
        float changeRateMax;
} LowPassFilterData;

/**
 * @brief 用指定的平滑因子初始化低通滤波器
 *
 * 该函数用给定的alpha系数初始化低通滤波器数据结构。
 * alpha值决定滤波器的截止频率和平滑特性。
 *
 * @param obj 指向待初始化的低通滤波器数据结构的指针
 * @param alpha 滤波器的平滑因子 (0.0 < alpha <= 1.0)
 *              较低值提供更多平滑但响应较慢
 *              较高值提供较少平滑但响应较快
 *
 * @note alpha参数应基于期望的截止频率和采样率计算：
 *       alpha = dt / (RC + dt)，其中
 *       RC = 1 / (2 * PI * 截止频率) 且 dt = 采样周期
 */
void lowPassFilterInit(LowPassFilterData *obj, float alpha);

/**
 * @brief 对输入值应用低通滤波
 *
 * @param obj 指向包含滤波器参数和状态的低通滤波器数据结构的指针
 * @param value 待滤波的输入信号值
 *
 * @details 该函数通过低通滤波算法处理输入值，
 *          更新滤波器的内部状态并产生平滑的输出信号。
 *          滤波特性由LowPassFilterData结构中存储的参数决定。
 */
void lowPassFilter(LowPassFilterData *obj, float value);

#endif /* CODE_LOWPASSFILTER_H_ */
