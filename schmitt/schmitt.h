/*
 * schmitt.h
 *
 *  Created on: 20250718
 *      Author: Hecate
 */

#ifndef CODE_SCHMITT_H_
#define CODE_SCHMITT_H_

#include <stdint.h>

/**
 * @brief 施密特触发器数据结构
 *
 * 施密特触发器是一种具有迟滞特性的比较器，通过设置两个不同的
 * 阈值电平来实现抗噪声和防抖动功能。广泛应用于信号整形、
 * 噪声消除和数字化处理。
 *
 * @param upperThreshold    上阈值 - 输入从低到高转换的触发电平
 * @param lowerThreshold    下阈值 - 输入从高到低转换的触发电平
 * @param outputState       当前输出状态（0或1）
 * @param lastOutputState   上一次输出状态，用于检测状态变化
 * @param lastInputValue    上一次输入值，用于调试和分析
 *
 * @note 迟滞特性：
 *       - 迟滞宽度 = upperThreshold - lowerThreshold
 *       - 迟滞宽度越大，抗噪声能力越强
 *       - 迟滞宽度过大会降低响应灵敏度
 */
typedef struct
{
    float upperThreshold;
    float lowerThreshold;
    uint8_t outputState;
    uint8_t lastOutputState;
    float lastInputValue;
} SchmittTrigger;

/**
 * @brief 初始化施密特触发器
 *
 * 此函数初始化施密特触发器的阈值和状态。施密特触发器通过引入
 * 迟滞特性来消除输入信号中的噪声和抖动，提供稳定的开关输出。
 *
 * @param obj 指向施密特触发器对象的指针
 * @param upperThresh 上阈值 - 从低到高转换的触发电平
 * @param lowerThresh 下阈值 - 从高到低转换的触发电平
 * @param initState 初始输出状态（0或1）
 *
 * @details 阈值处理逻辑：
 *          - 当upperThresh < lowerThresh时，自动交换两个阈值
 *          - 当upperThresh = lowerThresh时，退化为普通比较器
 *          - 正常情况下，upperThresh > lowerThresh形成迟滞区间
 *
 * @note 迟滞区间 = upperThresh - lowerThresh
 *       迟滞区间越大，抗噪声能力越强，但响应速度越慢
 *
 * @warning 初始状态会影响首次触发条件，请根据实际需求设置
 */
void schmittInit(SchmittTrigger *obj, float upperThresh, float lowerThresh, uint8_t initState);

/**
 * @brief 处理输入信号并更新施密特触发器状态
 *
 * 此函数根据施密特触发器的迟滞特性处理输入信号，
 * 产生稳定的数字输出，有效消除信号抖动。
 *
 * @param obj 指向施密特触发器对象的指针
 * @param inputValue 当前输入信号值
 *
 * @details 状态转换逻辑：
 *          - 当前状态为0（低）：inputValue >= upperThreshold时转为1
 *          - 当前状态为1（高）：inputValue <= lowerThreshold时转为0
 *          - 在迟滞区间内（lowerThreshold < inputValue < upperThreshold）保持当前状态
 *
 * @note 施密特触发器的优点：
 *       1. 消除输入信号的噪声和抖动
 *       2. 提供明确的开关阈值
 *       3. 防止在阈值附近的频繁切换
 *
 * @note 输出状态存储在obj->outputState中，
 *       可通过比较obj->outputState和obj->lastOutputState检测状态变化
 */
void schmittProcess(SchmittTrigger *obj, float inputValue);

/**
 * @brief 动态设置施密特触发器的阈值
 *
 * 此函数允许在运行时修改施密特触发器的上下阈值，
 * 适用于需要根据系统状态动态调整触发条件的场景。
 *
 * @param obj 指向施密特触发器对象的指针
 * @param upperThresh 新的上阈值
 * @param lowerThresh 新的下阈值
 *
 * @details 阈值设置规则：
 *          - 自动处理阈值反向的情况（交换阈值）
 *          - 当两个阈值相等时，保持不变（退化为比较器）
 *          - 不影响当前输出状态，仅改变后续触发条件
 *
 * @warning 在系统运行中修改阈值可能导致意外的状态转换，
 *          建议在系统稳态时进行修改
 *
 * @note 应用场景示例：
 *       - 自适应阈值调整
 *       - 不同工作模式的切换
 *       - 环境参数补偿
 */
void schmittSetThresholds(SchmittTrigger *obj, float upperThresh, float lowerThresh);

#endif /* CODE_SCHMITT_H_ */
