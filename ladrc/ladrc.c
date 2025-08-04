/*
 * ladrc.c
 *
 *  Created on: 2025年6月5日
 *      Author: Hecate
 */
#include "ladrc.h"

#include <math.h>

/**
 * @brief 确定浮点数的符号。
 *
 * 此函数返回输入值的符号作为整数。
 *
 * @param x 要检查的浮点数。
 * @return int 如果x为正返回1，如果x为负返回-1，如果x为零返回0。
 */
static inline int sign(float x)
{
    return (x > 0) ? 1 : (x < 0) ? -1
                                 : 0;
}

/**
 * @brief 快速最优控制综合函数 (fhan)
 *
 * 实现跟踪微分器中使用的快速最优控制综合算法。
 * 此函数根据当前状态和参数生成最优控制信号。
 *
 * @param x1 当前位置/状态值
 * @param x2 当前速度/状态的导数
 * @param r 速度因子 - 决定跟踪速度（r越大跟踪越快）
 * @param h 采样时间步长
 *
 * @return 将系统驱动到期望状态的最优控制信号
 *
 * @note 这是ADRC/LADRC控制系统中TD（跟踪微分器）的核心组件
 */
static float fhan(float x1, float x2, float r, float h)
{
    float d = r * h * h;
    float a0 = h * x2;
    float y = x1 + a0;
    float a1 = sqrtf(d * (d + 8.0f * fabs(y)));
    float a2 = a0 + sign(y) * (a1 - d) * 0.5;
    float a = (a0 + y) * (sign(y + d) - sign(y - d)) * 0.5f + a2 * (1 - (sign(y + d) - sign(y - d)) * 0.5f);

    return -r * (a / d) * (sign(a + d) - sign(a - d)) * 0.5f - r * sign(a) * (1.0f - (sign(a + d) - sign(a - d)) * 0.5f);
}

/**
 * @brief 原点附近具有线性段的非线性函数
 *
 * 实现自抗扰控制（ADRC）算法中常用的fal（具有线性段的函数）非线性函数。
 * 该函数在线性和非线性区域之间提供平滑过渡。
 *
 * @param e 输入误差信号
 * @param alfa 幂指数（典型使用时 0 < alfa < 1）
 * @param delta 线性区域阈值，决定线性段的宽度
 *
 * @return 非线性函数输出：
 *         - |e|^alfa * sign(e) 当 |e| > delta （非线性区域）
 *         - e / delta^(1-alfa) 当 |e| <= delta （线性区域）
 */
static float fal(float e, float alfa, float delta)
{
    if (fabs(e) > delta)
        return powf(fabs(e), alfa) * sign(e);
    else
        return e / powf(delta, 1.0f - alfa);
}

/**
 * @brief 初始化跟踪微分器对象
 *
 * 此函数使用指定参数初始化跟踪微分器（TD）。
 * 跟踪微分器用于从噪声测量中提取微分信号，
 * 同时提供滤波功能。
 *
 * @param obj 指向要初始化的TD结构的指针
 * @param r 速度因子，决定跟踪速度（r越大跟踪越快）
 * @param h 采样时间步长或积分步长
 * @param n 滤波器系数，影响滤波特性（通常n*h决定滤波器时间常数）
 *
 * @note 该函数将内部状态x1和x2设置为零，并标记TD为首次初始化
 */
void trackingDifferentiatorInit(TrackingDifferentiator *obj, float r, float h, float n)
{
    obj->firstFlag = 1;

    obj->r = r;
    obj->h = h;

    obj->h0 = n * h;

    obj->x1 = 0;
    obj->x2 = 0;
}

/**
 * @brief 用于信号跟踪和微分的跟踪微分器函数
 *
 * 此函数实现跟踪微分器算法，跟踪输入信号
 * 同时提供其导数。它使用fhan（最速跟踪微分器）函数
 * 实现平滑跟踪并抑制噪声。
 *
 * @param obj 指向TD（跟踪微分器）结构的指针，包含：
 *            - x1: 跟踪信号输出
 *            - x2: 跟踪信号的导数
 *            - h: 积分步长
 *            - r: 速度因子（r越大跟踪越快）
 *            - h0: fhan函数的滤波因子
 *            - firstFlag: 初始化标志
 * @param v   要跟踪的输入信号值
 *
 * @note 首次调用时（当firstFlag被设置时），函数用输入值初始化x1
 * @note 函数就地更新obj->x1（跟踪信号）和obj->x2（导数）
 */
