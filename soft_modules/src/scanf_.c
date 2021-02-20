#ifndef INC_SCANF__C
#define INC_SCANF__C

#include <string.h>

#include "scanf_.h"

#ifdef UART_HERE

static status init_status = un_init_;
static uart_handle handle = {};
static uart_inition init_ = {};

status scanf__custom_init(scanf__inition init) {
	memset(&init_,0,sizeof(uart_inition));
	memset(&handle,0,sizeof(uart_handle));

	handle.id = init.id;

	init_.baud_rate = init.baud_rate;
	init_.channel_mode = normal;
	init_.data_bits = init.data_bits;
	init_.parity_type = init.parity_type;
	init_.stop_bits = init.stop_bits;

	init_status = uart_init(&handle,&init_);

	return init_status;
}

status scanf__init(uint32_t usart_id) {
	memset(&init_,0,sizeof(uart_inition));
    memset(&handle,0,sizeof(uart_handle));

	init_.baud_rate = 115200;
	init_.channel_mode = normal;
	init_.data_bits = data_8;
	init_.parity_type = none_parity;
	init_.stop_bits = stop_1;

	handle.id = usart_id;

	init_status = uart_init(&handle,&init_);

	return init_status;
}

#endif
#endif /* INC_SCANF__C */
