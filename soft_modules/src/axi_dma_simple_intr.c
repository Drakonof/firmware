#include "xil_exception.h"

#include "axi_dma_intr_poll.h"

static int enable_intr_(axi_dma_handler_str *p_handler, axi_dma_init_str *p_init);
static int set_intr_handler_(uint32_t intr_id, XScuGic *p_scu_gic, void(*intr_handler)(void *), XAxiDma *p_axi_dma);

int axi_dma_init(axi_dma_init_str *p_init, axi_dma_handler_str *p_handler) {

    if ((NULL == p_init) || (NULL == p_handler)) {
        xil_printf("AXI DMA %d ERROR: the entire axi_dma_init function ERROR\r\n", p_init->dma_id);
        return XST_FAILURE;
    }

    memset(p_handler, 0, sizeof(axi_dma_handler_str));

    p_handler->p_config = XAxiDma_LookupConfig(p_init->dma_id);
    if (NULL == p_handler->p_config) {
        xil_printf("AXI DMA %d ERROR: the dma lookup config FAILED\r\n", p_init->dma_id);
        return XST_FAILURE;
    }

    if (XST_SUCCESS != XAxiDma_CfgInitialize(&(p_handler->axi_dma), p_handler->p_config)) {
        xil_printf("AXI DMA %d ERROR: the dma initialization FAILED\r\n", p_init->dma_id);
        return XST_FAILURE;
    }

    if(TRUE == XAxiDma_HasSg(&(p_handler->axi_dma))){
        xil_printf("AXI DMA %d ERROR: the device configured as SG mode\r\n", p_init->dma_id);
        return XST_FAILURE;
    }

    if (XST_SUCCESS != enable_intr_(p_handler, p_init)) {
        xil_printf("AXI DMA %d ERROR: the interrupt setup FAILED\r\n");
        return XST_FAILURE;
    }

    return XST_SUCCESS;
}

int axi_dma_dev_to_dma_poll (axi_dma_poll_str *p_poll, XAxiDma *p_axi_dma, uint32_t dma_id) {

	if ((NULL == p_poll) || (NULL == p_axi_dma)) {
	    xil_printf("AXI DMA %d ERROR: the entire axi_dma_poll function ERROR\r\n", dma_id);
	    return XST_FAILURE;
	}

	if (XST_SUCCESS != XAxiDma_SimpleTransfer(p_axi_dma,(UINTPTR) (p_poll->p_buf),
	                                              p_poll->size, XAXIDMA_DEVICE_TO_DMA)) {
	    xil_printf("AXI DMA %d ERROR: the rx buffer setting FAILED\r\n", dma_id);
	    return XST_FAILURE;
	}

	return XST_SUCCESS;
}

int axi_dma_dma_to_dev_poll(axi_dma_poll_str *p_poll, XAxiDma *p_axi_dma, uint32_t dma_id) {

    if ((NULL == p_poll) || (NULL == p_axi_dma)) {
        xil_printf("AXI DMA %d ERROR: the entire axi_dma_poll function ERROR\r\n", dma_id);
        return XST_FAILURE;
    }

    Xil_DCacheFlushRange((UINTPTR) (p_poll->p_buf), p_poll->size);

    if (XST_SUCCESS != XAxiDma_SimpleTransfer(p_axi_dma,(UINTPTR) (p_poll->p_buf),
                                              p_poll->size, XAXIDMA_DMA_TO_DEVICE)) {
        xil_printf("AXI DMA %d ERROR: the tx buffer setting FAILED\r\n", dma_id);
        return XST_FAILURE;
    }

    return XST_SUCCESS;
}

