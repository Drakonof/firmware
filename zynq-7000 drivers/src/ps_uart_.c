//todo: 1. realize all uart features
//      2. infer interrupts
//      3. XUARTPS_CR_OFFSET(0): XUARTPS_CR_STOPBRK, XUARTPS_CR_STARTBRK, XUARTPS_CR_TORST
//      4. add _ all static
#include <ps_uart_.h>
#include <ps_uart_sys_.h>

static uint32_t ii = 0;

//todo: do it not hardcore
char *un_block_array[2];
boolean tx_ready[2] = {false_, false_};
uint32_t num[2];

//---------------------------------------------upper half---------------------------------------------//

status uart_init(ps_uart_handle *p_handle, ps_uart_inition *init) {
	return init_driver_(p_handle, init, true_);
}

status uart_re_init(ps_uart_handle *p_handle, ps_uart_inition *init) {
	M_user_return_if(ok_ != p_handle->init, p_handle->init);

	return init_driver_(p_handle, init, true_);
}

status uart_release(ps_uart_handle *p_handle) {
	M_user_return_if(ok_ != p_handle->init, p_handle->init);

	ps_uart_inition init = {};

	return init_driver_(p_handle, &init, false_);
}

status uart_read_data(ps_uart_handle *p_handle, char *p_data, size_t size) {

	ps_uart_regs *p_uart_regs = null_;

	M_status_return_if((null_ == p_handle) || (null_ == p_data) || (0 == size));
	M_user_return_if(ok_ != p_handle->init, p_handle->init);

	p_handle->ready = false_;
	p_uart_regs = (ps_uart_regs* )p_handle->id;

	if (true_ == p_handle->do_unblocking_mode) {
		read_un_block_mode_data_();
	}
	else {
		read_block_mode_data_(p_uart_regs, p_data, size);
		p_handle->ready = true_;
	}

	return ok_;
}

status uart_write_data(ps_uart_handle *p_handle, char *p_data, size_t size) {

	ps_uart_regs *p_uart_regs = null_;

	M_status_return_if((null_ == p_handle) || (null_ == p_data) || (0 == size));
	M_user_return_if(ok_ != p_handle->init, p_handle->init);

	p_handle->ready = false_;
	p_uart_regs = (ps_uart_regs* )p_handle->id;

	if (true_ == p_handle->do_unblocking_mode) {
		write_un_block_mode_data_(p_uart_regs, p_data, size);
	}
	else {
		write_block_mode_data_(p_uart_regs, p_data, size);
		p_handle->ready = true_;
	}

	return ok_;
}

// не все поля uart_handle используются
status uart_reset(ps_uart_handle *p_handle, ps_uart_path_reset path_reset, boolean *reset_done) {
	M_user_return_if(ok_ != p_handle->init, p_handle->init);

	set_reset_((ps_uart_regs* )p_handle->id, path_reset, reset_done);
	return ok_;
}

static inline status init_driver_(ps_uart_handle *p_handle,
		                          ps_uart_inition *init,
								  boolean do_init) {
	boolean reset_done = false_;

	const uint32_t rx_fifo_level = 10;

	p_handle->ready = false_;
	p_handle->init  = un_init_;

	M_status_return_if((null_ == p_handle) || (null_ == init));
	M_status_return_if(none_id == p_handle->id);
	M_user_return_assert_if(ok_ != check_params_(init->baud_rate), p_handle->init);

	//todo: reset done??
	set_reset_((ps_uart_regs* )p_handle->id, both, &reset_done);

	set_tx_enable_((ps_uart_regs* )p_handle->id, false_);

	set_stop_bits   ((ps_uart_regs* )p_handle->id, init->stop_bits, do_init);
	set_parity_type ((ps_uart_regs* )p_handle->id, init->parity_type, do_init);
	set_data_bits   ((ps_uart_regs* )p_handle->id, init->data_bits, do_init);
	set_channel_mode_((ps_uart_regs* )p_handle->id, init->channel_mode, do_init);
	set_baud_rate   ((ps_uart_regs* )p_handle->id, init->baud_rate, do_init);
	set_rx_fifo_level ((ps_uart_regs* )p_handle->id, rx_fifo_level, do_init);

	if (true_ == do_init) {
		//set_enable_((ps_uart_regs* )p_handle->id, true_);
		p_handle->init = ok_;
		p_handle->ready = true_;
/*--------------------------------------------------------------------------------------*/
		XScuGic InterruptController;

		XScuGic_Config *GicConfig;

		GicConfig = XScuGic_LookupConfig(XPAR_PS7_UART_1_DEVICE_ID);
		XScuGic_CfgInitialize(&InterruptController, GicConfig,
							GicConfig->CpuBaseAddress);
		Xil_ExceptionInit();
		Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
					(Xil_ExceptionHandler) XScuGic_InterruptHandler,
					&InterruptController);
		Xil_ExceptionEnable();
		XScuGic_Connect(&InterruptController, 82,
					(Xil_ExceptionHandler) intrrupt_handler, (void *)&InterruptController);

		XScuGic_SetPriorityTriggerType(&InterruptController, 82,
								8, 0b11);
		XScuGic_InterruptMaptoCpu(&InterruptController, 0, 82);
		XScuGic_Enable(&InterruptController, 82);
/*--------------------------------------------------------------------------------------*/
	}

	return ok_;
}

