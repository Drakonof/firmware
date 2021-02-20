//todo: does need unblock mode?
#ifndef INC_PRINTF__H
#define INC_PRINTF__H

#include <ps_uart_.h>
#include "platform_.h"

#ifdef XPAR_XUARTPS_NUM_INSTANCES

typedef struct {
	ps_uart_stop_bits    stop_bits;
	ps_uart_parity_type  parity_type;
	ps_uart_data_bits    data_bits;
	uint32_t          baud_rate;
	uint32_t          id;
} printf__inition;

status printf__custom_init(printf__inition init);
status printf__init(ps_uart_id usart_id);
status putch_(char ch);
uint32_t printf_( char *str,...);

#endif

#endif /* INC_PRINTF__H */
