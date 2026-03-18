/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
 */
#include "project.h"
#include "GUI.h"
#include "BSP.h"

void BSP_PreInit(void)
{
    /* 使能全局中断（需先使能，否则部分组件如CapSense会初始化不成功） */
    __enable_irq();

    /* 初始化CapSense组件 */
    if (CapSense_Start() == CY_RET_SUCCESS)
    {
        CapSense_SetGestureTimestamp(0u);
    }
    else
    {
        CY_ASSERT(0); /* CapSense初始化失败，停止CPU */
    }

    /* 初始化按键模块 */
    Button_Module_Start();
}

void BSP_PostInit(void)
{
    /* 初始化emWin Graphics（使用了系统SysTick中断hook函数，须在OS启动后进行） */
    GUI_Init();             /* 初始化图形界面 */
    GUI_UC_SetEncodeUTF8(); /* 设置UTF-8编码用于显示汉字 */
    GUI_Clear();            /* 清除图形界面 */
}
