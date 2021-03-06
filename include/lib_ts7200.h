#ifndef LMCVITTI_Y247PAN_TS7000
#define LMCVITTI_Y247PAN_TS7000

/*
 * ts7200.h - definitions describing the ts7200 peripheral registers
 *
 * Specific to the TS-7200 ARM evaluation board
 *
 */

#define	TIMER1_BASE	0x80810000
#define	TIMER2_BASE	0x80810020
#define	TIMER3_BASE	0x80810080

#define	LDR_OFFSET	0x00000000	// 16/32 bits, RW
#define	VAL_OFFSET	0x00000004	// 16/32 bits, RO
#define CRTL_OFFSET	0x00000008	// 3 bits, RW
	#define	ENABLE_MASK	0x00000080
		#define CLOCK_PER_MILLISEC_2K 0x00000002
		#define CLOCK_PER_MILLISEC_508K 0x000001FC
	#define	MODE_MASK	0x00000040
	#define	CLKSEL_MASK	0x00000008
#define CLR_OFFSET	0x0000000c	// no data, WO


#define LED_ADDRESS	0x80840020
	#define LED_NONE	0x0
	#define LED_GREEN	0x1
	#define LED_RED		0x2
	#define LED_BOTH	0x3

#define IRDA_BASE	0x808b0000
#define UART1_BASE	0x808c0000
#define UART2_BASE	0x808d0000

// All the below registers for UART1
// First nine registers (up to Ox28) for UART 2

#define UART_DATA_OFFSET	0x0	// low 8 bits
	#define DATA_MASK	0xff
#define UART_RSR_OFFSET		0x4	// low 4 bits
	#define FE_MASK		0x1
	#define PE_MASK		0x2
	#define BE_MASK		0x4
	#define OE_MASK		0x8
#define UART_LCRH_OFFSET	0x8	// low 7 bits
	#define BRK_MASK	0x1
	#define PEN_MASK	0x2	// parity enable
	#define EPS_MASK	0x4	// even parity
	#define STP2_MASK	0x8	// 2 stop bits
	#define FEN_MASK	0x10	// fifo
	#define WLEN_MASK	0x60	// word length
#define UART_LCRM_OFFSET	0xc	// low 8 bits
	#define BRDH_MASK	0xff	// MSB of baud rate divisor
#define UART_LCRL_OFFSET	0x10	// low 8 bits
	#define BRDL_MASK	0xff	// LSB of baud rate divisor
#define UART_CTLR_OFFSET	0x14	// low 8 bits
	#define UARTEN_MASK	0x1
	#define MSIEN_MASK	0x8	// modem status int
	#define RIEN_MASK	0x10	// receive int
	#define TIEN_MASK	0x20	// transmit int
	#define RTIEN_MASK	0x40	// receive timeout int
	#define LBEN_MASK	0x80	// loopback
#define UART_FLAG_OFFSET	0x18	// low 8 bits
	#define CTS_MASK	0x1
	#define DCD_MASK	0x2
	#define DSR_MASK	0x4
	#define TXBUSY_MASK	0x8
	#define RXFE_MASK	0x10	// Receive buffer empty
	#define TXFF_MASK	0x20	// Transmit buffer full
	#define RXFF_MASK	0x40	// Receive buffer full
	#define TXFE_MASK	0x80	// Transmit buffer empty
#define UART_INTR_OFFSET	0x1c
	#define MIS			0x1
	#define RIS			0x2
	#define TIS			0x4
	#define RTIS		0x8
#define UART_DMAR_OFFSET	0x28

// Specific to UART1

#define UART_MDMCTL_OFFSET	0x100
#define UART_MDMSTS_OFFSET	0x104
#define UART_HDLCCTL_OFFSET	0x20c
#define UART_HDLCAMV_OFFSET	0x210
#define UART_HDLCAM_OFFSET	0x214
#define UART_HDLCRIB_OFFSET	0x218
#define UART_HDLCSTS_OFFSET	0x21c


// Specific to interrupt
#define VIC1 0x800B0000
	#define TC2UI 5
	#define TC2UI_MASK 0x20
#define VIC2 0x800C0000
	#define UART1_INTERRUPT 20
	#define UART1_INT_MASK 0x100000
	#define UART2_INTERRUPT 22
	#define UART2_INT_MASK 0x400000
#define VICxIntEnable		0x10
#define VICxIntEnClear		0x14
#define VICxIRQStatus		0x00
#define VICxRawIntr			0x08
#define VICxIntSelect		0x0c
#define VICxSoftInt			0x18
#define VICxSoftIntClear	0x1c
#define VICxProtection		0x20
#define VICxVectAddr		0x30
#define VIC_IRQ_MODE		0x0
#define VIC_TIMER_INTR_INTERVAL 10


// Specific to idle mode
#define SYS_SW_LOCK 		0x809300C0
#define DEVICE_CFG 			0x80930080
#define HALT 				0x80930008

// terminal
#define TERMINAL_ENTER_KEY_CODE 13
#define BACKSPACE_KEY_CODE 8

// train
#define TRAIN_START 96
#define TRAIN_STOP 97
	#define TRAIN_STOP_DELAY_TICKS 300
#define TRAIN_MIN_SPEED 0
#define TRAIN_MAX_SPEED 14
#define TRAIN_REVERSE_DIRECTION_LIGHTS_ON 31
#define TRAIN_REVERSE_DIRECTION_LIGHTS_OFF 15
#define TRAIN_LIGHTS_OFF 0
#define TRAIN_LIGHTS_ON 16

// rail
#define SWITCH_END 32
	// #define SWITCH_END_DELAY_TICKS 8
#define SWITCH_STRAIGHT 33
	#define SWITCH_STRAIGHT_SYMBOL 'S'
#define SWITCH_BRANCH 34
	#define SWITCH_BRANCH_SYMBOL 'C'
#define SWITCH_ONE_WAY_COUNT 18
#define SWITCH_TWO_WAY_1a 153
#define SWITCH_TWO_WAY_1b 154
#define SWITCH_TWO_WAY_2a 155
#define SWITCH_TWO_WAY_2b 156
#define SWITCH_SAFE_TICKS 50
#define TRACK_LOOP_SWITCH 7
#define SENSOR_DATA_FETCH_BYTE 133
	#define SENSOR_MODULE_BYTES_COUNT 10
	// #define SENSOR_BITS_PER_MODULE 16
	#define BIT_1_MASK 0x1
	#define BIT_2_MASK 0x2
	#define BIT_3_MASK 0x4
	#define BIT_4_MASK 0x8
	#define BIT_5_MASK 0x10
	#define BIT_6_MASK 0x20
	#define BIT_7_MASK 0x40
	#define BIT_8_MASK 0x80
#define INTER_COMMANDS_DELAY_TICKS 8

#endif
