//十轴传感器  rx  tx fifo
/**
 ****************************************************************************************************
 * @file        atk_ms901m_uart.c
 * @brief       ATK-MS901M (WT901C) 模块 UART 接口驱动代码 (STM32F10x 标准库版)
 ****************************************************************************************************
 */

#include "ms901-uart.h"
#include "stm32f10x.h"                  // Device header

/* 姿态数据接收环形 FIFO 缓冲及指针 */
static uint8_t atk_ms901m_rx_fifo[ATK_MS901M_UART_RX_FIFO_BUF_SIZE];
static uint16_t atk_ms901m_rx_fifo_head = 0;
static uint16_t atk_ms901m_rx_fifo_tail = 0;

/**
 * @brief       ATK-MS901M UART初始化
 * @param       baudrate: 串口波特率 (WT901C 常用 115200 或 9600)
 * @retval      无
 */
void atk_ms901m_uart_init(uint32_t baudrate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 1. 使能 GPIOB 和 USART3 核心时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    /* 2. 配置 USART3 Tx (PB10) - 强推挽复用输出 */
    GPIO_InitStructure.GPIO_Pin = ATK_MS901M_UART_TX_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ATK_MS901M_UART_TX_GPIO_PORT, &GPIO_InitStructure);

    /* 3. 配置 USART3 Rx (PB11) - 浮空输入 */
    GPIO_InitStructure.GPIO_Pin = ATK_MS901M_UART_RX_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(ATK_MS901M_UART_RX_GPIO_PORT, &GPIO_InitStructure);

    /* 4. USART3 NVIC 中断优先级配置 (作为姿态输入，优先级建议设高) */
    NVIC_InitStructure.NVIC_IRQChannel = ATK_MS901M_UART_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // 抢占优先级1 (根据实际工程调整)
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        // 子优先级0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* 5. USART3 协议参数初始化 */
    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(ATK_MS901M_UART_INTERFACE, &USART_InitStructure);

    /* 6. 开启接收中断 (RXNE) 并使能 USART3 */
    USART_ITConfig(ATK_MS901M_UART_INTERFACE, USART_IT_RXNE, ENABLE); 
    USART_Cmd(ATK_MS901M_UART_INTERFACE, ENABLE);
}

/**
 * @brief       ATK-MS901M UART发送数据
 * @param       dat: 数据指针
 * @param       len: 数据长度
 * @retval      无
 */
void atk_ms901m_uart_send(uint8_t *dat, uint8_t len)
{
    uint8_t i;
    for(i = 0; i < len; i++)
    {
        USART_SendData(ATK_MS901M_UART_INTERFACE, dat[i]);
        /* 轮询等待发送完成标志位 (TC) */
        while(USART_GetFlagStatus(ATK_MS901M_UART_INTERFACE, USART_FLAG_TC) == RESET); 
    }
}

/**
 * @brief       ATK-MS901M UART接收FIFO写入数据 (在中断中调用)
 * @param       dat: 待写入数据指针
 * @param       len: 待写入数据长度
 * @retval      0: 成功; 1: FIFO满丢包
 */
uint8_t atk_ms901m_uart_rx_fifo_write(uint8_t *dat, uint16_t len)
{
    uint16_t i;
    for(i = 0; i < len; i++)
    {
        uint16_t next_head = (atk_ms901m_rx_fifo_head + 1) % ATK_MS901M_UART_RX_FIFO_BUF_SIZE;
        
        /* 溢出保护：如果头指针追上尾指针，说明缓冲区已满 */
        if(next_head == atk_ms901m_rx_fifo_tail) 
        {
            return 1; 
        }
        
        atk_ms901m_rx_fifo[atk_ms901m_rx_fifo_head] = dat[i];
        atk_ms901m_rx_fifo_head = next_head;
    }
    return 0; 
}

/**
 * @brief       ATK-MS901M UART接收FIFO读取数据 (在主循环中调用)
 * @param       dat: 读取数据存放的指针
 * @param       len: 准备读取的数据长度
 * @retval      实际读取到的数据长度
 */
uint16_t atk_ms901m_uart_rx_fifo_read(uint8_t *dat, uint16_t len)
{
    uint16_t read_cnt = 0;
    while(read_cnt < len && atk_ms901m_rx_fifo_tail != atk_ms901m_rx_fifo_head)
    {
        dat[read_cnt++] = atk_ms901m_rx_fifo[atk_ms901m_rx_fifo_tail];
        atk_ms901m_rx_fifo_tail = (atk_ms901m_rx_fifo_tail + 1) % ATK_MS901M_UART_RX_FIFO_BUF_SIZE;
    }
    return read_cnt;
}

/**
 * @brief       ATK-MS901M UART接收FIFO清空
 * @param       无
 * @retval      无
 */
void atk_ms901m_rx_fifo_flush(void)
{
    atk_ms901m_rx_fifo_head = 0;
    atk_ms901m_rx_fifo_tail = 0;
}

/**
 * @brief       USART3 中断服务函数
 * @param       无
 * @retval      无
 */
volatile uint32_t ms901_rx_cnt = 0;

void USART3_IRQHandler(void)
{
    uint8_t res;

    if(USART_GetFlagStatus(USART3,USART_FLAG_ORE)!=RESET)
    {
        USART_ReceiveData(USART3);
    }

    if(USART_GetITStatus(USART3,USART_IT_RXNE)!=RESET)
    {
        res=(uint8_t)USART_ReceiveData(USART3);

        atk_ms901m_uart_rx_fifo_write(&res,1);
    }
}



