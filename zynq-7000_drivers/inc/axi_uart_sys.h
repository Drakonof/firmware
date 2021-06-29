#ifndef INC_AXI_UART_SYS__H
#define INC_AXI_UART_SYS__H

#include <stdbool.h>

#include "axi_uart_.h"

#define AXI_UART_CONTROL_REG_OFFSET     0U
#define AXI_UART_CONTROL_REG_RST_VAL    0U
#define AXI_UART_TX_EN_BIT_OFFSET       0U
#define AXI_UART_TX_EN_BIT_MASK         (1U << AXI_UART_TX_EN_BIT_OFFSET)
#define AXI_UART_DATA_NUM_BIT_OFFSET    4U
#define AXI_UART_DATA_NUM_BIT_MASK      (1U << AXI_UART_DATA_NUM_BIT_OFFSET)
#define AXI_UART_PARITY_BIT_OFFSET      8U
#define AXI_UART_PARITY_BIT__MASK       (1U << AXI_UART_PRTT_NUM_BIT_OFFSET)
#define AXI_UART_STOP_NUM_BIT_OFFSET    12U
#define AXI_UART_STOP_NUM_BIT_MASK      (1U << AXI_UART_STOP_NUM_BIT_OFFSET)

#define AXI_UART_BAUD_REG_OFFSET        4U
#define AXI_UART_BAUD_REG_RST_VAL       0U

#define AXI_UART_TX_DATA_REG_OFFSET     8U
#define AXI_UART_TX_DATA_REG_RST_VAL    0U

#define AXI_UART_STATUS_REG_OFFSET      12
#define AXI_UART_TX_START_BIT_OFFSET    0U
#define AXI_UART_TX_START_COMPLETE      (1U << AXI_UART_TX_START_BIT_OFFSET)
#define AXI_UART_TX_DATA_BIT_OFFSET     4U
#define AXI_UART_TX_DATA_COMPLETE       (1U << AXI_UART_TX_DATA_BIT_OFFSET)
#define AXI_UART_TX_STOP_BIT_OFFSET     8U
#define AXI_UART_TX_STOP_COMPLETE       (1U << AXI_UART_TX_STOP_BIT_OFFSET)
#define AXI_UART_TX_TRNS_COMPLETE       (AXI_UART_TX_START_COMPLETE | AXI_UART_TX_DATA_COMPLETE | AXI_UART_TX_STOP_COMPLETE)

typedef struct {
    volatile uint32_t control;
    volatile uint32_t baud_rate;
    volatile uint32_t tx_data;
    volatile uint32_t status;
    volatile uint32_t rx_data;
} axi_uart_regs;

//---------------------------------------------upper half---------------------------------------------//
static inline XStatus init_driver_(axi_uart_handle *p_handle, axi_uart_inition *init, _Bool do_init);

//---------------------------------------------lower half---------------------------------------------//
static inline void set_stop_bits(axi_uart_regs *p_reg_base, axi_uart_stop_bits stop_bits, _Bool do_setting);
//static inline void set_parity_type ((ps_uart_regs* )p_handle->id, init->parity_type, do_init);
static inline void set_data_bits(axi_uart_regs *p_reg_base, axi_uart_data_bits data_bits, _Bool do_setting);
static inline void set_baud_rate(axi_uart_regs *p_reg_base, uint32_t baud_rate, _Bool do_setting);
static inline void write_block_mode_data_(axi_uart_regs *p_uart_regs, char *p_data, size_t size);
static inline void set_tx_enable_(axi_uart_regs *p_reg_base, _Bool do_setting);
static inline void set_data_word_(axi_uart_regs *p_reg_base, char data);
static inline uint32_t get_status_(axi_uart_regs *p_reg_base);

#endif /* INC_AXI_UART_SYS__H */
