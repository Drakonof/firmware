//todo: mirror, repeat
#include "xparameters.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xiicps.h"

#include "i2c.h"

static XScuGic interrupt_cntr;
static XIicPs iic[XPAR_XIICPS_NUM_INSTANCES];

volatile boolean tx_complete[XPAR_XIICPS_NUM_INSTANCES],
                 rx_complete[XPAR_XIICPS_NUM_INSTANCES];

boolean is_master_mode[XPAR_XIICPS_NUM_INSTANCES];
boolean init[XPAR_XIICPS_NUM_INSTANCES];

volatile int8_t error_count[XPAR_XIICPS_NUM_INSTANCES];

static void interrupt_handler_(void *CallBackRef, u32 Event);

static status interrupt_init_(uint32_t id);
static status write_block_mode_(uint32_t id, uint8_t *p_data, size_t size, uint8_t address);
static status write_un_block_mode_(uint32_t id, uint8_t *p_data, size_t size, uint8_t address);
static status read_block_mode_(uint32_t id, uint8_t *p_data, size_t size, uint8_t address);
static status read_un_block_mode_(uint32_t id, uint8_t *p_data, size_t size, uint8_t address);

status i2c_init(i2c_inition *p_init) {
	XIicPs_Config *cfg = NULL;

	if (NULL == p_init) {
		return error_;
	}

	p_init->init = un_init_;

	if (NULL == (cfg = XIicPs_LookupConfig(p_init->id))) {
		return error_;
	}

	if (XST_SUCCESS != XIicPs_CfgInitialize(&iic[p_init->id],
			                                cfg,
											cfg->BaseAddress)) {
		return error_;
	}

	if (true_ == p_init->do_unblocking_mode) {
		if (ok_ != interrupt_init_(p_init->id)) {
			return error_;
		}
	}

    is_master_mode[p_init->id] = p_init->do_master;

	if (true_ != is_master_mode[p_init->id]) {
		XIicPs_SetupSlave(&iic[p_init->id], p_init->self_address);
	}

	if (XST_SUCCESS != XIicPs_SetSClk(&iic[p_init->id],
			                          p_init->sclk_rate)) {
		return error_;
	}


	if (true_ == p_init->do_ten_bit_address) {
		if (XST_SUCCESS != XIicPs_SetOptions(&iic[p_init->id],
				                             XIICPS_10_BIT_ADDR_OPTION)) {
			return error_;
		}
	}
	else {
		if (XST_SUCCESS != XIicPs_ClearOptions(&iic[p_init->id],
				                               XIICPS_10_BIT_ADDR_OPTION)) {
			return error_;
		}
	}

	p_init->init = ok_;
	init[p_init->id] = true_;

	return ok_;
}

status i2c_reinit(i2c_inition *p_init) {
	if (NULL == p_init) {
		return error_;
	}

	if (true_ != init[p_init->id]) {
		return init[p_init->id];
	}

	if (true_ == p_init->do_unblocking_mode) {
		if (ok_ != interrupt_init_(p_init->id)) {
			return XST_FAILURE;
		}
	}

	is_master_mode[p_init->id] = p_init->do_master;

	if (true_ != is_master_mode[p_init->id]) {
		XIicPs_SetupSlave(&iic[p_init->id], p_init->self_address);
	}

	if (XST_SUCCESS != XIicPs_SetSClk(&iic[p_init->id],
			                          p_init->sclk_rate)) {
		return error_;
	}

    return ok_;
}

//todo:
status i2c_release(i2c_handle *p_handle) {
	return ok_;
}

status i2c_write(i2c_handle *p_handle) {
	if (true_ != init[p_handle->id]) {
		return init[p_handle->id];
	}

	if (NULL == p_handle) {
		return error_;
	}

	tx_complete[p_handle->id] = false_;
	error_count[p_handle->id] = 0;

	if (true_ == p_handle->do_unblocking_mode) {
		if (ok_ != write_un_block_mode_(p_handle->id,
										p_handle->tx_buffer,
										p_handle->size,
										p_handle->bus_address)) {
			return error_;
		}
	}
	else {
		if (ok_ != write_block_mode_(p_handle->id,
								     p_handle->tx_buffer,
								     p_handle->size,
									 p_handle->bus_address)) {
			return error_;
		}

		tx_complete[p_handle->id] = true_;
	}

	return ok_;
}

status i2c_read(i2c_handle *p_handle) {
	if (true_ != init[p_handle->id]) {
		return init[p_handle->id];
	}

	if (NULL == p_handle) {
		return error_;
	}

	rx_complete[p_handle->id] = false_;
	error_count[p_handle->id] = 0;

	if (true_ == p_handle->do_unblocking_mode) {
		if (ok_ != read_un_block_mode_(p_handle->id,
									   p_handle->rx_buffer,
									   p_handle->size,
									   p_handle->bus_address)) {
			return error_;
		}
	}
	else {
		if (ok_ != read_block_mode_(p_handle->id,
								    p_handle->rx_buffer,
									p_handle->size,
									p_handle->bus_address)) {
			return error_;
		}

		rx_complete[p_handle->id] = true_;
	}

   return ok_;
}

