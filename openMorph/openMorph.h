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

//=================================================常量定义=================================================

/**
 * @brief 统一预分配缓冲区大小
 *
 * 用于连通区域检测算法的缓冲区大小，基于MT9V03X摄像头的图像尺寸计算。
 *
 * @note 大小计算：188 × 120 = 22560 像素
 * @note 这个值确保能够处理MT9V03X摄像头的完整图像
 * @note 超过此大小的图像将被函数拒绝处理以防止缓冲区溢出
 */
#define UNIFORMLY_PRE_ALLOCATED_BUFFER_SIZE 22560

//=================================================颜色宏定义=================================================

/**
 * @brief 黑色像素值
 *
 * 定义8位灰度图像中的黑色像素值，通常用作背景色。
 */
#define BLACK 0

/**
 * @brief 白色像素值
 *
 * 定义8位灰度图像中的白色像素值，通常用作前景色。
 */
#define WHITE 255

/**
 * @brief 颜色反转宏
 *
 * 将输入颜色值反转，黑色变白色，白色变黑色。
 *
 * @param x 输入颜色值（BLACK或WHITE）
 * @return 反转后的颜色值
 *
 * @note 非零值被视为WHITE，返回BLACK
 * @note 零值被视为BLACK，返回WHITE
 */
#define cReverse(x) ((x) ? BLACK : WHITE)

//=================================================图像结构体定义=================================================

/**
 * @brief 8位图像结构体
 *
 * 用于存储8位灰度或二值图像的数据结构，包含图像数据指针和尺寸信息。
 *
 * @note 主要用于形态学运算和连通区域检测
 * @note data指针必须指向有效的内存区域，大小至少为 width × height 字节
 */
typedef struct
{
    uint8_t* data;    ///< 图像数据指针，按行优先顺序存储
    uint16_t width;   ///< 图像宽度（像素数量）
    uint16_t height;  ///< 图像高度（像素数量）
} CroppedImage8;

/**
 * @brief 16位图像结构体
 *
 * 用于存储16位图像数据的结构体，主要用于RGB565格式的彩色图像处理。
 *
 * @note 每个像素占用2字节，通常用于RGB565格式
 * @note data指针必须指向有效的内存区域，大小至少为 width × height × 2 字节
 */
typedef struct
{
    uint16_t* data;   ///< 图像数据指针，按行优先顺序存储
    uint16_t width;   ///< 图像宽度（像素数量）
    uint16_t height;  ///< 图像高度（像素数量）
} CroppedImage16;

//=================================================形态学运算函数声明=================================================

/**
 * @brief 图像腐蚀运算
 *
 * 腐蚀运算通过结构元素对8位灰度图像进行腐蚀，用于去除小的噪声点，
 * 收缩前景对象，分离连接的对象。
 *
 * @param src 源8位灰度图像结构体指针
 * @param kernel 结构元素（核）结构体指针
 * @param dst 目标图像结构体指针（存储结果）
 * @param background 背景色值（BLACK/WHITE）
 *
 * @note 腐蚀算法：只有当结构元素覆盖的所有像素都为前景色时，
 *       中心像素才设为前景色，否则设为背景色
 * @warning 源图像和目标图像不能是同一块内存区域
 */
void morphErode(CroppedImage8* src, CroppedImage8* kernel, CroppedImage8* dst, uint8_t background);

/**
 * @brief 图像膨胀运算
 *
 * 膨胀运算通过结构元素对8位灰度图像进行膨胀，用于填充小的空洞，
 * 扩张前景对象，连接断开的对象。
 *
 * @param src 源8位灰度图像结构体指针
 * @param kernel 结构元素（核）结构体指针
 * @param dst 目标图像结构体指针（存储结果）
 * @param background 背景色值（BLACK/WHITE）
 *
 * @note 膨胀算法：只要结构元素覆盖的任意一个像素为前景色，
 *       中心像素就设为前景色
 * @warning 确保 dst 结构体中的 data 指针已分配足够内存
 */
