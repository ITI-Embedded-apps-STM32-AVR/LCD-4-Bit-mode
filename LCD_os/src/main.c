/* libs */
#include "STD_TYPES.h"
/* MCAL */
#include "RCC_interface.h"
#include "GPIO.h"
/* HAL */
#include "LCD.h"
/* Services */
#include "sched_interface.h"
#include <diag/Trace.h>

void app_task(void);

SCHED_task_t const task_app = {
	.runnable = &app_task,
	.periodicTimeMs = 5000,
};

/* set pin C13 mode: {output push-pull 2MHz} */
GPIO_t gpio = {
	.pin   = GPIO_PIN13_SELECT,
	.port  = GPIO_PIN13_PORTC,
	.mode  = GPIO_PIN13_MODE_OUTPUT_PUSH_PULL,
	.speed = GPIO_PIN13_SPEED_2MHZ,
};

void lcd_cmd_cb(void)
{
	trace_printf("LCD CMD CB\n");
}

void lcd_data_cb(void)
{
	trace_printf("LCD Data CB\n");
}

void main()
{
	/* enble HSE, and enable PORTC clock */
	RCC_Init();
	//RCC_EnablePreipheralClock(2, 4); /* port C */
	//RCC_EnablePreipheralClock(2, 3); /* port B */
	RCC_EnablePreipheralClock(2, 2); /* port A */

	/* init pin C13 */
	GPIO_InitPin(&gpio);

	LCD_RegisterCMD_Callback(lcd_cmd_cb);
	LCD_RegisterData_Callback(lcd_data_cb);

	LCD_WriteCMD(LCD_CMD_DisplayOn_CursorOff_BlinkOff); /* turn off cursor and blinking */

	LCD_WriteCMD(LCD_CMD_Set_DDRAM_Addr + 0x01);        /* goto line 1, column 2 */
	LCD_WriteData((u8 *)"Sara cute :)", 12);

	LCD_WriteCMD(LCD_CMD_Set_DDRAM_Addr + 0x43);        /* goto line 2, column 4 */
	LCD_WriteData((u8 *)"Mina -_-", 8);

	/* init Scheduler */
	SCHED_Init();

	SCHED_Start();
}

void app_task(void)
{
	trace_printf("APP CB\n");

}