void trackingDifferentiator(TrackingDifferentiator *obj, float v)
{
    if (obj->firstFlag)
    {
        obj->x1 = v;
        obj->firstFlag = 0;
    }

    obj->x1 = obj->x1 + obj->h * obj->x2;
    obj->x2 = obj->x2 + obj->h * fhan(obj->x1 - v, obj->x2, obj->r, obj->h0);
}

/**
 * @brief 初始化LADRC（线性自抗扰控制）控制器
 *
 * @param obj 指向要初始化的LADRC对象的指针
 * @param target 控制系统的目标设定值
 * @param r 跟踪微分器的速度因子
 * @param h 跟踪微分器的采样时间
 * @param n 跟踪微分器的滤波因子
 * @param omega 扩张状态观测器（ESO）的带宽
 * @param delta 非线性函数的过渡区间
 * @param b 控制增益参数
 * @param kp 线性状态误差反馈的比例增益
 * @param kd 线性状态误差反馈的微分增益
 *
 * @note ESO增益（beta1、beta2、beta3）根据带宽参数omega自动计算
 */
void ladrcInit(LADRC *obj, float target, float r, float h, float n, float omega, float delta, float b, float kp, float kd)
{
    obj->target = target;

    trackingDifferentiatorInit(&obj->td, r, h, n);

    obj->esoBeta1 = 3.0f * omega;
    obj->esoBeta2 = 3.0f * omega * omega;
    obj->esoBeta3 = omega * omega * omega;

    obj->delta = delta;

    obj->esoZ1 = 0;
    obj->esoZ2 = 0;
    obj->esoZ3 = 0;

    obj->b = b;
    obj->lsefKp = kp;
    obj->lsefKd = kd;
    obj->controlValue = 0;
}

/**
 * @brief 设置LADRC控制器的目标参考值
 *
 * @param obj 指向LADRC控制器对象的指针
 * @param target 控制器要跟踪的期望目标/参考值
 */
void ladrcSetTarget(LADRC *obj, float target)
{
    obj->target = target;
}

/**
 * @brief 处理线性自抗扰控制（LADRC）算法
 *
 * 此函数实现LADRC控制算法，包含三个主要组件：
 * - 跟踪微分器（TD）：生成平滑的跟踪信号及其导数
 * - 扩张状态观测器（ESO）：估计系统状态和总扰动
 * - 线性状态误差反馈（LSEF）：基于误差反馈计算控制输出
 *
 * @param obj 指向包含所有控制器参数和状态的LADRC结构的指针
 * @param y 当前系统输出/测量值
 *
 * @note 首次执行时（当td.firstFlag被设置时），函数用当前测量值
 *       初始化TD和ESO状态，以防止瞬态尖峰
 *
 * @details 算法流程：
 *          1. 首次运行时初始化状态
 *          2. 更新跟踪微分器以获得参考信号和导数
 *          3. 更新ESO以估计系统状态和扰动
 *          4. 使用带扰动补偿的LSEF律计算控制值
 */
void ladrcProcess(LADRC *obj, float y)
{
    if (obj->td.firstFlag)
    {
        obj->td.x1 = y;
        obj->esoZ1 = y;
        obj->td.firstFlag = 0;
    }

    // td
    trackingDifferentiator(&obj->td, obj->target);

    // eso
    float e = obj->esoZ1 - y;

    obj->esoZ1 = obj->esoZ1 + obj->td.h * (obj->esoZ2 - obj->esoBeta1 * e);
    obj->esoZ2 = obj->esoZ2 + obj->td.h * (obj->esoZ3 - obj->esoBeta2 * fal(e, 0.5f, obj->delta) + obj->b * obj->controlValue);
    obj->esoZ3 = obj->esoZ3 - obj->td.h * obj->esoBeta3 * fal(e, 0.25f, obj->delta);

    // lsef
    float e1 = obj->td.x1 - y;

    float e2 = obj->td.x2 - obj->esoZ2;

    obj->controlValue = obj->lsefKp * e1 + obj->lsefKd * e2 - obj->esoZ3 / obj->b;
}
