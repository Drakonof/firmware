//todo: mirror, repeat
//todo: get wave out, RAM, right am/pm, порядок и оптимизация
#include <stdbool.h>

#include "i2c_.h"

static XScuGic interrupt_cntr;
static XIicPs iic[XPAR_XIICPS_NUM_INSTANCES];

volatile _Bool tx_complete[XPAR_XIICPS_NUM_INSTANCES],
                 rx_complete[XPAR_XIICPS_NUM_INSTANCES];

_Bool is_master_mode[XPAR_XIICPS_NUM_INSTANCES];
_Bool init[XPAR_XIICPS_NUM_INSTANCES];

volatile int8_t error_count[XPAR_XIICPS_NUM_INSTANCES];

static void interrupt_handler_(void *CallBackRef, u32 Event);

static XStatus interrupt_init_(uint32_t id);
static XStatus write_block_mode_(uint32_t id, uint8_t *p_data, size_t size, uint8_t address);
static XStatus write_un_block_mode_(uint32_t id, uint8_t *p_data, size_t size, uint8_t address);
static XStatus read_block_mode_(uint32_t id, uint8_t *p_data, size_t size, uint8_t address);
static XStatus read_un_block_mode_(uint32_t id, uint8_t *p_data, size_t size, uint8_t address);

XStatus i2c_init(i2c_inition *p_init) {
    XIicPs_Config *cfg = NULL;

    if (NULL == p_init) {
        return XST_FAILURE;
    }

    p_init->init = XST_FAILURE;

    if (NULL == (cfg = XIicPs_LookupConfig(p_init->id))) {
        return XST_FAILURE;
    }

	if (XST_SUCCESS != XIicPs_CfgInitialize(&iic[p_init->id],
			                                cfg,
											cfg->BaseAddress)) {
		return XST_FAILURE;
	}

    if (true == p_init->do_unblocking_mode) {
        if (XST_SUCCESS != interrupt_init_(p_init->id)) {
            return XST_FAILURE;
        }
    }

    is_master_mode[p_init->id] = p_init->do_master;

    if (true != is_master_mode[p_init->id]) {
        XIicPs_SetupSlave(&iic[p_init->id], p_init->self_address);
    }

    if (XST_SUCCESS != XIicPs_SetSClk(&iic[p_init->id], p_init->sclk_rate)) {
        return XST_FAILURE;
    }

    if (true == p_init->do_ten_bit_address) {
        if (XST_SUCCESS != XIicPs_SetOptions(&iic[p_init->id],
                                             XIICPS_10_BIT_ADDR_OPTION)) {
            return XST_FAILURE;
        }
    }
    else {
        if (XST_SUCCESS != XIicPs_ClearOptions(&iic[p_init->id],
                                               XIICPS_10_BIT_ADDR_OPTION)) {
            return XST_FAILURE;
        }
    }

    p_init->init = XST_SUCCESS;
    init[p_init->id] = true;

    return XST_SUCCESS;
}

XStatus i2c_reinit(i2c_inition *p_init) {
    if (NULL == p_init) {
        return XST_FAILURE;
    }

    if (true != init[p_init->id]) {
        return init[p_init->id];
    }

    if (true == p_init->do_unblocking_mode) {
        if (XST_SUCCESS != interrupt_init_(p_init->id)) {
            return XST_FAILURE;
        }
    }

    is_master_mode[p_init->id] = p_init->do_master;

    if (true != is_master_mode[p_init->id]) {
        XIicPs_SetupSlave(&iic[p_init->id], p_init->self_address);
    }

    if (XST_SUCCESS != XIicPs_SetSClk(&iic[p_init->id], p_init->sclk_rate)) {
        return XST_FAILURE;
    }

    return XST_SUCCESS;
}

//todo:
XStatus i2c_release(i2c_handler *p_handler) {
    return XST_SUCCESS;
}

XStatus i2c_write(i2c_handler *p_handler) {
    if (true != init[p_handler->id]) {
        return init[p_handler->id];
    }

    if (NULL == p_handler) {
        return XST_FAILURE;
    }

    tx_complete[p_handler->id] = false;
    error_count[p_handler->id] = 0;

    if (true == p_handler->do_unblocking_mode) {
        if (XST_SUCCESS != write_un_block_mode_(p_handler->id,
                                        p_handler->tx_buffer,
                                        p_handler->size,
                                        p_handler->bus_address)) {
            return XST_FAILURE;
        }
    }
    else {
        if (XST_SUCCESS != write_block_mode_(p_handler->id,
                                     p_handler->tx_buffer,
                                     p_handler->size,
                                     p_handler->bus_address)) {
            return XST_FAILURE;
        }

        tx_complete[p_handler->id] = true;
    }

    return XST_SUCCESS;
}

XStatus i2c_read(i2c_handler *p_handler) {
    if (true != init[p_handler->id]) {
        return init[p_handler->id];
    }

    if (NULL == p_handler) {
        return XST_FAILURE;
    }

    rx_complete[p_handler->id] = false;
    error_count[p_handler->id] = 0;

    if (true == p_handler->do_unblocking_mode) {
        if (XST_SUCCESS != read_un_block_mode_(p_handler->id,
                                       p_handler->rx_buffer,
                                       p_handler->size,
                                       p_handler->bus_address)) {
            return XST_FAILURE;
        }
    }
    else {
        if (XST_SUCCESS != read_block_mode_(p_handler->id,
                                    p_handler->rx_buffer,
                                    p_handler->size,
                                    p_handler->bus_address)) {
            return XST_FAILURE;
        }

        rx_complete[p_handler->id] = true;
    }

    return XST_SUCCESS;
}

