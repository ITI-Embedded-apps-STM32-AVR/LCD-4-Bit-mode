/*
 *
 *  Created on: Apr 2, 2020
 *      Author: SARA_MINA
 */

#include "STD_TYPES.h"
#include "GPIO.h"
#include "LCD_cfg.h"

/* databus array indices */
#define LCD_DATA_BUS_PIN4 0
#define LCD_DATA_BUS_PIN5 1
#define LCD_DATA_BUS_PIN6 2
#define LCD_DATA_BUS_PIN7 3

const LCD_cfg_t lcd_cfg = {
	/* DB4 */
	.lcd_IO_DB[LCD_DATA_BUS_PIN4] = {
		.pin = GPIO_PIN4_SELECT,
		.port = GPIO_PIN_ALL_PORTA,
		.mode = GPIO_PIN_ALL_MODE_OUTPUT_PUSH_PULL,
		.speed = GPIO_PIN_ALL_SPEED_2MHZ,
	},

	/* DB5 */
	.lcd_IO_DB[LCD_DATA_BUS_PIN5] = {
		.pin = GPIO_PIN5_SELECT,
		.port = GPIO_PIN_ALL_PORTA,
		.mode = GPIO_PIN_ALL_MODE_OUTPUT_PUSH_PULL,
		.speed = GPIO_PIN_ALL_SPEED_2MHZ,
	},

	/* DB6 */
	.lcd_IO_DB[LCD_DATA_BUS_PIN6] = {
		.pin = GPIO_PIN6_SELECT,
		.port = GPIO_PIN_ALL_PORTA,
		.mode = GPIO_PIN_ALL_MODE_OUTPUT_PUSH_PULL,
		.speed = GPIO_PIN_ALL_SPEED_2MHZ,
	},

	/* DB7 */
	.lcd_IO_DB[LCD_DATA_BUS_PIN7] = {
		.pin = GPIO_PIN7_SELECT,
		.port = GPIO_PIN_ALL_PORTA,
		.mode = GPIO_PIN_ALL_MODE_OUTPUT_PUSH_PULL,
		.speed = GPIO_PIN_ALL_SPEED_2MHZ,
	},

	/* RS pin */
	.lcd_IO_RS = {
		.pin = GPIO_PIN8_SELECT,
		.port = GPIO_PIN_ALL_PORTA,
		.mode = GPIO_PIN_ALL_MODE_OUTPUT_PUSH_PULL,
		.speed = GPIO_PIN_ALL_SPEED_2MHZ,
	},

	/* RW pin */
	.lcd_IO_RW = {
		.pin = GPIO_PIN9_SELECT,
		.port = GPIO_PIN_ALL_PORTA,
		.mode = GPIO_PIN_ALL_MODE_OUTPUT_PUSH_PULL,
		.speed = GPIO_PIN_ALL_SPEED_2MHZ,
	},

	/* E pin */
	.lcd_IO_E = {
		.pin = GPIO_PIN10_SELECT,
		.port = GPIO_PIN_ALL_PORTA,
		.mode = GPIO_PIN_ALL_MODE_OUTPUT_PUSH_PULL,
		.speed = GPIO_PIN_ALL_SPEED_2MHZ,
	},
};

