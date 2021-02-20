#ifndef INC_SCANF__H
#define INC_SCANF__H

#include <ps_uart_.h>
#include "platform_.h"

#ifdef UART_HERE

typedef struct {
	uart_stop_bits    stop_bits;
	uart_parity_type  parity_type;
	uart_data_bits    data_bits;
	uint32_t          baud_rate;
	uint32_t          id;
} scanf__inition;

status scanf__custom_init(scanf__inition init);
status scanf__init(uint32_t usart_id);
uint32_t scanf_( char *str,...);

#endif
#endif /* INC_SCANF__H */