static inline status check_params_(uint32_t baud_rate) {
	const uint32_t highest_setup_value = 2,
		           lowest_baud_rate_value = 300;

    M_status_return_if((0 == baud_rate) || ((highest_setup_value < baud_rate) &&
    	               (lowest_baud_rate_value > baud_rate)));

    return ok_;
}

static inline void read_un_block_mode_data_() {



}

static inline void read_block_mode_data_(ps_uart_regs *p_uart_regs, char *p_data, size_t size) {
	uint32_t i = 0;

	for (i = 0; i < size; i++) {
		p_data[i] = get_data_word_(p_uart_regs);
	}
}

static inline void write_un_block_mode_data_(ps_uart_regs *p_uart_regs, char *p_data, size_t size) {
    uint32_t i = 0;
    const uint32_t interrupt_mask = 0x1FF;

    //todo:hardcode: 0
    un_block_array[0] = p_data;
    num[0] = size;
    ii = 1;

    set_interrupt_(p_uart_regs, interrupt_mask);

    for(i = 0; i < size; i++) {
    	//while (tx_ready[0]) {
    	   set_data_word_(p_uart_regs , p_data[i]);
    	//}

	}

    //set_tx_enable_(p_uart_regs, true_);

   // set_data_word_(p_uart_regs, p_data[i]);
}

static inline void write_block_mode_data_(ps_uart_regs *p_uart_regs, char *p_data, size_t size) {
	uint32_t i = 0;

	//todo: only tx
	set_tx_enable_(p_uart_regs, true_);

	for(i = 0; i < size; i++) {
		set_data_word_(p_uart_regs , p_data[i]);
	}
}

//---------------------------------------------lower half---------------------------------------------//

static inline void set_stop_bits(ps_uart_regs *p_reg_base, ps_uart_stop_bits stop_bits, boolean do_setting) {

	p_reg_base->mode &= ~((TRUE << PS_UART_STOP_NUM_BIT_0_OFFSET) | (TRUE << PS_UART_STOP_NUM_BIT_1_OFFSET));

	if (true_ == do_setting) {
		p_reg_base->mode |= (stop_bits << PS_UART_STOP_NUM_BIT_0_OFFSET);
	}
	else {
		p_reg_base->mode |= (PS_UART_MODE_REG_RESET_VALUE & PS_UART_STOP_NUM_BIT_MASK);
	}
}

static inline void set_parity_type(ps_uart_regs *p_reg_base, ps_uart_parity_type parity_type, boolean do_setting) {
	p_reg_base->mode &= ~((TRUE << PS_UART_PAR_TYP_BIT_0_OFFSET) | (TRUE << PS_UART_PAR_TYP_BIT_1_OFFSET));

	if (true_ == do_setting) {
		p_reg_base->mode |= (parity_type << PS_UART_PAR_TYP_BIT_0_OFFSET);
	}
	else {
		p_reg_base->mode |= (PS_UART_MODE_REG_RESET_VALUE & PS_UART_PAR_TYP_BIT_MASK);
	}
}