void morphDilate(CroppedImage8* src, CroppedImage8* kernel, CroppedImage8* dst, uint8_t background);

/**
 * @brief 开运算 (先腐蚀后膨胀)
 *
 * 开运算可以去除小的噪声点，平滑对象边界，分离细小的连接。
 * 开运算倾向于保持对象的面积和形状，同时去除细节。
 *
 * @param src 源8位灰度图像结构体指针
 * @param kernel 结构元素（核）结构体指针
 * @param dst 目标图像结构体指针（存储结果）
 * @param background 背景色值（BLACK/WHITE）
 *
 * @note 开运算 = 腐蚀 + 膨胀，主要用于去噪和分离对象
 * @warning 函数内部会分配临时内存，如果内存不足会直接返回
 */
void morphOpen(CroppedImage8* src, CroppedImage8* kernel, CroppedImage8* dst, uint8_t background);

/**
 * @brief 闭运算 (先膨胀后腐蚀)
 *
 * 闭运算可以填充对象内部的小洞，平滑对象边界，连接邻近的对象。
 * 闭运算倾向于保持对象的面积和形状，同时填充内部空洞。
 *
 * @param src 源8位灰度图像结构体指针
 * @param kernel 结构元素（核）结构体指针
 * @param dst 目标图像结构体指针（存储结果）
 * @param background 背景色值（BLACK/WHITE）
 *
 * @note 闭运算 = 膨胀 + 腐蚀，主要用于填洞和连接对象
 * @warning 函数内部会分配临时内存，如果内存不足会直接返回
 */
void morphClose(CroppedImage8* src, CroppedImage8* kernel, CroppedImage8* dst, uint8_t background);

/**
 * @brief 将8位灰度图像转换为二值图像
 *
 * 使用阈值将8位灰度图像转换为二值图像。
 * 大于阈值的像素转换为白色(255)，其他转换为黑色(0)。
 *
 * @param src 源8位灰度图像结构体指针
 * @param dst 目标8位二值图像结构体指针
 * @param threshold 二值化阈值 (0-255)
 *
 * @note 转换规则：
 *       - uint8_t值 > threshold → 255（白色）
 *       - uint8_t值 <= threshold → 0（黑色）
 * @note 目标结构体的 width 和 height 会被自动设置为源图像的尺寸
 *
 * @warning 确保目标结构体中的 data 指针已分配足够的内存空间
 *
 * @example
 *   CroppedImage8 grayImg;
 *   CroppedImage8 binImg;
 *   toBinary(&grayImg, &binImg, 128);
 */
void toBinary(CroppedImage8* src, CroppedImage8* dst, uint8_t threshold);

/**
 * @brief 使用双阈值将8位灰度图像转换为二值图像
 *
 * 使用上下两个阈值将8位灰度图像转换为二值图像，可以有效地提取特定灰度范围的目标。
 * 灰度值在指定范围内的像素转换为白色，其他转换为黑色。
 *
 * @param src 源8位灰度图像结构体指针
 * @param dst 目标8位二值图像结构体指针
 * @param thresholdUp 上阈值 (0-255)
 * @param thresholdDown 下阈值 (0-255)
 *
 * @note 转换规则：
 *       - 当 thresholdDown <= 灰度值 < thresholdUp 时 → 255（白色）
 *       - 当 灰度值 < thresholdDown 时 → 0（黑色）
 *       - 当 灰度值 >= thresholdUp 时 → 0（黑色）
 *
 * @note 双阈值二值化适用于：
 *       - 提取特定灰度范围的目标
 *       - 去除过亮或过暗的噪声
 *       - 边缘检测中的双阈值滞后
 *
 * @warning 确保目标结构体中的 data 指针已分配足够的内存空间
 * @warning thresholdDown 应该小于 thresholdUp，否则不会有像素被设为白色
 *
 * @example
 *   CroppedImage8 grayImg;
 *   CroppedImage8 binImg;
 *   // 提取灰度值在50-200之间的像素
 *   toBinaryDoubleThreshold(&grayImg, &binImg, 200, 50);
 */
