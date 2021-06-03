#ifndef INC_I2C_H
#define INC_I2C_H

#include "types_.h"

typedef struct {
	uint32_t         id;
	boolean          do_unblocking_mode;
//	void (*interrupt_handler) (void *, uint32_t);

	volatile status  init;

	uint32_t         sclk_rate;
} i2c_inition;

typedef struct {
	uint32_t         id;
	boolean          do_unblocking_mode;

	volatile boolean is_ready;

	boolean          do_master; //-

	uint8_t          bus_address;
	boolean          do_rep_start; //-
    boolean          do_ten_bit_address;

    uint8_t          *tx_buffer;
    uint8_t          *rx_buffer;

    size_t           size;
} i2c_handle;

status i2c_init(i2c_inition *p_init);
status i2c_reinit(i2c_inition *p_init);
status i2c_release(i2c_handle *p_handle); //todo
status i2c_write(i2c_handle *p_handle);
status i2c_read(i2c_handle *p_handle);
boolean i2c_get_ready(); //todo
status i2c_reset(i2c_handle *p_handle); //todo

#endif /* INC_I2C_H */
