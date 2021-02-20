#ifndef INC_UART__H
#define INC_UART__H

#include "xscugic.h"

#include "platform_.h"

#ifdef XPAR_XUARTPS_NUM_INSTANCES

//todo
typedef enum
{
	none_id,
//#ifdef XPAR_PS7_UART_0_DEVICE_ID
//    ps_usart_id_0 = XPAR_PS7_UART_0_BASEADDR,
//#endif
//#ifdef XPAR_AXI_UART_1_DEVICE_ID
    ps_usart_id_1 = XPAR_PS7_UART_1_BASEADDR,
//#endif
}  ps_uart_id;

typedef enum
{
    stop_1,
    stop_1d5,
    stop_2,
} ps_uart_stop_bits;

typedef enum
{
	even,
    odd,
    space,
    mark,
    none_parity
} ps_uart_parity_type;

typedef enum
{
	data_8,
	data_7 = 2,
	data_6
} ps_uart_data_bits;

typedef enum
{
    normal,
	echo,
	local_loopback,
	remote_loopback
} ps_uart_channel_mode;

typedef enum
{
	tx,
	rx,
	both
} ps_uart_path_reset;

typedef struct
{
	ps_uart_stop_bits    stop_bits;
	ps_uart_parity_type  parity_type;
	ps_uart_data_bits    data_bits;
	ps_uart_channel_mode channel_mode;
	uint32_t             baud_rate;       // 0 == disable baud,1 == clk/8, 2 == clk
} ps_uart_inition;

typedef struct
{
	volatile boolean     ready;
	boolean              do_unblocking_mode;
	ps_uart_id           id;
	volatile status      init;
} ps_uart_handle;

status uart_init(ps_uart_handle *p_handle, ps_uart_inition *init);
status uart_reinit(ps_uart_handle *p_handle, ps_uart_inition *init);
status uart_release(ps_uart_handle *p_handle);
status uart_read_data(ps_uart_handle *p_handle, char *p_data, size_t size); //todo:address
status uart_write_data(ps_uart_handle *p_handle, char *p_data, size_t size);
status uart_reset(ps_uart_handle *p_handle, ps_uart_path_reset path_reset, boolean *reset_done);

//status ps_uart_sleep(void);
//status ps_uart_waik(void);

#endif
#endif /* INC_UART__H */
