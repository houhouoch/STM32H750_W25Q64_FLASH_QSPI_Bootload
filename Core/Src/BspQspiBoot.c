#include "BspQspiBoot.h"
#include "quadspi.h"

uint32_t BspQspiBoot_ReadID(void)
{
    QSPI_CommandTypeDef sCommand = {0};
    uint32_t ulID;
    uint8_t ucaBuff[3] = {0};
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
    sCommand.Instruction       = BQB_Cmd_ReadID;
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.DataMode          = QSPI_DATA_1_LINE;
    sCommand.NbData            = 3;
    sCommand.DummyCycles       = 0;
    if(HAL_QSPI_Command(&hqspi, &sCommand, HAL_MAX_DELAY) != HAL_OK)
    {
        Error_Handler();
    }
    if(HAL_QSPI_Receive(&hqspi, ucaBuff, HAL_MAX_DELAY) != HAL_OK)
    {
        Error_Handler();
    }
    ulID = (ucaBuff[0] << 16) | (ucaBuff[1] << 8) | ucaBuff[2];
    return ulID;
}

static int BspQspiBoot_AutoPollingMemReady(QSPI_HandleTypeDef *p_hqspi)
{
    QSPI_CommandTypeDef     sCommand = {0};
    QSPI_AutoPollingTypeDef sConfig = {0};
    /* 基本配置 */
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* 1线方式发送指令 */
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;       /* 24位地址 */
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* 无交替字节 */
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* W25Q64JV不支持DDR */
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* DDR模式，数据输出延迟 */
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   /* 每次传输都发指令 */
    /* 读取状态*/
    sCommand.Instruction       = BQB_Cmd_ReadStatus1;        /* 读取状态命令 */
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;          /* 无需地址 */
    sCommand.DataMode          = QSPI_DATA_1_LINE;           /* 1线数据 */
    sCommand.DummyCycles       = 0;                          /* 无需空周期 */
    /* 屏蔽位设置的bit0，匹配位等待bit0为0，即不断查询状态寄存器bit0，等待其为0 */
    sConfig.Mask               = 0x01;
    sConfig.Match              = 0x00;
    sConfig.MatchMode          = QSPI_MATCH_MODE_AND;
    sConfig.StatusBytesSize    = 1;
    sConfig.Interval           = 0x10;
    sConfig.AutomaticStop      = QSPI_AUTOMATIC_STOP_ENABLE;
    if(HAL_QSPI_AutoPolling(p_hqspi, &sCommand, &sConfig, HAL_MAX_DELAY) != HAL_OK)
    {
        return 1;
    }
    return 0;
}

int BspQspiBoot_WriteEnable(void)
{
    QSPI_CommandTypeDef sCommand = {0};
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
    sCommand.Instruction       = BQB_Cmd_WriteEnable;
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.DataMode          = QSPI_DATA_NONE;
    sCommand.NbData            = 0;
    sCommand.DummyCycles       = 0;
    if(HAL_QSPI_Command(&hqspi, &sCommand, HAL_MAX_DELAY) != HAL_OK)
    {
        return 1;
    }
    return 0;
}

int BspQspiBoot_WriteDisable(void)
{
    QSPI_CommandTypeDef sCommand = {0};
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
    sCommand.Instruction       = BQB_Cmd_WriteDisable;
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.DataMode          = QSPI_DATA_NONE;
    sCommand.NbData            = 0;
    sCommand.DummyCycles       = 0;
    if(HAL_QSPI_Command(&hqspi, &sCommand, HAL_MAX_DELAY) != HAL_OK)
    {
        return 1;
    }
    return 0;
}


int BspQspiBoot_EraseSector(uint32_t ulSectorAddr)
{
    QSPI_CommandTypeDef sCommand = {0};
    /* 写使能 */
    BspQspiBoot_WriteEnable();
    /* 基本配置 */
    sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;             /* 1线方式发送指令 */
    sCommand.AddressSize = QSPI_ADDRESS_24_BITS;                    /* 32位地址 */
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;         /* 无交替字节 */
    sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;                       /* W25Q256JV不支持DDR */
    sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;          /* DDR模式，数据输出延迟 */
    sCommand.SIOOMode = QSPI_SIOO_INST_ONLY_FIRST_CMD;                   /* 每次传输都发指令 */
    /* 擦除配置 */
    sCommand.Instruction = BQB_Cmd_SectorErase;         /* 32bit地址方式的扇区擦除命令，扇区大小64KB*/
    sCommand.AddressMode = QSPI_ADDRESS_1_LINE;                     /* 地址发送是1线方式 */
    sCommand.Address = ulSectorAddr;                                     /* 扇区首地址，保证是4KB整数倍 */
    sCommand.DataMode = QSPI_DATA_NONE;                             /* 无需发送数据 */
    sCommand.DummyCycles = 0;                                       /* 无需空周期 */
    if(HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return 1;
    }
    /* 等待编程结束 */
    if(BspQspiBoot_AutoPollingMemReady(&hqspi) == 1)
    {
        return 1;
    }
    return 0;
}

