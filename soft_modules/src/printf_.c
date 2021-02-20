#include <stdarg.h>
#include <string.h> 
#include <stdio.h>

#include "printf_.h"

#ifdef XPAR_XUARTPS_NUM_INSTANCES

static status init_status = un_init_;
static ps_uart_handle handle = {};
static ps_uart_inition init_ = {};

status printf__custom_init(printf__inition init) {
	memset(&init_,0,sizeof(ps_uart_inition));
	memset(&handle,0,sizeof(ps_uart_handle));

	handle.id = init.id;

#if (TRUE == PRINTF_UNBLOCK_MODE_ENABLE)
	handle.do_unblocking_mode = true_;
#else
	handle.do_unblocking_mode = false_;
#endif

	init_.baud_rate = init.baud_rate;
	init_.channel_mode = normal;
	init_.data_bits = init.data_bits;
	init_.parity_type = init.parity_type;
	init_.stop_bits = init.stop_bits;

	init_status = uart_init(&handle,&init_);

	return init_status;
}

status printf__init(ps_uart_id usart_id) {
	memset(&init_,0,sizeof(ps_uart_inition));
    memset(&handle,0,sizeof(ps_uart_handle));

	init_.baud_rate = 115200;
	init_.channel_mode = normal;
	init_.data_bits = data_8;
	init_.parity_type = none_parity;
	init_.stop_bits = stop_1;

	handle.id = usart_id;

#if (TRUE == PRINTF_UNBLOCK_MODE_ENABLE)
	handle.do_unblocking_mode = true_;
#else
	handle.do_unblocking_mode = false_;
#endif

	init_status = uart_init(&handle,&init_);

	return init_status;
}

status putch_(char ch) {
	return uart_write_data(&handle, &ch, 1);
}

//todo: количесво символов для вывода, что делать если пустая строка
uint32_t printf_( char *str,...) {
	char result_str[100] = {}, arg_buff[100] = {};

    uint32_t i = 0;
    size_t size = 0;
	va_list argptr;

	if (null_ == str) {
	    return error_;
	}

	memset(result_str,0,100);

	va_start(argptr, str);

	while ('\0' != str[i]) {
	    if ('%' == str[i]) {
		   i++;

		   memset(arg_buff,0,100);

           switch (str[i++])
		   {
		   case 'c':
			   sprintf(arg_buff, "%c", va_arg(argptr,int));
		   break;
		   case 'd':
			   sprintf(arg_buff, "%d", va_arg(argptr,int));
		   break;
		   case 'f':
			   sprintf(arg_buff, "%f", va_arg(argptr,double));
		   break;
		   case 's':
			   strcpy(arg_buff, va_arg(argptr,const char *));
		   break;
		   case 'u':
			   sprintf(arg_buff, "%u", va_arg(argptr,unsigned int));
		   break;
		   case 'p':
			   sprintf(arg_buff, "%p", va_arg(argptr,void *));
		   break;
		   case '%':
			   strcpy(arg_buff, "%");
		   break;
		   case 'x':
			   sprintf(arg_buff, "%x", va_arg(argptr, int));
		   break;
		   case 'X':
		       sprintf(arg_buff, "%X", va_arg(argptr, int));
		   break;
		   case 'o':
			   sprintf(arg_buff, "%o", va_arg(argptr, int));
		   break;
		   default:
			   continue;
		   break;
		   }

           size += strlen(arg_buff);
           strcat(result_str, arg_buff);
		}
		else {
			result_str[size++] = str[i++];
		}
	}

    va_end(argptr);

    result_str[size++] = '\0';
	uart_write_data(&handle, result_str, size);
	return size;
}

#endif
