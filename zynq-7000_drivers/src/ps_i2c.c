//todo: mirror, repeat
//todo: get wave out, RAM, right am/pm, порядок и оптимизация
#include "ps_i2c.h"

static XScuGic interrupt_cntr;
static XIicPs iic[XPAR_XIICPS_NUM_INSTANCES];

volatile _Bool tx_complete[XPAR_XIICPS_NUM_INSTANCES],
               rx_complete[XPAR_XIICPS_NUM_INSTANCES];

_Bool is_master_mode[XPAR_XIICPS_NUM_INSTANCES];

volatile int8_t error_count[XPAR_XIICPS_NUM_INSTANCES];

static void interrupt_handler_(void *CallBackRef, u32 Event);

static XStatus interrupt_init_(uint32_t id);
static XStatus write_block_mode_(uint32_t id, uint8_t *p_data, size_t size, uint8_t address);
static void write_un_block_mode_(uint32_t id, uint8_t *p_data, size_t size, uint8_t address);
static XStatus read_block_mode_(uint32_t id, uint8_t *p_data, size_t size, uint8_t address);
static void read_un_block_mode_(uint32_t id, uint8_t *p_data, size_t size, uint8_t address);

XStatus ps_i2c_init(ps_i2c_handler *p_handler) {
    XStatus status = XST_FAILURE;
    XIicPs_Config *p_cfg = NULL;

    M_check_pointer(p_handler);

    p_handler->init = XST_FAILURE;

    p_cfg = XIicPs_LookupConfig(p_handler->id);
    M_check_pointer(p_cfg);

    status = XIicPs_CfgInitialize(&iic[p_handler->id],p_cfg,p_cfg->BaseAddress);
    M_check_status(status);

    if (true == p_handler->do_unblocking_mode) {
        status = interrupt_init_(p_handler->id);
        M_check_status(status);
    }

    is_master_mode[p_handler->id] = p_handler->do_master;

    if (true != is_master_mode[p_handler->id]) {
        XIicPs_SetupSlave(&iic[p_handler->id], p_handler->self_address);
    }

    status = XIicPs_SetSClk(&iic[p_handler->id], p_handler->sclk_rate);
    M_check_status(status);

    if (true == p_handler->do_ten_bit_address) {
        status = XIicPs_SetOptions(&iic[p_handler->id],XIICPS_10_BIT_ADDR_OPTION);
        M_check_status(status);
    }
    else {
        status = XIicPs_ClearOptions(&iic[p_handler->id],XIICPS_10_BIT_ADDR_OPTION);
        M_check_status(status);
    }

    p_handler->init = XST_SUCCESS;

    return XST_SUCCESS;
}

XStatus ps_i2c_reinit(ps_i2c_handler *p_handler) {
    XStatus status = XST_FAILURE;

    M_check_pointer(p_handler);
    M_check_status(p_handler->init);

    p_handler->init = XST_FAILURE;

    if (true == p_handler->do_unblocking_mode) {
       status = interrupt_init_(p_handler->id);
       M_check_status(status);
    }

    is_master_mode[p_handler->id] = p_handler->do_master;

    if (true != is_master_mode[p_handler->id]) {
        XIicPs_SetupSlave(&iic[p_handler->id], p_handler->self_address);
    }

    status = XIicPs_SetSClk(&iic[p_handler->id], p_handler->sclk_rate);
    M_check_status(status);

    p_handler->init = XST_SUCCESS;

    return XST_SUCCESS;
}


XStatus ps_i2c_write(ps_i2c_handler *p_handler) {
    XStatus status = XST_FAILURE;

    M_check_pointer(p_handler);
    M_check_status(p_handler->init);

    tx_complete[p_handler->id] = false;
    error_count[p_handler->id] = 0;

    if (true == p_handler->do_unblocking_mode) {
        write_un_block_mode_(p_handler->id,p_handler->tx_buffer,
                             p_handler->size, p_handler->bus_address);
    }
    else {
        status = write_block_mode_(p_handler->id, p_handler->tx_buffer,
                                   p_handler->size, p_handler->bus_address);
        M_check_status(status);

        tx_complete[p_handler->id] = true;
    }

    return XST_SUCCESS;
}

