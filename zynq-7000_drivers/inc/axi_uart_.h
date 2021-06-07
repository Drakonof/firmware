#ifndef INC_AXI_UART__H
#define INC_AXI_UART__H
#include <stdbool.h>

#include "xil_types.h"
#include "xstatus.h"

typedef enum {
    stop_1,
    stop_2,
} axi_uart_stop_bits;

typedef enum {
    even,
    odd,
    none_parity
} axi_uart_parity_type;

typedef enum {
    data_8,
    data_7
} axi_uart_data_bits;

typedef struct {
    axi_uart_stop_bits   stop_bits;
    axi_uart_parity_type parity_type;
    axi_uart_data_bits   data_bits;
    uint32_t             baud_rate;
} axi_uart_inition;

typedef struct {
    volatile _Bool   ready;
    _Bool            do_unblocking_mode;
    uint32_t         id;
    volatile XStatus init;
} axi_uart_handle;

XStatus axi_uart_init(axi_uart_handle *p_handle, axi_uart_inition *init);
XStatus axi_uart_reinit(axi_uart_handle *p_handle, axi_uart_inition *init);
XStatus axi_uart_release(axi_uart_handle *p_handle);
XStatus axi_uart_write_data(axi_uart_handle *p_handle, char *p_data, size_t size);

#endif /* INC_AXI_UART__H */
