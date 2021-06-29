//todo: неблокирующее чтение и запись
#include "ps_gpio.h"

static XGpioPs gpio[XPAR_XGPIOPS_NUM_INSTANCES];

static XScuGic interrupt_cntr;

static void interrupt_handler_(void *call_backRef, u32 bank, u32 status);

static XStatus interrupt_init_(uint32_t id, uint8_t bank, _Bool is_level_intrr_type,
                               _Bool is_low_intrr_polarity, _Bool is_single_edge_inttr,
                               uint32_t pin_number);

XStatus ps_gpio_init(ps_gpio_handler *p_handler) {
    XStatus status = XST_FAILURE;
    XGpioPs_Config *p_cfg = NULL;

    if (NULL == p_handler) {
        return XST_FAILURE;
    }

    p_handler->init = XST_FAILURE;

    p_cfg = XGpioPs_LookupConfig(p_handler->id);

    if (NULL == p_cfg) {
        return XST_FAILURE;
    }

    status = XGpioPs_CfgInitialize(&gpio[p_handler->id], p_cfg, p_cfg->BaseAddr);
    if (XST_SUCCESS != status) {
        return status;
    }

    if (true == p_handler->do_unblocking_mode) {
        status = interrupt_init_(p_handler->id,p_handler->bank_number,
                                 p_handler->do_level_intrr_type,
                                 p_handler->do_low_intrr_polarity,
                                 p_handler->do_single_edge_inttr,
                                 p_handler->pin_number);
        if (XST_SUCCESS != status) {
            return status;
        }
    }

    XGpioPs_SetOutputEnablePin(&gpio[p_handler->id], p_handler->pin_number, TRUE);

    p_handler->init = XST_SUCCESS;

    return XST_SUCCESS;
}

XStatus ps_gpio_reinit(ps_gpio_handler *p_handler) {
    XStatus status = XST_FAILURE;

    if (XST_SUCCESS != p_handler->init) {
        return p_handler->init;
    }

    p_handler->init = XST_FAILURE;

    if (true == p_handler->do_unblocking_mode) {
        status = interrupt_init_(p_handler->id,p_handler->bank_number,
                                 p_handler->do_level_intrr_type,
                                 p_handler->do_low_intrr_polarity,
                                 p_handler->do_single_edge_inttr,
                                 p_handler->pin_number);
        if (XST_SUCCESS != status) {
            return status;
        }
    }

    p_handler->init = XST_SUCCESS;

    return XST_SUCCESS;
}

XStatus ps_gpio_write(ps_gpio_handler *p_handler, _Bool value) {
    if (NULL == p_handler) {
        return XST_FAILURE;
    }

    if (XST_SUCCESS != p_handler->init) {
        return p_handler->init;
    }

    if (true == p_handler->is_sleep) {
        XGpioPs_SetOutputEnablePin(&gpio[p_handler->id], p_handler->pin_number, TRUE);
    }

    XGpioPs_SetDirectionPin(&gpio[p_handler->id], p_handler->pin_number, TRUE);
    XGpioPs_WritePin(&gpio[p_handler->id], p_handler->pin_number, (uint32_t) value);

    return XST_SUCCESS;
}

XStatus ps_gpio_read(ps_gpio_handler *p_handler, _Bool *bit) {
    if (NULL == p_handler) {
        return XST_FAILURE;
    }

    if (NULL == bit) {
        return XST_FAILURE;
    }


    if (XST_SUCCESS != p_handler->init) {
        return p_handler->init;
    }

    if (true == p_handler->is_sleep) {
        XGpioPs_SetOutputEnablePin(&gpio[p_handler->id], p_handler->pin_number, TRUE);
    }

    *bit = false;

    XGpioPs_SetDirectionPin(&gpio[p_handler->id], p_handler->pin_number, FALSE);
    *bit = XGpioPs_ReadPin(&gpio[p_handler->id], p_handler->pin_number);

    return XST_SUCCESS;
}

XStatus ps_gpio_sleep(ps_gpio_handler *p_handler) {
    if (NULL == p_handler) {
        return XST_FAILURE;
    }

    if (XST_SUCCESS != p_handler->init) {
        return p_handler->init;
    }

    XGpioPs_SetOutputEnablePin(&gpio[p_handler->id], p_handler->pin_number, FALSE);

    p_handler->is_sleep = true;

    return XST_SUCCESS;
}

XStatus ps_gpio_wake(ps_gpio_handler *p_handler) {
    if (NULL == p_handler) {
        return XST_FAILURE;
    }

    if (XST_SUCCESS != p_handler->init) {
        return p_handler->init;
    }

    XGpioPs_SetOutputEnablePin(&gpio[p_handler->id], p_handler->pin_number, TRUE);

    p_handler->is_sleep = false;

    return XST_SUCCESS;
}

static XStatus interrupt_init_(uint32_t id, uint8_t bank, _Bool is_level_intrr_type,
                               _Bool is_low_intrr_polarity, _Bool is_single_edge_inttr,
                               uint32_t pin_number) {
    uint32_t *id_ = NULL;
    Xil_InterruptHandler p_interrupt_handler = NULL;
    XStatus status = XST_FAILURE;

    XGpioPs instance = {};

    XScuGic_Config *p_interrupt_cfg = NULL;

    Xil_ExceptionInit();

    p_interrupt_cfg = XScuGic_LookupConfig(XPAR_SCUGIC_SINGLE_DEVICE_ID);
    if (NULL == p_interrupt_cfg) {
    return XST_FAILURE;
    }

    status = XScuGic_CfgInitialize(&interrupt_cntr, p_interrupt_cfg, p_interrupt_cfg->CpuBaseAddress);
    if (XST_SUCCESS != status) {
        return status;
    }

    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,(Xil_ExceptionHandler)XScuGic_InterruptHandler, &interrupt_cntr);

    *id_ = id;
    status = XScuGic_Connect(&interrupt_cntr, XPAR_XIICPS_0_INTR,
                             p_interrupt_handler, (void *)&gpio[id]);
    if (XST_SUCCESS != status) {
        return status;
    }

    XGpioPs_SetIntrType(&gpio[id], bank, is_level_intrr_type,
                        is_low_intrr_polarity/*0xFFFFFFFF*/, is_single_edge_inttr);

    XGpioPs_SetCallbackHandler(&gpio[id], (void *) id_, &interrupt_handler_);

    //todo:------------------------------------
    instance.IsReady = XIL_COMPONENT_IS_READY;
    instance.MaxBanks = 3;
    instance.GpioConfig.BaseAddr = XPAR_PS7_GPIO_0_BASEADDR;
    XGpioPs_IntrEnable(&instance, bank, (1 << pin_number));
    //-----------------------------------------

    XScuGic_Enable(&interrupt_cntr, XPAR_XGPIOPS_0_INTR);

    Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);

    return XST_SUCCESS;
}

static void interrupt_handler_(void *call_backRef, uint32_t bank, uint32_t status) {
    //todo:
}