//todo:
status i2c_reset(i2c_handle *p_handle) {
	if (true_ != init[p_handle->id]) {
		return init[p_handle->id];
	}

	if (NULL == p_handle) {
		return error_;
	}


	return ok_;
}

boolean i2c_get_ready(i2c_handle *p_handle, boolean is_tx_complete) {
	if (true_ != init[p_handle->id]) {
		return init[p_handle->id];
	}

	if (NULL == p_handle) {
		return error_;
	}

	if (true_ == is_tx_complete) {
		return tx_complete[p_handle->id];
	}
	else {
		return rx_complete[p_handle->id];
	}

	p_handle->errors = error_count[p_handle->id];
}

static status write_block_mode_(uint32_t id, uint8_t *p_data, size_t size, uint8_t address) {
	while (TRUE == XIicPs_BusIsBusy(&iic[id])) {
		asm("NOP");
	}

	if (true_ == is_master_mode[id]) {
	    if (XST_SUCCESS != XIicPs_MasterSendPolled(&iic[id], p_data, size, address)) {
		    return error_;
	    }
	}
	else {
	    if (XST_SUCCESS != XIicPs_SlaveSendPolled(&iic[id], p_data, size)) {
	        return error_;
		}
	}

	return ok_;
}

static status write_un_block_mode_(uint32_t id, uint8_t *p_data, size_t size, uint8_t address) {
	if (true_ == is_master_mode[id]) {
	    XIicPs_MasterSend(&iic[id], p_data, size, address);
	}
	else {
		XIicPs_SlaveSend(&iic[id], p_data, size);
	}

	return ok_;
}

static status read_block_mode_(uint32_t id, uint8_t *p_data, size_t size, uint8_t address) {
	while (TRUE == XIicPs_BusIsBusy(&iic[id])) {
		asm("NOP");
	}

	if (true_ == is_master_mode[id]) {
	    if (XST_SUCCESS != XIicPs_MasterRecvPolled(&iic[id],
			                                       p_data,
											       size,
											       address)) {
		    return error_;
	     }
	}
	else {
		if (XST_SUCCESS != XIicPs_SlaveRecvPolled(&iic[id], p_data, size)) {
			return error_;
		}
	}

	return ok_;
}

static status read_un_block_mode_(uint32_t id, uint8_t *p_data, size_t size, uint8_t address) {
	if (true_ == is_master_mode[id]) {
	    XIicPs_MasterRecv(&iic[id], p_data, size, address);
	}
	else {
		XIicPs_SlaveRecv(&iic[id], p_data, size);
	}

	return ok_;
}

static status interrupt_init_(uint32_t id) {
	XScuGic_Config *interrupt_cfg = NULL;
	uint32_t *id_ = NULL;
    Xil_InterruptHandler interrupt_handler = NULL;

    if (true_ == is_master_mode[id]) {
    	interrupt_handler = (Xil_InterruptHandler) XIicPs_MasterInterruptHandler;
    }
    else {
    	interrupt_handler =  (Xil_InterruptHandler) XIicPs_SlaveInterruptHandler;
    }

	*id_ = id;

	Xil_ExceptionInit();

	if (NULL == (interrupt_cfg = XScuGic_LookupConfig(XPAR_SCUGIC_SINGLE_DEVICE_ID))) {
		return error_;
	}

	if (XST_SUCCESS != XScuGic_CfgInitialize(&interrupt_cntr,
			                                 interrupt_cfg,
											 interrupt_cfg->CpuBaseAddress)) {
		return error_;
	}

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
			                    (Xil_ExceptionHandler) XScuGic_InterruptHandler,
				                &interrupt_cntr);

	if (XST_SUCCESS != XScuGic_Connect(&interrupt_cntr,
			                           XPAR_XIICPS_0_INTR,
			                           interrupt_handler,
			                           (void *)&iic[id])) {
		return error_;
	}

	XIicPs_SetStatusHandler(&iic[id], (void *) id_, &interrupt_handler_);

	XScuGic_Enable(&interrupt_cntr, XPAR_XIICPS_0_INTR);

	Xil_ExceptionEnable();

	return ok_;
}

static void interrupt_handler_(void *call_back, uint32_t event) {
	uint32_t id = 0;

	id = *(uint32_t *)call_back;

	if (0 != (event & XIICPS_EVENT_COMPLETE_RECV)){
		rx_complete[id] = true_;
	}
	else if (0 != (event & XIICPS_EVENT_COMPLETE_SEND)) {
		tx_complete[id] = true_;
	}
	else if (0 == (event & XIICPS_EVENT_SLAVE_RDY)){
		error_count[id]++;
	}
}
