#include "xparameters.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xiicps.h"

#include "i2c.h"

XScuGic InterruptController;	/* Instance of the Interrupt Controller */

static status interrupt_init_(uint32_t id);
static status write_block_mode_(uint32_t id, uint8_t *p_data, size_t size, uint8_t address);
static status write_un_block_mode_(uint32_t id, uint8_t *p_data, size_t size, uint8_t address);

void interrupt_handler_(void *CallBackRef, u32 Event); // STATIC?

static XIicPs Iic[XPAR_XIICPS_NUM_INSTANCES];

status i2c_init(i2c_inition *p_init) {
	XIicPs_Config *cfg = NULL;

	if (NULL == p_init) {
		return error_;
	}

	p_init->init = un_init_;

	if (NULL == (cfg = XIicPs_LookupConfig(p_init->id))) {
		return error_;
	}

	if (XST_SUCCESS != XIicPs_CfgInitialize(&Iic[p_init->id],
			                                cfg,
											cfg->BaseAddress)) {
		return error_;
	}

	if (true_ == p_init->do_unblocking_mode) {
		if (ok_ != interrupt_init_(p_init->id)) {
			return XST_FAILURE;
		}

		XIicPs_SetStatusHandler(&Iic[p_init->id], (void *) &Iic[p_init->id], interrupt_handler_);
	}

	if (XST_SUCCESS != XIicPs_SetSClk(&Iic[p_init->id],
			                          p_init->sclk_rate)) {
		return error_;
	}

	p_init->init = ok_;

	return ok_;
}

status i2c_reinit(i2c_inition *p_init) {
	if (NULL == p_init) {
		return error_;
	}

	if (ok_ != p_init->init) {
		return p_init->init;
	}

	if (true_ == p_init->do_unblocking_mode) {
		if (ok_ != interrupt_init_(p_init->id)) {
			return XST_FAILURE;
		}
	}

	if (XST_SUCCESS != XIicPs_SetSClk(&Iic[p_init->id],
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


	if (NULL == p_handle) {
		return error_;
	}

	if (true_ == p_handle->do_ten_bit_address) {
		if (XST_SUCCESS != XIicPs_SetOptions(&Iic[p_handle->id],
				                             XIICPS_10_BIT_ADDR_OPTION)) {
			return error_;
		}
	}
	else {
		if (XST_SUCCESS != XIicPs_ClearOptions(&Iic[p_handle->id],
				                               XIICPS_10_BIT_ADDR_OPTION)) {
			return error_;
		}
	}

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
	}

	return ok_;
}

static status write_block_mode_(uint32_t id, uint8_t *p_data, size_t size, uint8_t address) {
	if (XST_SUCCESS != XIicPs_MasterSendPolled(&Iic[id], p_data, size, address)) {
		return error_;
	}

	while (XIicPs_BusIsBusy(&Iic[id])) {
		asm("NOP");
	}

	return ok_;
}

static status write_un_block_mode_(uint32_t id, uint8_t *p_data, size_t size, uint8_t address) {

	XIicPs_MasterSend(&Iic[id], p_data, size,
			address);

	return ok_;
}

status i2c_read(i2c_handle *p_handle) {
	if (NULL == p_handle) {
		return error_;
	}

	if (true_ == p_handle->do_ten_bit_address) {
		if (XST_SUCCESS != XIicPs_SetOptions(&Iic[p_handle->id],
				                             XIICPS_10_BIT_ADDR_OPTION)) {
			return error_;
		}
	}
	else {
		if (XST_SUCCESS != XIicPs_ClearOptions(&Iic[p_handle->id],
				                               XIICPS_10_BIT_ADDR_OPTION)) {
			return error_;
		}
	}

	if (XST_SUCCESS != XIicPs_MasterRecvPolled(&Iic[p_handle->id],
			                                   p_handle->rx_buffer,
											   p_handle->size,
											   p_handle->bus_address)) {
		return error_;
	}

   return ok_;
}

//todo:
status i2c_reset(i2c_handle *p_handle) {
	return ok_;
}

static status interrupt_init_(uint32_t id) {
	XScuGic_Config *interrupt_cfg = NULL;

	Xil_ExceptionInit();

	if (NULL == (interrupt_cfg = XScuGic_LookupConfig(XPAR_XIICPS_0_INTR))) {
		return error_;
	}

	if (XST_SUCCESS != XScuGic_CfgInitialize(&InterruptController,
			                                 interrupt_cfg,
											 interrupt_cfg->CpuBaseAddress)) {
		return error_;
	}

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
			                    (Xil_ExceptionHandler) XScuGic_InterruptHandler,
				                &InterruptController);

	if (XST_SUCCESS != XScuGic_Connect(&InterruptController,
			                           XIL_EXCEPTION_ID_IRQ_INT,
			                           (Xil_InterruptHandler)XIicPs_MasterInterruptHandler,
			                           (void *)&Iic[id])) {
		return error_;
	}

	XScuGic_Enable(&InterruptController, XIL_EXCEPTION_ID_IRQ_INT);

	Xil_ExceptionEnable();

	return ok_;
}

void interrupt_handler_(void *call_back, uint32_t event)
{
	if (0 != (event & XIICPS_EVENT_COMPLETE_RECV)){
	//	tx_complete = TRUE;
	} else if (0 != (event & XIICPS_EVENT_COMPLETE_SEND)) {
	//	rx_complete = TRUE;
	} else if (0 == (event & XIICPS_EVENT_SLAVE_RDY)){
	//	error_count++;
	}
}