int BspQspiBoot_EraseChip(void)
{
    QSPI_CommandTypeDef sCommand = {0};
    BspQspiBoot_WriteEnable();
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
    sCommand.Instruction       = BQB_Cmd_ChipErase;
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.DataMode          = QSPI_DATA_NONE;
    sCommand.NbData            = 0;
    sCommand.DummyCycles       = 0;
    if(HAL_QSPI_Command(&hqspi, &sCommand, HAL_MAX_DELAY) != HAL_OK)
    {
        return 1;
    }
    if(BspQspiBoot_AutoPollingMemReady(&hqspi) == 1)
    {
        return 1;
    }
    return 0;
}

int BspQspiBoot_WritePage(uint8_t *p_Buff, uint32_t ulWriteAddr, uint16_t usWriteSize)
{
    QSPI_CommandTypeDef sCommand = {0};
    BspQspiBoot_WriteEnable();
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_ONLY_FIRST_CMD;
    sCommand.Instruction       = BQB_Cmd_PageProgram_Quad;
    sCommand.AddressMode       = QSPI_ADDRESS_1_LINE;
    sCommand.Address           = ulWriteAddr;
    sCommand.DataMode          = QSPI_DATA_4_LINES;
    sCommand.NbData            = usWriteSize;
    sCommand.DummyCycles       = 0;
    if(HAL_QSPI_Command(&hqspi, &sCommand, HAL_MAX_DELAY) != HAL_OK)
    {
        return 1;
    }
    if(HAL_QSPI_Transmit(&hqspi, p_Buff, HAL_MAX_DELAY) != HAL_OK)
    {
        return 1;
    }
    if(BspQspiBoot_AutoPollingMemReady(&hqspi) == 1)
    {
        return 1;
    }
    return 0;
}

void BspQspiBoot_Write_NoCheck(uint32_t addr, uint8_t *buffer, uint16_t length)
{
    uint16_t pageremain;
    pageremain = 256 - addr % 256; // 单页剩余的字节数
    if(length <= pageremain)
    {
        pageremain = length;    // 不大于256个字节
    }
    while(1)
    {
        BspQspiBoot_WritePage(buffer, addr, pageremain);
        if(length == pageremain)
        {
            break;    // 写入结束了
        }
        else // NumByteToWrite>pageremain
        {
            buffer += pageremain;
            addr += pageremain;
            length -= pageremain;       // 减去已经写入了的字节数
            if(length > 256)
            {
                pageremain = 256;    // 一次可以写入256个字节
            }
            else
            {
                pageremain = length;    // 不够256个字节了
            }
        }
    }
}


int BspQspiBoot_ReadBuff(uint8_t *p_Buff, uint32_t ulReadAddr, uint32_t ulReadSize)
{
    QSPI_CommandTypeDef sCommand = {0};
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;       /* 1线方式发送指令 */
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;          /* 24位地址 */
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;     /* 无交替字节 */
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;         /* W25Q64JV不支持DDR */
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;     /* DDR模式，数据输出延迟 */
    sCommand.SIOOMode          = QSPI_SIOO_INST_ONLY_FIRST_CMD;      /* 每次传输要发指令 */
    sCommand.Instruction       = BQB_Cmd_FastRead_Quad;       /* 24bit地址的4线快速读取命令 */
    sCommand.AddressMode       = QSPI_ADDRESS_4_LINES;        /* 4线地址 */
    sCommand.Address           = ulReadAddr;                  /* 读取数据的起始地址 */
    sCommand.DataMode          = QSPI_DATA_4_LINES;           /* 4线数据 */
    sCommand.NbData            = ulReadSize;                  /* 读取的数据大小 */
    sCommand.DummyCycles       = 6;                           /* 空周期 */
    if(HAL_QSPI_Command(&hqspi, &sCommand, HAL_MAX_DELAY) != HAL_OK)
    {
        return 1;
    }
    if(HAL_QSPI_Receive(&hqspi, p_Buff, HAL_MAX_DELAY) != HAL_OK)
    {
        return 1;
    }
    if(BspQspiBoot_AutoPollingMemReady(&hqspi) == 1)
    {
        return 1;
    }
    return 0;
}

int BspQspiBoot_MemMapped(void)
{
    QSPI_CommandTypeDef s_command = {0};
    QSPI_MemoryMappedTypeDef s_mem_mapped_cfg = {0};
    /* 基本配置 */
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;      /* 1线方式发送指令 */
    s_command.AddressSize = QSPI_ADDRESS_24_BITS;             /* 32位地址 */
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* 无交替字节 */
    s_command.DdrMode = QSPI_DDR_MODE_DISABLE;                /* 不支持DDR */
    s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;   /* DDR模式，数据输出延迟 */
    s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;            /* 每次传输都发指令 */
    /* 全部采用4线 */
    s_command.Instruction = BQB_Cmd_FastRead_Quad; /* 快速读取命令 */
    s_command.AddressMode = QSPI_ADDRESS_4_LINES;                 /* 4个地址线 */
    s_command.DataMode = QSPI_DATA_4_LINES;                       /* 4个数据线 */
    s_command.DummyCycles = 6;                                    /* 空周期 */
    /* 关闭溢出计数 */
    s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
    s_mem_mapped_cfg.TimeOutPeriod = 0;
    if(HAL_QSPI_MemoryMapped(&hqspi, &s_command, &s_mem_mapped_cfg) != HAL_OK)
    {
        Error_Handler();
    }
    return 0;
}

int BspQspiBoot_QuitMemMapped(void)
{
    HAL_QSPI_Abort(&hqspi);
    return 0;
}