static inline void set_data_bits(ps_uart_regs *p_reg_base, ps_uart_data_bits data_bits, boolean do_setting) {
	p_reg_base->mode &= ~((TRUE << PS_UART_DATA_NUM_BIT_0_OFFSET) | (TRUE << PS_UART_DATA_NUM_BIT_1_OFFSET));

	if (true_ == do_setting) {
		p_reg_base->mode |= (data_bits << PS_UART_DATA_NUM_BIT_0_OFFSET);
	}
	else {
		p_reg_base->mode |= (PS_UART_MODE_REG_RESET_VALUE & PS_UART_DATA_NUM_BIT_MASK);
	}
}

static inline void set_channel_mode_(ps_uart_regs *p_reg_base, ps_uart_channel_mode channel_mode, boolean do_setting) {
	p_reg_base->mode &= ~((TRUE << PS_UART_CHNL_MD_BIT_0_OFFSET) | (TRUE << PS_UART_CHNL_MD_BIT_1_OFFSET));

	if (true_ == do_setting) {
		p_reg_base->mode |= (channel_mode << PS_UART_CHNL_MD_BIT_0_OFFSET);
	}
	else {
		p_reg_base->mode |= (PS_UART_MODE_REG_RESET_VALUE & PS_UART_CHNL_MD_BIT_MASK);
	}
}

//XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ
static inline void set_baud_rate(ps_uart_regs *p_reg_base, uint32_t baud_rate, boolean do_setting) {
	p_reg_base->mode &= ~(TRUE << PS_UART_CLK_SELECT_BIT_0_OFFSET);
	p_reg_base->baud_rate_divider = 0;

	if (true_ == do_setting) {
		if (PS_UART_BD_GN_REG_DSABLE_VALUE == baud_rate) {
			p_reg_base->baud_rate_generator = PS_UART_BD_GN_REG_DSABLE_VALUE;
			p_reg_base->mode |= (PS_UART_MODE_REG_RESET_VALUE & PS_UART_CLK_SELECT_BIT_MASK);
			p_reg_base->baud_rate_divider = PS_UART_BD_RT_REG_RESET_VALUE;
		}
		else if (PS_UART_CLK_SELECT_DIV8_VALUE == baud_rate){
			p_reg_base->baud_rate_generator = PS_UART_BD_GN_REG_BPASS_VALUE;
			p_reg_base->mode |= (PS_UART_CLK_SELECT_DIV8_VALUE << PS_UART_CLK_SELECT_BIT_0_OFFSET);
			p_reg_base->baud_rate_divider = PS_UART_BD_RT_REG_RESET_VALUE;
		}
		else {
			p_reg_base->baud_rate_generator = PS_UART_BD_GN_REG_DEF_VALUE;
			p_reg_base->mode |= (PS_UART_CLK_SELECT_NONDIV8_VALUE << PS_UART_CLK_SELECT_BIT_0_OFFSET);
			p_reg_base->baud_rate_divider = (XPAR_XUARTPS_0_UART_CLK_FREQ_HZ  / (baud_rate * PS_UART_BD_GN_REG_DEF_VALUE)) - 1;
		}
	}
	else {
		p_reg_base->baud_rate_generator = PS_UART_BD_GN_REG_RESET_VALUE;
		p_reg_base->mode |= (PS_UART_MODE_REG_RESET_VALUE & PS_UART_CLK_SELECT_BIT_MASK);
		p_reg_base->baud_rate_divider = PS_UART_BD_RT_REG_RESET_VALUE;
	}
}

static inline char get_data_word_(ps_uart_regs *p_reg_base) {
	while((p_reg_base->channel_status & 0x2) == 0x2); // todo: mask
	return p_reg_base->tx_rx_fifo;
}

static inline void set_data_word_(ps_uart_regs *p_reg_base, char data) {
	while((p_reg_base->channel_status & 0x8) != 0x8);
	p_reg_base->tx_rx_fifo = data;
}

static inline void set_tx_enable_(ps_uart_regs *p_reg_base, boolean do_setting) {
	if (true_ == do_setting) {
		p_reg_base->control |= (TRUE << PS_UART_TX_ENABLE_BIT_OFFSET) | (TRUE << PS_UART_RX_ENABLE_BIT_OFFSET);
	}
	else {
		//p_reg_base->control |= (TRUE << PS_UART_TX_DISABLE_BIT_OFFSET) | (TRUE << PS_UART_RX_DISABLE_BIT_OFFSET);
	}
}