void toBinaryDoubleThreshold(CroppedImage8* src, CroppedImage8* dst, uint8_t thresholdUp, uint8_t thresholdDown);

/**
 * @brief 将RGB565图像转换为8位二值图像
 *
 * 将 uint16_t 类型的RGB565图像转换为 uint8_t 类型的二值图像。
 * 根据自定义的RGB阈值范围判断每个像素是否为前景。
 *
 * @param src 源RGB565图像结构体指针
 * @param dst 目标8位二值图像结构体指针
 * @param background 背景色值 (BLACK/WHITE)
 * @param r_min 红色分量最小阈值 (0-255)
 * @param r_max 红色分量最大阈值 (0-255)
 * @param g_min 绿色分量最小阈值 (0-255)
 * @param g_max 绿色分量最大阈值 (0-255)
 * @param b_min 蓝色分量最小阈值 (0-255)
 * @param b_max 蓝色分量最大阈值 (0-255)
 *
 * @note 转换规则：
 *       - 当 r_min <= R <= r_max && g_min <= G <= g_max && b_min <= B <= b_max 时 → 前景色
 *       - 否则 → 背景色
 *
 * @note RGB565格式转换：
 *       - R: (pixel >> 11) << 3  (5位扩展到8位)
 *       - G: ((pixel >> 5) & 0x3F) << 2  (6位扩展到8位)
 *       - B: (pixel & 0x1F) << 3  (5位扩展到8位)
 *
 * @warning 确保目标结构体中的 data 指针已分配足够的内存空间
 * @warning 确保 min 值小于等于对应的 max 值
 *
 * @example
 *   CroppedImage16 rgb565Img;
 *   CroppedImage8 binImg;
 *   // 提取红色区域 (红色值128-255, 绿色值0-100, 蓝色值0-100)，背景为0
 *   rgb565toBinary(&rgb565Img, &binImg, 0, 128, 255, 0, 100, 0, 100);
 */
void rgb565toBinary(CroppedImage16* src, CroppedImage8* dst, uint8_t background, uint8_t r_min, uint8_t r_max, uint8_t g_min, uint8_t g_max, uint8_t b_min, uint8_t b_max);

//=================================================连通区域检测相关结构体定义=================================================

/**
 * @brief 简单信标区域（Simple Beacon Area）结构体
 *
 * 用于存储连通区域的基本信息，包括质心坐标和面积。
 *
 * @note SBEA 是 Simple Beacon Area 的缩写
 * @note 质心坐标通过区域内所有像素坐标的算术平均值计算得出
 * @note 面积表示该连通区域包含的像素总数
 */
typedef struct
{
    uint32_t beaX;     ///< 区域质心的X坐标（像素为单位）
    uint32_t beaY;     ///< 区域质心的Y坐标（像素为单位）
    uint16_t beaArea;  ///< 区域面积（像素数量）
} SBEA;

/**
 * @brief 信标区域信息（Beacon Area Information）结构体
 *
 * 用于存储连通区域检测的所有参数、数据和结果。包含检测配置、图像数据指针、
 * 过滤条件以及检测结果数组。
 *
 * @note BEAINF 是 Beacon Area Information 的缩写
 * @note 支持最多100个连通区域的检测和存储
 * @note 结构体必须通过 twoPassConnectedAreaInit() 函数初始化后才能使用
 */
typedef struct
{
    uint8_t background;  ///< 背景色值（BLACK=0 或 WHITE=255）

    CroppedImage8* data;  ///< 源二值图像数据指针（输入）
    CroppedImage8* show;  ///< 显示用图像指针（输出，用于标记检测结果）

    uint16_t areaRange[2];  ///< 面积过滤范围 [最小面积, 最大面积]
    uint16_t xRange[2];     ///< X坐标有效范围 [最小X, 最大X]（基于死区计算）
    uint16_t yRange[2];     ///< Y坐标有效范围 [最小Y, 最大Y]（基于死区计算）

    uint16_t beaCount;                   ///< 检测到的有效区域数量（0-100）
    uint8_t lastFrameBeaExist;           ///< 上一帧是否有有效区域（0=无, 1=有）
    uint32_t lastFrameSelectedBeaXY[2];  ///< 上一帧选中区域的质心坐标（X, Y）
    uint16_t selectedIndex;              ///< 选中的区域索引（100表示未选中任何区域）

    SBEA sbea[100];  ///< 检测结果数组，存储所有符合条件的连通区域信息
} BEAINF;

