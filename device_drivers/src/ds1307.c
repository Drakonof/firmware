//todo: get wave out, RAM, right am/pm, порядок и оптимизация
#include <string.h>

#include "ds1307.h"

#define DATA_BUFFER_SIZE 9U
#define AM_PM_GET_BIT_OFFSET 5U
#define AM_PM_SET_BIT_OFFSET 6U
#define SQUARE_RATE_OFFSET 0U
#define SQUARE_WAVE_OFFSET 4U
#define HOURS_MASK       0x3FU
#define HOURS_MASK_AM    0x1FU

#define WR_CONVERT(x)		((x / 10 ) << 4) | (x % 10 )
#define RD_CONVERT(x)		((x  >> 4 ) * 10)  + (x  & 0x0F)

static i2c_handler i2c_handler_;

static uint8_t rx_buffer[DATA_BUFFER_SIZE],
               tx_buffer[DATA_BUFFER_SIZE];

status ds1307_init(uint32_t i2c_id) {
	memset(rx_buffer, 0, DATA_BUFFER_SIZE);
	memset(tx_buffer, 0, DATA_BUFFER_SIZE);
	memset(&i2c_handler_, 0, sizeof(i2c_handler));

//todo: platform relatedness
#if (2 == XPAR_XIICPS_NUM_INSTANCES)
	if ((XPAR_XIICPS_0_DEVICE_ID != i2c_id) || (XPAR_XIICPS_1_DEVICE_ID != i2c_id)) {
		return error_;
	}
#elif (1 == XPAR_XIICPS_NUM_INSTANCES)
	if (XPAR_XIICPS_0_DEVICE_ID != i2c_id) {
		return error_;
	}
#endif

	i2c_handler_.bus_address = DS1307_BUS_ADDRESS;
	i2c_handler_.rx_buffer = rx_buffer;
	i2c_handler_.tx_buffer = tx_buffer;
	i2c_handler_.id = i2c_id;

	return ok_;
}

status ds1307_write(ds1307_handler *p_handler, ds1307_param param) {
	const uint8_t first_wr_field = 0;

	if (NULL == p_handler) {
		return error_;
	}

	if (all != param) {
		i2c_handler_.size = 1;
		tx_buffer[0] = param;
	}
	else {
		i2c_handler_.size = DATA_BUFFER_SIZE - 1;
		tx_buffer[0] = first_wr_field;
	}

	i2c_handler_.do_unblocking_mode = p_handler->do_unblocking_mode;

	switch (param) {
	case seconds:
		tx_buffer[1] = WR_CONVERT(p_handler->seconds);
	break;
	case minutes:
		tx_buffer[1] = WR_CONVERT(p_handler->minutes);
	break;
	case hours:
		tx_buffer[1] = (WR_CONVERT(p_handler->hours) | (p_handler->do_am << AM_PM_SET_BIT_OFFSET));
	break;
	case day:
		tx_buffer[1] = WR_CONVERT(p_handler->day);
	break;
	case date:
		tx_buffer[1] = WR_CONVERT(p_handler->date);
	break;
	case month:
		tx_buffer[1] = WR_CONVERT(p_handler->month);
	break;
	case year:
		tx_buffer[1] = WR_CONVERT(p_handler->year);
	break;
	case control:
		tx_buffer[1] = ((p_handler->do_square_wave << SQUARE_WAVE_OFFSET) |
		               (p_handler->square_rate << SQUARE_RATE_OFFSET));
	break;
	case all:
		tx_buffer[1] = WR_CONVERT(p_handler->seconds);
		tx_buffer[2] = WR_CONVERT(p_handler->minutes);
		tx_buffer[3] = (WR_CONVERT(p_handler->hours) | (p_handler->do_am << AM_PM_SET_BIT_OFFSET));
		tx_buffer[4] = WR_CONVERT(p_handler->day);
		tx_buffer[5] = WR_CONVERT(p_handler->date);
		tx_buffer[6] = WR_CONVERT(p_handler->month);
		tx_buffer[7] = WR_CONVERT(p_handler->year);
		tx_buffer[8] = ((p_handler->do_square_wave << SQUARE_WAVE_OFFSET) |
				       (p_handler->square_rate << SQUARE_RATE_OFFSET));
	break;
	default:
		return error_;
    break;
	}

	if (ok_ != i2c_write(&i2c_handler_)) {
		return error_;
	}

	return ok_;
}

status ds1307_read(boolean do_unblocking_mode, ds1307_param param) {
	const uint8_t first_wr_field = 0;

	i2c_handler_.size = 1;
	i2c_handler_.do_unblocking_mode = do_unblocking_mode;

	if (all != param) {
		rx_buffer[0] = param;
	}
	else {
		rx_buffer[0] = first_wr_field;
	}

	if (ok_ != i2c_write(&i2c_handler_)) {
		return error_;
	}

	while(!i2c_get_ready(&i2c_handler_, true_)) {
		asm("NOP");
	}

	if (all == param) {
		i2c_handler_.size = DATA_BUFFER_SIZE - 1;
	}

	if (ok_ != i2c_read(&i2c_handler_)) {
		return error_;
	}

	return ok_;
}

status ds1307_get_data(ds1307_handler *p_handler, ds1307_param param) {
	if (NULL == p_handler) {
		return error_;
	}

	switch (param) {
	case seconds:
		p_handler->seconds = RD_CONVERT(rx_buffer[0]);
	break;
	case minutes:
		p_handler->minutes = RD_CONVERT(rx_buffer[0]);
	break;
	case hours:
		p_handler->is_am = (boolean)((rx_buffer[2] >> AM_PM_GET_BIT_OFFSET) & 0x1);
		p_handler->hours = RD_CONVERT(rx_buffer[2] /*& ((p_handler->do_am) ? HOURS_MASK_AM : HOURS_MASK)*/);
	break;
	case day:
		p_handler->day = RD_CONVERT(rx_buffer[0]);
	break;
	case date:
		p_handler->date = RD_CONVERT(rx_buffer[0]);
	break;
	case month:
			p_handler->month = RD_CONVERT(rx_buffer[0]);
	break;
	case year:
		p_handler->year = RD_CONVERT(rx_buffer[0]);
	break;
	case control:
		p_handler->do_square_wave = (boolean) (rx_buffer[0] << SQUARE_WAVE_OFFSET);
		p_handler->square_rate = (square_wave_rate) (rx_buffer[0] << SQUARE_RATE_OFFSET);
	break;
	case all:
		p_handler->seconds = RD_CONVERT(rx_buffer[0]);
		p_handler->minutes = RD_CONVERT(rx_buffer[1]);
		p_handler->is_am = (boolean) ((rx_buffer[2] >> AM_PM_GET_BIT_OFFSET) & 0x1);
		p_handler->hours = RD_CONVERT(rx_buffer[2] /*& ((p_handler->do_am) ? HOURS_MASK_AM : HOURS_MASK)*/);
		p_handler->day = RD_CONVERT(rx_buffer[3]);
		p_handler->date = RD_CONVERT(rx_buffer[4]);
		p_handler->month = RD_CONVERT(rx_buffer[5]);
		p_handler->year = RD_CONVERT(rx_buffer[6]);
		p_handler->do_square_wave = (boolean) (rx_buffer[7] >> SQUARE_WAVE_OFFSET);
		p_handler->square_rate = (square_wave_rate) (rx_buffer[7] << SQUARE_RATE_OFFSET);
	break;
	default:
		return error_;
	break;
	}

	return ok_;
}

boolean ds1307_get_ready(boolean is_tx_complete) {
    return i2c_get_ready(&i2c_handler_, is_tx_complete);
}