//errors??
_Bool i2c_get_ready(i2c_handler *p_handler, i2c_ready_flags ready_flag) {
    if (true != init[p_handler->id]) {
        return init[p_handler->id];
    }

    if (NULL == p_handler) {
        return false;
    }

    p_handler->errors = error_count[p_handler->id];

    if (tx_ready_flag == ready_flag) {
        return tx_complete[p_handler->id];
    }
    else {
        return rx_complete[p_handler->id];
    }
}

//todo:
XStatus i2c_reset(i2c_handler *p_handler) {
    if (true != init[p_handler->id]) {
        return init[p_handler->id];
    }

    if (NULL == p_handler) {
        return XST_FAILURE;
    }

    return XST_SUCCESS;
}

static XStatus interrupt_init_(uint32_t id) {
    uint32_t *id_ = NULL;
    XScuGic_Config *interrupt_cfg = NULL;
    Xil_InterruptHandler interrupt_handler = NULL;

    if (true == is_master_mode[id]) {
        interrupt_handler = (Xil_InterruptHandler) XIicPs_MasterInterruptHandler;
    }
    else {
        interrupt_handler =  (Xil_InterruptHandler) XIicPs_SlaveInterruptHandler;
    }

    *id_ = id;

    Xil_ExceptionInit();

    if (NULL == (interrupt_cfg = XScuGic_LookupConfig(XPAR_SCUGIC_SINGLE_DEVICE_ID))) {
        return XST_FAILURE;
    }

    if (XST_SUCCESS != XScuGic_CfgInitialize(&interrupt_cntr,
                                             interrupt_cfg,
                                             interrupt_cfg->CpuBaseAddress)) {
        return XST_FAILURE;
    }

    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
                                (Xil_ExceptionHandler) XScuGic_InterruptHandler,
                                &interrupt_cntr);

    if (XST_SUCCESS != XScuGic_Connect(&interrupt_cntr,
                                       XPAR_XIICPS_0_INTR,
                                       interrupt_handler,
                                       (void *)&iic[id])) {
        return XST_FAILURE;
    }

    XIicPs_SetStatusHandler(&iic[id], (void *) id_, &interrupt_handler_);

    XScuGic_Enable(&interrupt_cntr, XPAR_XIICPS_0_INTR);

    Xil_ExceptionEnable();

    return XST_SUCCESS;
}

static XStatus write_un_block_mode_(uint32_t id, uint8_t *p_data, size_t size, uint8_t address) {
    if (true == is_master_mode[id]) {
        XIicPs_MasterSend(&iic[id], p_data, size, address);
    }
    else {
        XIicPs_SlaveSend(&iic[id], p_data, size);
    }

    return XST_SUCCESS;
}

static XStatus write_block_mode_(uint32_t id, uint8_t *p_data, size_t size, uint8_t address) {
    while (TRUE == XIicPs_BusIsBusy(&iic[id])) {
        asm("NOP");
    }

    if (true == is_master_mode[id]) {
        if (XST_SUCCESS != XIicPs_MasterSendPolled(&iic[id], p_data, size, address)) {
            return XST_FAILURE;
        }
    }
    else {
        if (XST_SUCCESS != XIicPs_SlaveSendPolled(&iic[id], p_data, size)) {
            return XST_FAILURE;
        }
    }

    return XST_SUCCESS;
}

static XStatus read_un_block_mode_(uint32_t id, uint8_t *p_data, size_t size, uint8_t address) {
    if (true == is_master_mode[id]) {
        XIicPs_MasterRecv(&iic[id], p_data, size, address);
    }
    else {
        XIicPs_SlaveRecv(&iic[id], p_data, size);
    }

    return XST_SUCCESS;
}

static XStatus read_block_mode_(uint32_t id, uint8_t *p_data, size_t size, uint8_t address) {
    while (TRUE == XIicPs_BusIsBusy(&iic[id])) {
        asm("NOP");
    }

    if (true  == is_master_mode[id]) {
        if (XST_SUCCESS != XIicPs_MasterRecvPolled(&iic[id],
                                                   p_data,
                                                   size,
                                                   address)) {
            return XST_FAILURE;
        }
    }
    else {
        if (XST_SUCCESS != XIicPs_SlaveRecvPolled(&iic[id], p_data, size)) {
            return XST_FAILURE;
        }
    }

	return XST_SUCCESS;
}

static void interrupt_handler_(void *call_back, uint32_t event) {
    uint32_t id = 0;

    id = *(uint32_t *)call_back;

    if (0 != (event & XIICPS_EVENT_COMPLETE_RECV)){
        rx_complete[id] = true;
    }
    else if (0 != (event & XIICPS_EVENT_COMPLETE_SEND)) {
        tx_complete[id] = true;
    }
    else if (0 == (event & XIICPS_EVENT_SLAVE_RDY)){
        error_count[id]++;
    }
}
