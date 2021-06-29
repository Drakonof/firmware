#include "axi_uart_sys_.h"

XStatus axi_uart_init(axi_uart_handle *p_handle, axi_uart_inition *init) {
    return init_driver_(p_handle, init, true);
}

static inline XStatus init_driver_(axi_uart_handle *p_handle, axi_uart_inition *init, _Bool do_init) {
    p_handle->ready = false;
    p_handle->init  = XST_FAILURE;

    if ((NULL == p_handle) || (NULL == init)) {
        return XST_FAILURE;
    }

    set_stop_bits((axi_uart_regs* )p_handle->id, init->stop_bits, do_init);
    //set_parity_type ((ps_uart_regs* )p_handle->id, init->parity_type, do_init);
    set_data_bits((axi_uart_regs* )p_handle->id, init->data_bits, do_init);
    set_baud_rate((axi_uart_regs* )p_handle->id, init->baud_rate, do_init);

    if (true == do_init) {
        p_handle->init = XST_SUCCESS;
        p_handle->ready = true;
    }

    return XST_SUCCESS;
}

XStatus axi_uart_reinit(axi_uart_handle *p_handle, axi_uart_inition *init) {
    if (XST_SUCCESS != p_handle->init) {
        return p_handle->init;
    }

    return init_driver_(p_handle, init, true);
}

XStatus axi_uart_release(axi_uart_handle *p_handle) {
    if (XST_SUCCESS != p_handle->init) {
        return p_handle->init;
    }

    axi_uart_inition init = {};

    return init_driver_(p_handle, &init, false);
}

XStatus axi_uart_write_data(axi_uart_handle *p_handle, char *p_data, size_t size) {
    axi_uart_regs *p_uart_regs = NULL;

    if ((NULL == p_handle) || (NULL == p_data) || (0 == size)) {
        return XST_FAILURE;
    }

    if (XST_SUCCESS != p_handle->init) {
        return p_handle->init;
    }

    p_handle->ready = false;

    p_uart_regs = (axi_uart_regs* )p_handle->id;

    if (true == p_handle->do_unblocking_mode) {
        //todo: all
    }
    else {
        write_block_mode_data_(p_uart_regs, p_data, size);
        p_handle->ready = true;
    }

    return XST_SUCCESS;
}

static inline void write_block_mode_data_(axi_uart_regs *p_uart_regs, char *p_data, size_t size) {
    uint32_t i = 0;

    for(i = 0; i < size; i++) {
        set_data_word_(p_uart_regs , p_data[i]);
        set_tx_enable_(p_uart_regs, true);
        while(AXI_UART_TX_TRNS_COMPLETE != get_status_(p_uart_regs));
    }
}

//---------------------------------------------lower half---------------------------------------------//

static inline void set_stop_bits(axi_uart_regs *p_reg_base, axi_uart_stop_bits stop_bits, _Bool do_setting) {
    p_reg_base->control &= ~(TRUE << AXI_UART_STOP_NUM_BIT_OFFSET);

    if (true == do_setting) {
        p_reg_base->control |= (stop_bits << AXI_UART_STOP_NUM_BIT_OFFSET);
    }
    else {
        p_reg_base->control |= (AXI_UART_CONTROL_REG_RST_VAL & AXI_UART_STOP_NUM_BIT_MASK);
    }
}

static inline void set_data_bits(axi_uart_regs *p_reg_base, axi_uart_data_bits data_bits, _Bool do_setting) {
    p_reg_base->control &= ~(TRUE << AXI_UART_DATA_NUM_BIT_OFFSET);

    if (true == do_setting) {
        p_reg_base->control |= (data_bits << AXI_UART_DATA_NUM_BIT_OFFSET);
    }
    else {
        p_reg_base->control |= (AXI_UART_CONTROL_REG_RST_VAL & AXI_UART_DATA_NUM_BIT_MASK);
    }
}

static inline void set_baud_rate(axi_uart_regs *p_reg_base, uint32_t baud_rate, _Bool do_setting) {
    p_reg_base->baud_rate = AXI_UART_BAUD_REG_RST_VAL;

    if (true == do_setting) {
        p_reg_base->baud_rate = baud_rate;
    }
}

static inline void set_tx_enable_(axi_uart_regs *p_reg_base, _Bool do_setting) {
    p_reg_base->control &= ~(TRUE << AXI_UART_TX_EN_BIT_OFFSET);

    if (true == do_setting) {
        p_reg_base->control |= (TRUE << AXI_UART_TX_EN_BIT_OFFSET);
    }
    else {
        p_reg_base->control |= (AXI_UART_CONTROL_REG_RST_VAL & AXI_UART_TX_EN_BIT_MASK);
    }
}

//status pre or after?
static inline void set_data_word_(axi_uart_regs *p_reg_base, char data) {
    p_reg_base->tx_data = data;
}

static inline uint32_t get_status_(axi_uart_regs *p_reg_base) {
    return p_reg_base->status;
}
