/**********************************   Bootload    **********************************/
说明：整个案例是移植安富莱的工程

前提：作者使用的是RTOS 使用
1、关于Cubemx 设置SYS的 Timebase Source
 默认是使用Systick （适合不带操作系统的简单工程）
作者这边使用的是 --TIM6-- （根据ST官方 建议将HAL Tick换成硬件定时器）
Systick会被RTOS用作任务调度
2、 APP地址 为AppAddr = 0x90002000;  /* APP 地址 */
这个要在APP前端SCB->VTOR = 0x90002000（要对应MDK设置中Options for Target的ROM1）;
使用这个改变中断向量表 

踩过的坑：使用安富莱(未修改JumpToApp)的教程  
现象： 跳转到APP中会卡在时钟中断中，跑不到的程序中

解决：    【添加】HAL_SuspendTick();  它是专门用来停掉 HAL 库 Tick 时钟中断的（无论底层是 Systick 还是 TIMx）。
作用：停止TIM6在后台继续计时。
遇到过的坑：不加上这个 确实是能跳转到到APP程序， 但TIM6按照 Bootloader 设定的频率不断计数。CPU 一旦发现 TIM6 计数溢出，就会去查中断向量表，CPU 就会跳回 Bootloader 的中断函数，导致逻辑混乱或卡死。

HAL_RCC_DeInit也可以加上在HAL_SuspendTick 但作用不知道是什么作用

整体的流程：Bootloader 阶段：DISABLE_INT() -> 清理外设（HAL_SuspendTick等） -> 跳转
APP 启动阶段：
执行汇编 Reset_Handler（此时中断依然是关闭的）。
进入 main 函数。
第一步：SCB->VTOR = 0x90002000;（交接向量表）。
第二步：HAL_Init();（内部会重新配置 Systick/TIMx 并自动使能全局中断）