int axi_dma_release(XScuGic *p_scu_gic, uint32_t tx_intr_id, uint32_t rx_intr_id) {

    if ((NULL == p_scu_gic) || (0 == tx_intr_id) || (0 == rx_intr_id)) {
        xil_printf("AXI DMA %d ERROR: the entire axi_dma_release function ERROR\r\n", p_scu_gic->Config->DeviceId);
        return XST_FAILURE;
    }

    XScuGic_Disconnect(p_scu_gic, tx_intr_id);
    XScuGic_Disconnect(p_scu_gic, rx_intr_id);

    //DisableIntrSystem(&Intc, TX_INTR_ID, RX_INTR_ID);

    return XST_SUCCESS;
}

static int set_intr_handler_(uint32_t intr_id, XScuGic *p_scu_gic,
		                    void(*intr_handler)(void *), XAxiDma *p_axi_dma) {

	const uint8_t c_priority = 0xA0, c_trigger_type = 0x3;

	if (0 != intr_id) {

		if (NULL == intr_handler) {
			return XST_FAILURE;
		}

		XScuGic_SetPriorityTriggerType(p_scu_gic, intr_id, c_priority, c_trigger_type);
		if (XST_SUCCESS != XScuGic_Connect(p_scu_gic, intr_id, (Xil_InterruptHandler) intr_handler,
									  p_axi_dma)) {
			 xil_printf("AXI DMA ERROR: the scu gic tx connection FAILED\r\n");
			 return XST_FAILURE;
		}
	}
	else {
		//todo
	}

	return XST_SUCCESS;
}

static int enable_intr_(axi_dma_handler_str *p_handler, axi_dma_init_str *p_init) {



    p_handler->p_intc_config = XScuGic_LookupConfig(XPAR_SCUGIC_SINGLE_DEVICE_ID);
    if (NULL == p_handler->p_intc_config) {
		xil_printf("AXI DMA %d ERROR: the scu gic lookup config FAILED\r\n", p_init->dma_id);
		return XST_FAILURE;
	}

    if (XST_SUCCESS != XScuGic_CfgInitialize(&(p_handler->scu_gic), p_handler->p_intc_config,
                                               p_handler->p_intc_config->CpuBaseAddress)) {
        xil_printf("AXI DMA %d ERROR: the scu gic initialization FAILED\r\n", p_init->dma_id);
        return XST_FAILURE;
	}

    if (XST_SUCCESS != set_intr_handler_(p_init->tx_intr_id, &(p_handler->scu_gic),
    		                             p_init->tx_intr_handler, &(p_handler->axi_dma))) {
    	xil_printf("AXI DMA %d ERROR: the tx intr_handler ERROR\r\n", p_init->dma_id);
    	return XST_FAILURE;
    }

    if (XST_SUCCESS != set_intr_handler_(p_init->rx_intr_id, &(p_handler->scu_gic),
        		      p_init->rx_intr_handler, &(p_handler->axi_dma))) {
    	xil_printf("AXI DMA %d ERROR: the rx intr_handler ERROR\r\n", p_init->dma_id);
    	return XST_FAILURE;
    }

    XScuGic_Enable(&(p_handler->scu_gic), p_init->tx_intr_id);
    XScuGic_Enable(&(p_handler->scu_gic), p_init->rx_intr_id);

    Xil_ExceptionInit();
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)XScuGic_InterruptHandler,
			                    (void *)&(p_handler->scu_gic));

    Xil_ExceptionEnable();

    XAxiDma_IntrDisable(&(p_handler->axi_dma), XAXIDMA_IRQ_ALL_MASK,
                       XAXIDMA_DMA_TO_DEVICE);

    XAxiDma_IntrDisable(&(p_handler->axi_dma), XAXIDMA_IRQ_ALL_MASK,
                       XAXIDMA_DEVICE_TO_DMA);

    XAxiDma_IntrEnable(&(p_handler->axi_dma), XAXIDMA_IRQ_ALL_MASK,
                      XAXIDMA_DMA_TO_DEVICE);

    XAxiDma_IntrEnable(&(p_handler->axi_dma), XAXIDMA_IRQ_ALL_MASK,
                      XAXIDMA_DEVICE_TO_DMA);

    return XST_SUCCESS;
}