static inline void set_reset_(ps_uart_regs *p_reg_base, ps_uart_path_reset path_reset, boolean *reset_done) {
	switch (path_reset) {
	case tx:
	    p_reg_base->control |= (TRUE << PS_UART_TX_RESET_BIT_OFFSET);
	    *reset_done = !(p_reg_base->control & (TRUE << PS_UART_TX_RESET_BIT_OFFSET));
	break;
	case rx:
		p_reg_base->control |= (TRUE << PS_UART_RX_RESET_BIT_OFFSET);
		*reset_done = !(p_reg_base->control & (TRUE << PS_UART_RX_RESET_BIT_OFFSET));
	break;
	case both:
		p_reg_base->control |= (TRUE << PS_UART_TX_RESET_BIT_OFFSET) | (TRUE << PS_UART_RX_RESET_BIT_OFFSET);
		*reset_done = !(p_reg_base->control & ((TRUE << PS_UART_TX_RESET_BIT_OFFSET) | (TRUE << PS_UART_RX_RESET_BIT_OFFSET)));
	break;
	default:
		*reset_done = 0;
    break;
	}
}

//todo: навести порядок
static inline void set_interrupt_(ps_uart_regs *p_uart_regs, uint32_t interrupt_mask) {
	p_uart_regs->interrupt_enable = 0x1FF & interrupt_mask;
}


static inline void set_rx_fifo_level(ps_uart_regs *p_reg_base, uint8_t rx_fifo_level, boolean do_setting)
{
	if (true_ == do_setting) {
		p_reg_base->rx_fifo_level = rx_fifo_level;
	}
	else {
		p_reg_base->rx_fifo_level = PS_UART_RX_FIFO_LVL_REG_RESET_VALUE;
	}
}

//---------------------------------------------interrupts---------------------------------------------//

static void intrrupt_handler(void *callback)
{
	ps_uart_regs *uart_regs = (ps_uart_regs *)ps_usart_id_1;


	if (uart_regs->interrupt_status & PS_UART_RX_FIFO_TRIG_INTR_MSK) {
		asm("nop");
	}

	if (uart_regs->interrupt_status & PS_UART_RX_FIFO_EMPTY_INTR_MSK) {
		asm("nop");
	}

	if (uart_regs->interrupt_status & PS_UART_RX_FIFO_FULL_INTR_MSK) {
		asm("nop");
	}

	if (uart_regs->interrupt_status & PS_UART_TX_FIFO_EMPTY_INTR_MSK) {
		//todo:hardcode: 0
		//tx_ready[0] = true_;

		uart_regs->interrupt_enable = 0x1FF;

/*		// todo: hardcode: 0x1FF
		uart_regs->interrupt_disable = 0x1FF;

		if (num[0] > ii) {
			// todo: hardcode: 0
			set_data_word_(uart_regs, un_block_array[0][ii]);
			ii++;
			// todo: hardcode: 0x1FF
			uart_regs->interrupt_enable = 0x1FF;
		}*/
	}

	if (uart_regs->interrupt_status & PS_UART_TX_FIFO_FULL_INTR_MSK) {
		asm("nop");
	}

	if (uart_regs->interrupt_status & PS_UART_RX_OVERFLOW_INTR_MSK) {
		asm("nop");
	}

	if (uart_regs->interrupt_status & PS_UART_RX_FRME_ERROR_INTR_MSK) {
		asm("nop");
	}

	if (uart_regs->interrupt_status & PS_UART_RX_PART_ERROR_INTR_MSK) {
		asm("nop");
	}

	if (uart_regs->interrupt_status & PS_UART_RX_TMOUT_ERROR_INTR_MSK) {
	    asm("nop");
	}

	if (uart_regs->interrupt_status & PS_UART_MD_STATUS_INDR_INTR_MSK) {
	    asm("nop");
	}

	if (uart_regs->interrupt_status & PS_UART_TX_FIFO_TRIG_INTR_MSK) {
	    asm("nop");
	}

	if (uart_regs->interrupt_status & PS_UART_TX_FIFO_NRFULL_INTR_MSK) {
	//	tx_ready[0] = false_;
		set_tx_enable_(uart_regs, true_);
	}

	if (uart_regs->interrupt_status & PS_UART_TX_OVERFLOW_INTR_MSK) {
		asm("nop");
	}
}
