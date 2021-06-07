#ifndef INC_I2C__H
#define INC_I2C__H

#include "xparameters.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xiicps.h"

typedef enum {
    tx_ready_flag,
    rx_ready_flag
} i2c_ready_flags;

typedef struct {
    _Bool do_unblocking_mode;
    _Bool do_ten_bit_address;
    _Bool do_master;

    uint32_t id;
    uint16_t self_address; // for slave mode
    uint32_t sclk_rate;

    XStatus init;
} i2c_inition;

typedef struct {
	_Bool do_unblocking_mode;
	_Bool do_rep_start; //-
	uint16_t bus_address; // for master mode
    uint32_t id;

    uint8_t *tx_buffer;
    uint8_t *rx_buffer;
    size_t size;

    uint32_t errors;
} i2c_handler;

XStatus i2c_init(i2c_inition *p_init);
XStatus i2c_reinit(i2c_inition *p_init);
XStatus i2c_release(i2c_handler *p_handler); //todo
XStatus i2c_write(i2c_handler *p_handler);
XStatus i2c_read(i2c_handler *p_handler);
_Bool i2c_get_ready(i2c_handler *p_handler, i2c_ready_flags ready_flag);
XStatus i2c_reset(i2c_handler *p_handler); //todo

//sleep/ wake up

#endif /* INC_I2C_H */
