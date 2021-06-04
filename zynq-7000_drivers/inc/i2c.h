#ifndef INC_I2C_H
#define INC_I2C_H

#include "xparameters.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xiicps.h"

#include "types_.h"

typedef struct {
	uint32_t id;
	boolean do_unblocking_mode;
	status init;

    boolean do_master;
    uint16_t self_address; // for slave mode
	uint32_t sclk_rate;
    boolean do_ten_bit_address;
} i2c_inition;

typedef struct {
	uint32_t id;
	boolean do_unblocking_mode;

	uint16_t bus_address; // for master mode

	boolean do_rep_start; //-

    uint8_t *tx_buffer;
    uint8_t *rx_buffer;

    size_t size;

    uint32_t errors;
} i2c_handler;

status i2c_init(i2c_inition *p_init);
status i2c_reinit(i2c_inition *p_init);
status i2c_release(i2c_handler *p_handler); //todo
status i2c_write(i2c_handler *p_handler);
status i2c_read(i2c_handler *p_handler);
boolean i2c_get_ready(i2c_handler *p_handler, boolean is_tx_complete);
status i2c_reset(i2c_handler *p_handler); //todo

//sleep/ wake up

#endif /* INC_I2C_H */
