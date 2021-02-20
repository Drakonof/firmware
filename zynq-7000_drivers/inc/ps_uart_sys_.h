#ifndef INC_UART_SYS__H
#define INC_UART_SYS__H

#ifdef XPAR_XUARTPS_NUM_INSTANCES

#define PS_UART_MODE_REG_RESET_VALUE     (uint32_t) 0x00000000

#define PS_UART_STOP_NUM_BIT_MASK        (uint32_t) 0x000000C0
#define PS_UART_STOP_NUM_BIT_0_OFFSET    (uint32_t) 0x00000006
#define PS_UART_STOP_NUM_BIT_1_OFFSET    (uint32_t) 0x00000007

#define PS_UART_PAR_TYP_BIT_MASK         (uint32_t) 0x00000018
#define PS_UART_PAR_TYP_BIT_0_OFFSET     (uint32_t) 0x00000003
#define PS_UART_PAR_TYP_BIT_1_OFFSET     (uint32_t) 0x00000004

#define PS_UART_DATA_NUM_BIT_MASK        (uint32_t) 0x00000006
#define PS_UART_DATA_NUM_BIT_0_OFFSET    (uint32_t) 0x00000001
#define PS_UART_DATA_NUM_BIT_1_OFFSET    (uint32_t) 0x00000001

#define PS_UART_CHNL_MD_BIT_MASK         (uint32_t) 0x00000300
#define PS_UART_CHNL_MD_BIT_0_OFFSET     (uint32_t) 0x00000008
#define PS_UART_CHNL_MD_BIT_1_OFFSET     (uint32_t) 0x00000009

#define PS_UART_CLK_SELECT_BIT_MASK      (uint32_t) 0x00000300
#define PS_UART_CLK_SELECT_BIT_0_OFFSET  (uint32_t) 0x00000008
#define PS_UART_CLK_SELECT_DIV8_VALUE    (uint32_t) 0x00000001
#define PS_UART_CLK_SELECT_NONDIV8_VALUE (uint32_t) 0x00000000

#define PS_UART_BD_GN_REG_RESET_VALUE    (uint32_t) 0x0000028B
#define PS_UART_BD_GN_REG_BPASS_VALUE    (uint32_t) 0x00000001
#define PS_UART_BD_GN_REG_DSABLE_VALUE   (uint32_t) 0x00000000
#define PS_UART_BD_GN_REG_DEF_VALUE      (uint32_t) 0x0000007C

#define PS_UART_BD_RT_REG_RESET_VALUE    (uint32_t) 0x0000000F

#define PS_UART_CONTROL_REG_RESET_VALUE  (uint32_t) 0x00000128
#define PS_UART_TX_DISABLE_BIT_OFFSET    (uint32_t) 0x00000005
#define PS_UART_TX_ENABLE_BIT_OFFSET     (uint32_t) 0x00000004
#define PS_UART_RX_DISABLE_BIT_OFFSET    (uint32_t) 0x00000003
#define PS_UART_RX_ENABLE_BIT_OFFSET     (uint32_t) 0x00000002
#define PS_UART_TX_RESET_BIT_OFFSET      (uint32_t) 0x00000001
#define PS_UART_RX_RESET_BIT_OFFSET      (uint32_t) 0x00000001

#define PS_UART_RX_FIFO_LVL_REG_RESET_VALUE (uint8_t) 0x20

#define PS_UART_RX_FIFO_TRIG_INTR_MSK   (uint32_t) 0x00000001
#define PS_UART_RX_FIFO_EMPTY_INTR_MSK  (uint32_t) 0x00000002
#define PS_UART_RX_FIFO_FULL_INTR_MSK   (uint32_t) 0x00000004
#define PS_UART_TX_FIFO_EMPTY_INTR_MSK  (uint32_t) 0x00000008
#define PS_UART_TX_FIFO_FULL_INTR_MSK   (uint32_t) 0x00000010
#define PS_UART_RX_OVERFLOW_INTR_MSK    (uint32_t) 0x00000020
#define PS_UART_RX_FRME_ERROR_INTR_MSK  (uint32_t) 0x00000040
#define PS_UART_RX_PART_ERROR_INTR_MSK  (uint32_t) 0x00000080
#define PS_UART_RX_TMOUT_ERROR_INTR_MSK (uint32_t) 0x00000100
#define PS_UART_MD_STATUS_INDR_INTR_MSK (uint32_t) 0x00000200
#define PS_UART_TX_FIFO_TRIG_INTR_MSK   (uint32_t) 0x00000400
#define PS_UART_TX_FIFO_NRFULL_INTR_MSK (uint32_t) 0x00000800
#define PS_UART_TX_OVERFLOW_INTR_MSK    (uint32_t) 0x00001000

typedef struct {
	volatile uint32_t control;
	volatile uint32_t mode;
	volatile uint32_t interrupt_enable;
	volatile uint32_t interrupt_disable;
	volatile uint32_t interrupt_mask;
	volatile uint32_t interrupt_status;
	volatile uint32_t baud_rate_generator;
	volatile uint32_t rx_time_out;
	volatile uint32_t rx_fifo_level;
	volatile uint32_t modem_control;
	volatile uint32_t modem_status;
	volatile uint32_t channel_status;
	volatile uint32_t tx_rx_fifo;
	volatile uint32_t baud_rate_divider;
	volatile uint32_t flow_daley;
	volatile uint32_t tx_fifo_level;
} ps_uart_regs;

//---------------------------------------------upper half---------------------------------------------//
static inline status init_driver_(ps_uart_handle *p_handle,
		                          ps_uart_inition *init,
								  boolean do_init);

static inline status check_params_(uint32_t baud_rate);
static inline void read_un_block_mode_data_();
static inline void read_block_mode_data_(ps_uart_regs *p_uart_regs, char *p_data, size_t size);
static inline void write_un_block_mode_data_();
static inline void write_block_mode_data_(ps_uart_regs *p_uart_regs, char *p_data, size_t size);

//---------------------------------------------lower half---------------------------------------------//
static inline void set_stop_bits(ps_uart_regs *p_reg_base, ps_uart_stop_bits stop_bits, boolean do_setting);
static inline void set_parity_type(ps_uart_regs *p_reg_base, ps_uart_parity_type parity_type, boolean do_setting);
static inline void set_data_bits(ps_uart_regs *p_reg_base, ps_uart_data_bits data_bits, boolean do_setting);
static inline void set_channel_mode_(ps_uart_regs *p_reg_base, ps_uart_channel_mode channel_mode, boolean do_setting);
static inline void set_baud_rate(ps_uart_regs *p_reg_base, uint32_t baud_rate, boolean do_setting);
static inline char get_data_word_(ps_uart_regs *p_reg_base);
static inline void set_data_word_(ps_uart_regs *p_reg_base, char data);
static inline void set_tx_enable_(ps_uart_regs *p_reg_base, boolean do_setting);
static inline void set_reset_(ps_uart_regs *p_reg_base, ps_uart_path_reset path_reset, boolean *reset_done);
static inline void set_interrupt_(ps_uart_regs *p_uart_regs, uint32_t interrupt_mask); //uint32_t??
static inline void set_rx_fifo_level(ps_uart_regs *p_reg_base, uint8_t rx_fifo_level, boolean do_setting);

static void intrrupt_handler(void *callback);

#endif /* XPAR_XUARTPS_NUM_INSTANCES */
#endif /* INC_UART_SYS__H */
