/**
 ****************************************************************************************************
 * @file        atk_ms901m_uart.h
 * @author      正点原子团队(ALIENTEK) 
 * @brief       ATK-MS901M (WT901C) 模块 UART 接口驱动头文件
 ****************************************************************************************************
 */

#ifndef __MS901_UART_H
#define __MS901_UART_H

#include "stm32f10x.h"  /* 引入 STM32F10x 标准外设库核心头文件 */

/* 引脚定义 */
#define ATK_MS901M_UART_TX_GPIO_PORT            GPIOB
#define ATK_MS901M_UART_TX_GPIO_PIN             GPIO_Pin_10

#define ATK_MS901M_UART_RX_GPIO_PORT            GPIOB
#define ATK_MS901M_UART_RX_GPIO_PIN             GPIO_Pin_11

/* 串口外设与中断定义 */
#define ATK_MS901M_UART_INTERFACE               USART3
#define ATK_MS901M_UART_IRQn                    USART3_IRQn
#define ATK_MS901M_UART_IRQHandler              USART3_IRQHandler

/* UART接收FIFO缓冲大小 */
#define ATK_MS901M_UART_RX_FIFO_BUF_SIZE        128

/* 操作函数声明 */
uint8_t atk_ms901m_uart_rx_fifo_write(uint8_t *dat, uint16_t len);  /* ATK-MS901M UART接收FIFO写入数据 */
uint16_t atk_ms901m_uart_rx_fifo_read(uint8_t *dat, uint16_t len);  /* ATK-MS901M UART接收FIFO读取数据 */
void atk_ms901m_rx_fifo_flush(void);                                /* ATK-MS901M UART接收FIFO清空 */
void atk_ms901m_uart_send(uint8_t *dat, uint8_t len);               /* ATK-MS901M UART发送数据 */
void atk_ms901m_uart_init(uint32_t baudrate);                       /* ATK-MS901M UART初始化 */

#endif

