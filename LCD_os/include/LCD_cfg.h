/*
 *
 *  Created on: Apr 2, 2020
 *      Author: SARA_MINA
 */

#ifndef LCD_CFG_H_
#define LCD_CFG_H_

/* maximum size of queue */
#define LCD_QUEUE_MAX_LEN 10

/* maximum size of data buffer within the queue */
#define LCD_DATA_MAX_LEN  16

#define LCD_COUNT         1

typedef struct
{
	GPIO_t lcd_IO_DB[4];
	GPIO_t lcd_IO_RS;
	GPIO_t lcd_IO_RW;
	GPIO_t lcd_IO_E;
} LCD_cfg_t;


#endif /* LCD_CFG_H_ */
