#ifndef INC_AXI_DMA_POLL_H
#define INC_AXI_DMA_POLL_H

#include "xaxidma.h"
#include "xscugic.h"

#include "platform_.h"

typedef struct {
    uint32_t *p_buf;
    size_t size;
} axi_dma_poll_str;

typedef struct {
    uint32_t tx_intr_id;
    uint32_t rx_intr_id;
    void (*rx_intr_handler)(void *);
    void (*tx_intr_handler)(void *);
    uint32_t dma_id;
} axi_dma_init_str;

typedef struct {
    XAxiDma axi_dma;
    XAxiDma_Config *p_config;
    XScuGic scu_gic;
    XScuGic_Config *p_intc_config;
} axi_dma_handler_str;

int axi_dma_init(axi_dma_init_str *p_init, axi_dma_handler_str *p_handler);
int axi_dma_dev_to_dma_poll(axi_dma_poll_str *p_poll, XAxiDma *p_axi_dma, uint32_t dma_id);
int axi_dma_dma_to_dev_poll(axi_dma_poll_str *p_poll, XAxiDma *p_axi_dma, uint32_t dma_id);
int axi_dma_release(XScuGic *p_scu_gic, uint32_t tx_intr_id, uint32_t rx_intr_id);

#endif