XStatus ps_i2c_read(ps_i2c_handler *p_handler) {
    XStatus status = XST_FAILURE;

    M_check_pointer(p_handler);
    M_check_status(p_handler->init);

    rx_complete[p_handler->id] = false;
    error_count[p_handler->id] = 0;

    if (true == p_handler->do_unblocking_mode) {
        read_un_block_mode_(p_handler->id, p_handler->rx_buffer,
                            p_handler->size, p_handler->bus_address);

    }
    else {
        status = read_block_mode_(p_handler->id, p_handler->rx_buffer,
                                  p_handler->size, p_handler->bus_address);
        M_check_status(status);

        rx_complete[p_handler->id] = true;
    }

    return XST_SUCCESS;
}

//errors??
XStatus ps_i2c_get_ready(ps_i2c_handler *p_handler, ps_i2c_ready_flags ready_flag, _Bool *ready) {
    M_check_pointer(p_handler);
    M_check_status(p_handler->init);

    p_handler->errors = error_count[p_handler->id];

    if (tx_ready_flag == ready_flag) {
    	*ready = tx_complete[p_handler->id];
    }
    else {
    	*ready =  rx_complete[p_handler->id];
    }

    return XST_SUCCESS;
}

static XStatus interrupt_init_(uint32_t id) {
    uint32_t *id_ = NULL;
    XStatus status = XST_FAILURE;
    XScuGic_Config *p_interrupt_cfg = NULL;
    Xil_InterruptHandler p_interrupt_handler = NULL;

    if (true == is_master_mode[id]) {
        p_interrupt_handler = (Xil_InterruptHandler) XIicPs_MasterInterruptHandler;
    }
    else {
        p_interrupt_handler =  (Xil_InterruptHandler) XIicPs_SlaveInterruptHandler;
    }

    *id_ = id;

    Xil_ExceptionInit();

    p_interrupt_cfg = XScuGic_LookupConfig(XPAR_SCUGIC_SINGLE_DEVICE_ID);
    M_check_pointer(p_interrupt_cfg);

    status = XScuGic_CfgInitialize(&interrupt_cntr, p_interrupt_cfg,
                                   p_interrupt_cfg->CpuBaseAddress);
    M_check_status(status);

    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
                                (Xil_ExceptionHandler) XScuGic_InterruptHandler,
                                &interrupt_cntr);

    status = XScuGic_Connect(&interrupt_cntr, XPAR_XIICPS_0_INTR,
    		                 p_interrupt_handler, (void *)&iic[id]);
    M_check_status(status);

    XIicPs_SetStatusHandler(&iic[id], (void *) id_, &interrupt_handler_);

    XScuGic_Enable(&interrupt_cntr, XPAR_XIICPS_0_INTR);

    Xil_ExceptionEnable();

    return XST_SUCCESS;
}

static void write_un_block_mode_(uint32_t id, uint8_t *p_data, size_t size, uint8_t address) {
    if (true == is_master_mode[id]) {
        XIicPs_MasterSend(&iic[id], p_data, size, address);
    }
    else {
        XIicPs_SlaveSend(&iic[id], p_data, size);
    }
}

static XStatus write_block_mode_(uint32_t id, uint8_t *p_data, size_t size, uint8_t address) {
    XStatus status = XST_FAILURE;

    M_check_pointer(p_data);
    M_check_data(size_t, size);

    while (TRUE == XIicPs_BusIsBusy(&iic[id])) {
        asm("NOP");
    }

    if (true == is_master_mode[id]) {
        status = XIicPs_MasterSendPolled(&iic[id], p_data, size, address);
        M_check_status(status);
    }
    else {
        status = XIicPs_SlaveSendPolled(&iic[id], p_data, size);
        M_check_status(status);
    }

    return XST_SUCCESS;
}

static void read_un_block_mode_(uint32_t id, uint8_t *p_data, size_t size, uint8_t address) {
    if (true == is_master_mode[id]) {
        XIicPs_MasterRecv(&iic[id], p_data, size, address);
    }
    else {
        XIicPs_SlaveRecv(&iic[id], p_data, size);
    }
}

static XStatus read_block_mode_(uint32_t id, uint8_t *p_data, size_t size, uint8_t address) {
    XStatus status = XST_FAILURE;

    M_check_pointer(p_data);
    M_check_data(size_t, size);

   // block
    while (TRUE == XIicPs_BusIsBusy(&iic[id])) {
        asm("NOP");
    }

    if (true  == is_master_mode[id]) {
        status = XIicPs_MasterRecvPolled(&iic[id], p_data, size, address);
        M_check_status(status);
    }
    else {
        status = XIicPs_SlaveRecvPolled(&iic[id], p_data, size);
        M_check_status(status);
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