//=================================================连通区域检测函数声明=================================================

/**
 * @brief 初始化两次扫描连通区域检测对象
 *
 * 初始化BEAINF结构体，设置必要的参数用于后续的连通区域检测。
 *
 * @param obj BEAINF结构体指针，用于存储检测参数和结果
 * @param background 背景色值（BLACK/WHITE）
 * @param src 源8位图像结构体指针
 * @param sho 显示用8位图像结构体指针，用于标记检测结果
 * @param areaMin 连通区域的最小面积阈值
 * @param areaMax 连通区域的最大面积阈值
 * @param leftDeadZone 左侧死区比例（0.0-1.0）
 * @param rightDeadZone 右侧死区比例（0.0-1.0）
 * @param topDeadZone 顶部死区比例（0.0-1.0）
 * @param bottomDeadZone 底部死区比例（0.0-1.0）
 *
 * @note 此函数必须在调用 twoPassFourConnectedAreaProcess() 之前调用
 * @note 函数会重置 beaCount 为0
 * @note src和sho必须是已分配内存的有效图像结构体
 * @note 死区参数用于定义有效检测区域，值为0.0表示无死区，值为0.1表示10%的边缘死区
 *
 * @warning 确保 src 和 sho 指向的图像结构体已正确初始化且内存已分配
 * @warning areaMin 应该小于等于 areaMax
 * @warning 死区参数应在0.0-1.0范围内，且左+右死区、上+下死区不应超过1.0
 *
 * @example
 *   BEAINF detector;
 *   CroppedImage8 srcImg, showImg;
 *   // ... 初始化图像 ...
 *   // 设置面积范围10-1000，左右各10%死区，上下各5%死区
 *   twoPassConnectedAreaInit(&detector, 0, &srcImg, &showImg, 10, 1000, 0.1f, 0.1f, 0.05f, 0.05f);
 */
void twoPassConnectedAreaInit(BEAINF* obj, uint8_t background, CroppedImage8* src, CroppedImage8* sho, uint16_t areaMin, uint16_t areaMax, float leftDeadZone, float rightDeadZone, float topDeadZone, float bottomDeadZone);

/**
 * @brief 执行两次扫描四连通区域检测
 *
 * 使用两次扫描算法检测二值图像中的四连通区域，计算每个区域的质心和面积，
 * 并根据面积阈值过滤结果。合格的区域会在显示图像中用十字标记其质心位置。
 * 支持智能信标选择：根据上一帧状态选择最佳信标。
 *
 * @param obj 已初始化的BEAINF结构体指针
 *
 * @note 检测算法流程：
 *       1. 第一次扫描：标记连通区域并记录标签等价关系
 *       2. 第二次扫描：计算每个区域的质心坐标和面积
 *       3. 过滤阶段：根据面积阈值过滤并标记合格区域
 *       4. 智能选择：根据上一帧信息选择最佳信标
 *
 * @note 信标选择逻辑：
 *       - 当上一帧无有效信标时：选择面积最大的信标
 *       - 当上一帧有有效信标时：选择距离上一帧选中信标最近的信标
 *       - 选中的信标索引存储在 obj->selectedIndex 中
 *
 * @note 四连通：只检测上、下、左、右四个方向的邻接像素
 * @note 检测结果存储在 obj->sbea 数组中，有效数量为 obj->beaCount
 * @note 显示图像中会用十字标记标识检测到的区域质心
 * @note 函数会自动更新 obj->lastFrameBeaExist 和 obj->lastFrameSelectedBeaXY
 *
 * @warning 必须先调用 twoPassConnectedAreaInit() 进行初始化
 * @warning 函数会修改显示图像的内容
 * @warning 最大支持1024个不同的标签，超出可能导致未定义行为
 * @warning 如果内存分配失败，函数会提前返回
 *
 * @example
 *   BEAINF detector;
 *   // ... 初始化检测器 ...
 *   twoPassFourConnectedAreaProcess(&detector);
 *   printf("检测到 %d 个区域\n", detector.beaCount);
 *   if(detector.selectedIndex != 100) {
 *       printf("选中信标: 质心(%d,%d), 面积=%d\n", 
 *              detector.sbea[detector.selectedIndex].beaX, 
 *              detector.sbea[detector.selectedIndex].beaY, 
 *              detector.sbea[detector.selectedIndex].beaArea);
 *   }
 *   for(int i = 0; i < detector.beaCount; i++) {
 *       printf("区域%d: 质心(%d,%d), 面积=%d\n", i,
 *              detector.sbea[i].beaX, detector.sbea[i].beaY, detector.sbea[i].beaArea);
 *   }
 */
