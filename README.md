# PSoC Ebook Reader

基于Cypress PSoC 6平台的电子阅读器项目，双核ARM Cortex-M0+和Cortex-M4架构。

## 硬件平台

- **MCU**: PSoC 6 (CY8C6xx7)
- **架构**: 双核 (CM0+ + CM4)
- **显示屏**: E-Ink电子墨水屏
- **存储**: 外部Flash

## 功能特性

- 双核协同工作
- FreeRTOS实时操作系统
- emWin图形界面
- 中文字体渲染
- 电子书解析与显示
- 低功耗设计

## 技术栈

- **IDE**: PSoC Creator
- **RTOS**: FreeRTOS
- **GUI**: emWin
- **语言**: C

## 项目结构

```
PSoC电子阅读器/
└── E-Reader2.cydsn/
    ├── main_cm0p.c      # CM0+核心代码
    ├── main_cm4.c       # CM4核心代码
    ├── BSP.c/h          # 板级支持包
    ├── GUIConf.c/h      # GUI配置
    ├── LCDConf.c/h      # 显示配置
    ├── Chinese_*.c      # 中文字体
    ├── FreeRTOSConfig.h # RTOS配置
    └── TopDesign/       # 原理图设计
```

## 双核分工

- **CM0+**: 系统管理、低功耗控制
- **CM4**: 主应用逻辑、GUI渲染、文件解析

## 开发环境

1. 安装PSoC Creator
2. 打开E-Reader2.cyprj
3. 编译并下载到开发板

## 学习收获

- PSoC 6双核开发
- FreeRTOS多任务编程
- emWin图形界面开发
- 电子墨水屏驱动
- 低功耗设计技巧

## 相关项目

- [STM32-Projects](https://github.com/1184197761/STM32-Projects) - STM32电子阅读器

---
*PSoC平台嵌入式开发实践*
