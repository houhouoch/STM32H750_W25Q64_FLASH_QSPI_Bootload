# STM32H750 + W25Q64 QSPI Flash (XIP) Bootloader 移植指南

本项目基于安富莱（Armfly）V7 教程移植，针对 **STM32H750IBK6** 与外部 **QSPI Flash (W25Q64)** 实现 XIP（Execute In Place）直接运行模式。

---

## 🛠️ 1. 环境背景与底层架构

* **MCU**: STM32H750IBK6 (Cortex-M7, 480MHz)。
* **存储环境**: 程序存储于外部 W25Q64，通过 QSPI 映射至地址 `0x90000000` 实现 XIP 运行。
* **时钟源 (Timebase Source)**: HAL 库 Tick 源配置为 **TIM6**。
    * **深度解析**: 在 H7 这种高性能架构中，若配合 FreeRTOS 等 OS 使用，`SysTick` 会被内核占用。ST 官方建议 HAL 库使用独立定时器（如 TIM6/TIM7）作为 Tick 维护者，以避免中断优先级冲突和任务调度干扰。

---

## ⚙️ 2. APP（从程序）关键配置

APP 若要从 Bootloader 成功跳转并稳定运行，必须满足以下硬性条件：

1.   **内部 Flash (0x08000000)**：存放 **Bootloader**。
        * 负责初始化 QSPI、校验外部程序完整性、管理固件升级。
     **外部 QSPI Flash (0x90000000)**：
      * **0x90000000 - 0x90001FFF (8KB)**：**APP 信息区 (Header/Descriptor)**。
        * 用于存放 APP 的校验和 (CRC)、版本号、文件长度等。
     **0x90002000 - 结束**：**APP 固件区**。
        * APP 的真正起始位置，也是中断向量表 (`SCB->VTOR`) 存放处。

   **起始地址**: 设置为 `0x90002000`（偏移 8KB，留点空间 进行开机前的校验（留出信息区）  当然你也可以设置成0x90000000 （确保这个和Read Memory Areas一致即可） ）。
    * 需同步修改 MDK 的 `Target -> IROM1` 设置。
    
2.    **向量表重定向**: 在 APP 的 `main` 函数第一行执行：  
    ```c
    SCB->VTOR = 0x90002000; // 权力交接的核心：确保中断指向 APP 自己的函数
    ```
    > [!IMPORTANT]
    > 若漏掉此步，APP 触发任何中断（如串口、定时器）时，CPU 会跳回 Bootloader 的中断向量表，导致死机。

---

## ⚠️ 3. 核心避坑：跳转挂死 (Hang) 问题诊断

### 踩坑现场
**现象**: 跳转到 APP 的瞬间，直接卡死在 `HAL_TIM_PeriodElapsedCallback` 或触发 `HardFault`。
**病因 (带伤跳转)**: 
Bootloader 在跳转前虽然调用了 `__disable_irq()` 屏蔽了 CPU 总中断，但 **TIM6 硬件定时器仍在计数**。跳转后，APP 尚未配置好新的 `VTOR` 向量表，此时硬件 TIM6 溢出抛出中断请求。由于 `VTOR` 还是旧的，CPU 会尝试执行旧地址的回调，导致逻辑错乱。

### 解药
**`HAL_SuspendTick()`**: 必须在跳转前调用。
该函数不仅是暂停计数，更重要的是它会禁用定时器的中断使能位（DIER 寄存器），从根源上切断硬件向内核发送信号的路径。

---

## 🚀 4. 标准跳转流程 (Best Practice)

### 第一阶段：Bootloader 离场清理
在执行 `Jump()` 之前，必须手动将 CPU 恢复到近乎“刚上电”的干净状态：

1.  **停掉 Tick 源**: `HAL_SuspendTick()`，防止跳转瞬间产生硬件中断。
2.  **回归时钟树**: `HAL_RCC_DeInit()`。将时钟（PLL、HCLK）重置为 HSI (64MHz)，避免残留的高主频配置干扰 APP 初始化。
3.  **使能映射**: `BspQspiBoot_MemMapped()`。**极其关键**，若 QSPI 不在映射模式，CPU 跳转到 `0x9000xxxx` 地址时会因为找不到代码而直接 HardFault。
4.  **清理 NVIC 标志**: 
    * 通过循环 `NVIC->ICER` 清除所有中断使能。
    * 通过循环 `NVIC->ICPR` 清除所有挂起的中断标志。
5.  **跳转**: 严禁在跳转前开启全局中断。

### 第二阶段：APP 接管启动
1.  **重置 VTOR**: 抢在任何硬件外设初始化前，完成向量表映射。
2.  **环境重建**: `HAL_Init()` -> `SystemClock_Config(480MHz)`。
3.  **开启全局中断**: 环境就绪，正式开启 `__enable_irq()`。

---

## 💡 知识点补充 (Knowledge Expansion)

### 1. NVIC 寄存器的清理逻辑
很多简单的 Bootloader 只是关闭了总中断（PRIMASK），但没有清理 NVIC 的挂起状态（Pending）。
* **ICPR (Interrupt Clear-Pending Register)**: 如果 Bootloader 运行期间某个外设产生了中断但没处理，这个标志会一直挂着。跳转后一旦开启总中断，CPU 会立刻冲进该中断服务程序。**务必清零所有 Pending 位**。

### 2. H7 系列的 Cache 一致性
由于 STM32H7 具有 **L1 Cache**（指令缓存 I-Cache 和数据缓存 D-Cache）：
* 在 Bootloader 涉及到修改外部 Flash 代码（如 IAP 升级）后，跳转前必须调用 `SCB_CleanInvalidateDCache()`。
* 否则，CPU 可能会执行 Cache 中缓存的旧代码，而不是刚写进去的新 APP 代码。

### 3. 跳转函数的汇编细节
跳转时通常采用以下逻辑：
```c
/* 设置主堆栈指针 MSP */
__set_MSP(*(uint32_t *)APP_ADDRESS);
/* 获取复位向量地址并跳转 */
JumpFunction = (pFunction)(*(uint32_t *)(APP_ADDRESS + 4));
JumpFunction();
