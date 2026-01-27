/**********************************   Bootload    **********************************/
说明：整个案例是移植安富莱的工程

1. 环境背景
*硬件架构：STM32H750IBK6 + W25Q64 (QSPI Flash)。
*运行模式：XIP (Execute In Place)，程序直接在外部 Flash 运行。
*时钟源 (Timebase Source)：HAL 库 Tick 源配置为 TIM6（而非 SysTick ）。
	*原因：在 RTOS (如 FreeRTOS) 环境下，SysTick 被内核抢占用于任务调度，ST 官方建议 HAL 库使用独立硬件定时器以确保时钟逻辑不冲突。

2. APP 关键配置
*起始地址：0x90002000 (需与 MDK Target - IROM1 设置严格一致)。
*向量表重定向：在 APP 的 main 函数首行（或 HAL_Init 之前）执行：
	*SCB->VTOR = 0x90002000; // 必须手动对齐中断向量表，否则触发中断会跳回 Bootloader

3.核心避坑：跳转挂死 (Hang) 问题诊断
踩过的坑：使用安富莱(未修改JumpToApp)的教程  
现象：跳转 APP 瞬间卡死在 HAL_TIM_PeriodElapsedCallback 或 HardFault。
病因：“带伤跳转”。Bootloader 关闭了全局中断，但未停止 TIM6 硬件计数。
*跳转到 APP 后，APP 尚未设置 VTOR。
*TIM6 计数溢出产生中断请求。

解药：HAL_SuspendTick()。
该函数通过禁用定时器中断位（DIER 寄存器），彻底切断硬件向内核发送中断请求的路径。

4. HAL_RCC_DeInit() 的必要性
作用：将时钟树（PLL、HCLK、PCLK）重置为 HSI (64MHz) 默认状态。
*清理外设时钟：确保 APP 拿到的是一个“干净”的处理器环境，避免 Bootloader 残留的时钟配置干扰 APP 的初始化逻辑。


5. 标准跳转流程 (Best Practice)
第一阶段：Bootloader 离场清理
DISABLE_INT()：关闭全局中断（__set_PRIMASK(1)）。
HAL_SuspendTick()：停掉 TIM6/SysTick 中断源。
HAL_RCC_DeInit()：时钟树回归 HSI 默认值，关闭 PLL。
BspQspiBoot_MemMapped()：确保 QSPI 映射已开启，否则 CPU 无法读取 APP 地址代码。
清除 NVIC 标志：循环执行 ICER 和 ICPR，清除所有挂起的中断。
跳转：设置 MSP 堆栈指针，获取复位向量地址，执行 Jump()。
注：跳转前严禁开启全局中断（让app去开启）。


第二阶段：APP 接管启动
执行启动汇编：执行 Reset_Handler。
重置 VTOR：SCB->VTOR = 0x90002000;（权力交接的最关键一步）。
HAL_Init()：重新初始化 HAL 基础环境。
SystemClock_Config()：将 H7 主频重新拉升至 480MHz。
开启中断：此时环境已就绪，CPU 可以在新的向量表指导下安全处理中断。

