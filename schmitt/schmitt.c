/*
 * schmitt.c
 *
 *  Created on: 20250718
 *  Author: Hecate
 */
#include "schmitt.h"

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
 */
void schmittInit(SchmittTrigger *obj, float upperThresh, float lowerThresh, uint8_t initState)
{
    // 确保初始状态为0或1
    if (initState > 1)
    {
        initState = 0;
    }

    // 处理阈值逻辑
    if (upperThresh < lowerThresh)
    {
        // 下阈值大于上阈值时，交换二者
        obj->upperThreshold = lowerThresh;
        obj->lowerThreshold = upperThresh;
    }
    else if (upperThresh == lowerThresh)
    {
        // 相等时不做处理，保持原值
        obj->upperThreshold = upperThresh;
        obj->lowerThreshold = lowerThresh;
    }
    else
    {
        // 正常情况
        obj->upperThreshold = upperThresh;
        obj->lowerThreshold = lowerThresh;
    }

    // 设置初始状态
    obj->outputState = initState;
    obj->lastOutputState = initState;  // 上一次状态等于初始状态
    obj->lastInputValue = 0.0f;        // 初始输入值
}

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
 */
void schmittProcess(SchmittTrigger *obj, float inputValue)
{
    // 保存上一次输出状态
    obj->lastOutputState = obj->outputState;

    // 保存输入值
    obj->lastInputValue = inputValue;

    // 根据当前状态和输入值决定输出
    if (obj->outputState == 0)
    {
        // 当前为低状态，检查是否超过上阈值
        if (inputValue >= obj->upperThreshold)
        {
            obj->outputState = 1;
        }
    }
    else
    {
        // 当前为高状态，检查是否低于下阈值
        if (inputValue <= obj->lowerThreshold)
        {
            obj->outputState = 0;
        }
    }
}

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
 */
void schmittSetThresholds(SchmittTrigger *obj, float upperThresh, float lowerThresh)
{
    if (upperThresh < lowerThresh)
    {
        // 交换阈值
        obj->upperThreshold = lowerThresh;
        obj->lowerThreshold = upperThresh;
    }
    else if (upperThresh != lowerThresh)
    {
        // 正常设置
        obj->upperThreshold = upperThresh;
        obj->lowerThreshold = lowerThresh;
    }
    // 相等时不处理
}