void twoPassFourConnectedAreaProcess(BEAINF* obj);

/**
 * @brief 执行两次扫描八连通区域检测
 *
 * 使用两次扫描算法检测二值图像中的八连通区域，计算每个区域的质心和面积，
 * 并根据面积阈值过滤结果。合格的区域会在显示图像中用十字标记其质心位置。
 * 支持智能信标选择：根据上一帧状态选择最佳信标。
 *
 * @param obj 已初始化的BEAINF结构体指针
 *
 * @note 检测算法流程：
 *       1. 第一次扫描：标记连通区域并记录标签等价关系
 *       2. 第二次扫描：计算每个区域的质心坐标和面积
 *       3. 过滤阶段：根据面积阈值过滤并标记合格区域
 *       4. 智能选择：根据上一帧信息选择最佳信标
 *
 * @note 信标选择逻辑：
 *       - 当上一帧无有效信标时：选择面积最大的信标
 *       - 当上一帧有有效信标时：选择距离上一帧选中信标最近的信标
 *       - 选中的信标索引存储在 obj->selectedIndex 中
 *
 * @note 八连通：检测上、下、左、右、左上、右上、左下、右下八个方向的邻接像素
 * @note 相比四连通，八连通能检测到更多的连接模式，特别是对角连接的区域
 * @note 检测结果存储在 obj->sbea 数组中，有效数量为 obj->beaCount
 * @note 显示图像中会用十字标记标识检测到的区域质心
 * @note 函数会自动更新 obj->lastFrameBeaExist 和 obj->lastFrameSelectedBeaXY
 *
 * @warning 必须先调用 twoPassConnectedAreaInit() 进行初始化
 * @warning 函数会修改显示图像的内容
 * @warning 最大支持1024个不同的标签，超出可能导致未定义行为
 * @warning 如果内存分配失败，函数会提前返回
 * @warning 八连通比四连通计算复杂度更高，处理时间更长
 *
 * @example
 *   BEAINF detector;
 *   // ... 初始化检测器 ...
 *   twoPassEightConnectedAreaProcess(&detector);
 *   printf("检测到 %d 个区域\n", detector.beaCount);
 *   if(detector.selectedIndex != 100) {
 *       printf("选中信标: 质心(%d,%d), 面积=%d\n", 
 *              detector.sbea[detector.selectedIndex].beaX, 
 *              detector.sbea[detector.selectedIndex].beaY, 
 *              detector.sbea[detector.selectedIndex].beaArea);
 *   }
 *   for(int i = 0; i < detector.beaCount; i++) {
 *       printf("区域%d: 质心(%d,%d), 面积=%d\n", i,
 *              detector.sbea[i].beaX, detector.sbea[i].beaY, detector.sbea[i].beaArea);
 *   }
 */
void twoPassEightConnectedAreaProcess(BEAINF* obj);

#endif  // OPENMORPH_H