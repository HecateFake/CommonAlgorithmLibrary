/*
 * pid.h
 *
 *  Created on: 2024年12月9日
 *      Author: Hecate
 */

#ifndef CODE_PID_H_
#define CODE_PID_H_

/**
 * @brief PID控制器模式枚举
 *
 * 定义PID控制器的工作模式，支持位置式和增量式两种模式。
 */
typedef enum
{
    positionMode,  /**< 位置式PID模式 - 直接输出控制量 */
    incrementMode, /**< 增量式PID模式 - 输出控制增量 */
} PidMode;

/**
 * @brief PID控制器数据结构
 *
 * 包含PID控制器运行所需的所有参数和状态变量。
 * 支持位置式和增量式两种控制模式。
 *
 * @param pidMode          PID控制模式（位置式或增量式）
 * @param expectedValue    期望值/目标值/设定值
 * @param error            当前误差 (e[k] = expectedValue - currentValue)
 * @param lastError        上一次误差 (e[k-1])
 * @param beforeLastError  上上次误差 (e[k-2])，仅用于增量式PID
 * @param integral         积分累积值，仅用于位置式PID
 * @param kp               比例增益系数
 * @param ki               积分增益系数
 * @param kd               微分增益系数
 * @param controlValue     控制输出值
 * @param integralMax      积分限幅值，防止积分饱和（仅位置式PID使用）
 */
typedef struct
{
    PidMode pidMode;

    float expectedValue;
    float error;
    float lastError;
    float beforeLastError;
    float integral;
    float kp, ki, kd;
    float controlValue;

    float integralMax;
} PID;

// 函数声明

/**
 * @brief 初始化PID控制器
 *
 * 此函数初始化PID控制器的所有参数和状态变量。
 * 根据积分限幅参数自动选择PID模式。
 *
 * @param integralMax 积分限幅值：
 *                    - 当integralMax = 0时，选择增量式PID（无积分限幅需求）
 *                    - 当integralMax > 0时，选择位置式PID（需要积分限幅）
 * @param obj 指向要初始化的PID控制器对象的指针
 * @param target 控制目标值/设定值
 * @param kp 比例增益系数
 *           - 决定系统的响应速度
 *           - 过大会导致超调和振荡
 *           - 过小会导致响应缓慢
 * @param ki 积分增益系数
 *           - 消除稳态误差
 *           - 过大会导致积分饱和和振荡
 *           - 过小会导致稳态误差消除缓慢
 * @param kd 微分增益系数
 *           - 预测误差变化趋势，提前纠正
 *           - 过大会放大噪声
 *           - 合适的值可以减少超调，提高稳定性
 *
 * @note 参数调节建议：
 *       1. 先调kp，使系统有合适的响应速度
 *       2. 再调ki，消除稳态误差
 *       3. 最后调kd，改善动态性能
 */
void pidInit(float integralMax, PID *obj, float target, float kp, float ki, float kd);

/**
 * @brief 设置PID控制器的目标值
 *
 * 此函数用于在运行时更新PID控制器的目标设定值，
 * 常用于目标值动态变化的控制场景。
 *
 * @param obj 指向PID控制器对象的指针
 * @param targetValue 新的目标设定值
 *
 * @note 目标值变化时，系统会产生新的误差，
 *       PID控制器将自动调整输出以跟踪新目标
 */
void pidSetTarget(PID *obj, float targetValue);

/**
 * @brief PID控制器主处理函数
 *
 * 此函数根据PID控制器的模式选择相应的算法进行处理。
 * 支持位置式和增量式两种PID算法。
 *
 * @param obj 指向PID控制器对象的指针
 * @param currentValue 当前系统输出值/反馈值
 *
 * @details 根据obj->pidMode的值选择处理方式：
 *          - position: 使用位置式PID算法
 *          - increment: 使用增量式PID算法
 *
 * @note 调用此函数后，控制输出值存储在obj->controlValue中
 */
void pidProcess(PID *obj, float currentValue);

#endif /* CODE_PID_H_ */
