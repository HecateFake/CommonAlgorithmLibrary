/*
 * ladrc.h
 *
 *  Created on: 2025年6月5日
 *      Author: Hecate
 */

#ifndef CODE_LADRC_H_
#define CODE_LADRC_H_

/**
 * @brief 跟踪微分器（TD）结构体，用于信号跟踪和滤波
 *
 * TD用于跟踪输入信号，同时提供平滑过渡和滤波功能。
 * 它从有噪声或不连续的输入信号中生成平滑的跟踪信号及其导数。
 *
 * @param r 时间尺度因子 - 控制跟踪速度（值越大跟踪越快）
 * @param h 积分时间步长（秒） - 离散实现的采样周期
 * @param h0 滤波因子 - 用于跟踪过程中的噪声抑制
 * @param x1 跟踪状态变量 - 表示滤波/跟踪后的信号
 * @param x2 跟踪状态导数 - 表示跟踪信号的导数
 * @param firstFlag 初始化标志 - 指示TD是否已初始化
 */
typedef struct
{
    float r;
    float h;
    float h0;
    float x1;
    float x2;
    float firstFlag;
} TrackingDifferentiator;

/**
 * @brief 线性自抗扰控制（LADRC）结构体
 *
 * 该结构体包含实现线性自抗扰控制器所需的所有参数和状态变量。
 *
 * @param target 控制系统的期望设定值或参考值
 * @param td 跟踪微分器（TD）结构体，用于平滑信号跟踪
 * @param esoBeta1 扩张状态观测器（ESO）增益参数1
 * @param esoBeta2 扩张状态观测器（ESO）增益参数2
 * @param esoBeta3 扩张状态观测器（ESO）增益参数3
 * @param esoZ1 ESO状态变量1 - 估计的系统输出
 * @param esoZ2 ESO状态变量2 - 估计的系统输出导数
 * @param esoZ3 ESO状态变量3 - 估计的总扰动
 * @param delta 控制律的过渡区间参数
 * @param b 系统的控制增益/补偿因子
 * @param lsefKp 线性状态误差反馈比例增益
 * @param lsefKd 线性状态误差反馈微分增益
 * @param controlValue 最终控制输出信号，将应用于系统
 */
typedef struct
{
    float target;
    TrackingDifferentiator td;
    float esoBeta1;
    float esoBeta2;
    float esoBeta3;
    float esoZ1;
    float esoZ2;
    float esoZ3;
    float delta;
    float b;
    float lsefKp;
    float lsefKd;
    float controlValue;
} LADRC;

// 函数声明

/**
 * @brief 初始化跟踪微分器结构体
 *
 * @param obj 指向要初始化的TD结构体的指针
 * @param r 跟踪速度参数（值越大表示跟踪越快）
 * @param h 积分步长（采样周期）
 * @param n 滤波系数（噪声抑制因子）
 */
void trackingDifferentiatorInit(TrackingDifferentiator *obj, float r, float h, float n);

/**
 * @brief 执行跟踪微分器算法
 *
 * @param obj 指向包含微分器参数和状态的TD（跟踪微分器）结构体的指针
 * @param v 要跟踪和微分的输入信号值
 *
 * @details 此函数实现了跟踪微分器，它从输入信号中提供平滑的跟踪信号
 *          及其导数估计。它有助于滤除噪声，同时保持良好的动态响应特性。
 */
void trackingDifferentiator(TrackingDifferentiator *obj, float v);

/**
 * @brief 初始化线性自抗扰控制（LADRC）控制器
 *
 * @param obj 指向要初始化的LADRC结构体的指针
 * @param target 控制系统的目标参考值
 * @param r 期望闭环极点的半径
 * @param h 采样周期（时间步长）
 * @param n 系统的阶数
 * @param omega 观测器的自然频率
 * @param delta 观测器的阻尼系数
 * @param b 控制增益系数
 * @param kp 反馈控制器的比例增益
 * @param kd 反馈控制器的微分增益
 */
void ladrcInit(LADRC *obj, float target, float r, float h, float n, float omega, float delta, float b, float kp, float kd);

/**
 * @brief 设置LADRC控制器的目标参考值
 *
 * @param obj 指向LADRC控制器对象的指针
 * @param target 控制系统的期望目标/参考值
 */
void ladrcSetTarget(LADRC *obj, float target);

/**
 * @brief 处理LADRC（线性自抗扰控制）算法
 *
 * @param obj 指向包含控制器参数和状态的LADRC结构体的指针
 * @param y 当前系统输出测量值
 *
 * @details 此函数执行LADRC控制算法的一次迭代，
 *          根据当前测量值更新内部状态并计算控制输出。
 */
void ladrcProcess(LADRC *obj, float y);

#endif /* CODE_LADRC_H_ */
