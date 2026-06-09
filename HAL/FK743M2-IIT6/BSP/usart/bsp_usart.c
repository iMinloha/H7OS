#include "bsp_usart.h"
#include "usart.h"
#include "dev_register.h"
#include <stdio.h>
#include <string.h>

/* ── Ring buffer for non-blocking RX ─────────────────────── */
#define USART_RX_BUF_SIZE  256

static volatile uint8_t  rx_buf[USART_RX_BUF_SIZE];
static volatile uint16_t rx_head = 0;
static volatile uint16_t rx_tail = 0;
static volatile int      rx_open = 0;

/* USART1 interrupt handler (weak default overridden) */
void USART1_IRQHandler(void) {
    HAL_UART_IRQHandler(&huart1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1 && huart->pRxBuffPtr) {
        rx_buf[rx_head] = *huart->pRxBuffPtr;
        rx_head = (rx_head + 1) % USART_RX_BUF_SIZE;
        if (rx_head == rx_tail) rx_tail = (rx_tail + 1) % USART_RX_BUF_SIZE;
        static uint8_t next;
        HAL_UART_Receive_IT(&huart1, &next, 1);
    }
}

static int usart1_open(void *dev) {
    (void)dev;
    rx_head = rx_tail = 0;
    if (!rx_open) {
        HAL_NVIC_EnableIRQ(USART1_IRQn);
        static uint8_t dummy;
        HAL_UART_Receive_IT(&huart1, &dummy, 1);
        rx_open = 1;
    }
    return 0;
}

static int usart1_close(void *dev) {
    (void)dev;
    HAL_UART_AbortReceive_IT(&huart1);
    HAL_NVIC_DisableIRQ(USART1_IRQn);
    rx_open = 0;
    return 0;
}

/** Non-blocking: returns available bytes immediately (0 if none) */
static int usart1_read(void *dev, uint8_t *buf, uint32_t len) {
    (void)dev;
    uint32_t got = 0;
    while (got < len && rx_tail != rx_head) {
        buf[got++] = rx_buf[rx_tail];
        rx_tail = (rx_tail + 1) % USART_RX_BUF_SIZE;
    }
    return (int)got;
}

static int usart1_write(void *dev, const uint8_t *buf, uint32_t len) {
    (void)dev;
    if (HAL_UART_Transmit(&huart1, (uint8_t *)buf, len, HAL_MAX_DELAY) == HAL_OK)
        return (int)len;
    return -1;
}

bsp_file_ops_t usart1_fops = {
    .open  = usart1_open,
    .close = usart1_close,
    .read  = usart1_read,
    .write = usart1_write,
};

void usart1_device_init(void) {
    dev_register("dev/serial", &huart1, "USART1", "Serial bus device", DEVICE_SERIAL, &usart1_fops);
}
