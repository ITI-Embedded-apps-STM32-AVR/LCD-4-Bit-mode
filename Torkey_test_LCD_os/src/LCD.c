/*
 *
 *  Created on: Apr 2, 2020
 *      Author: SARA_MINA
 */

#include "STD_TYPES.h"
#include "LCD.h"
#include "GPIO.h"
#include "LCD_cfg.h"
#include "sched_interface.h"

/* queue element could be either CMD or data */
#define LCD_QUEUE_OBJECT_TYPE_CMD             0
#define LCD_QUEUE_OBJECT_TYPE_DATA            1

/* RS pin modes */
#define LCD_RS_COMMAND_MODE                   GPIO_PIN_ALL_VALUE_LOW
#define LCD_RS_DATA_MODE                      GPIO_PIN_ALL_VALUE_HIGH

/* RW pin modes (we're only concerned with the write mode) */
#define LCD_RW_WRITE_MODE                     GPIO_PIN_ALL_VALUE_LOW

/* initial power-on delay (60ms required / 2ms periodicity = 30) */
#define LCD_INIT_POWER_DELAY_MAX_CTR_VALUE    30
/* during the initialization in the 4-bits commands
 * we must wait 6ms after each command (6ms required / 2ms periodicity - wasted cycle = 2) */
#define LCD_INIT_NIBBLE_DELAY_MAX_CTR_VALUE   2

/* each nibble (higher or lower) is written on 2 stages (data setup stage, then data hold stage)
 * with a tick time = 2ms between each */
#define LCD_NIBBLE_STAGE_COUNT                2

/* macros to extract either the higher or lower nibbles from am 8-bit value */
#define LOWER_4_BITS(n)                       (n & (0x0F))
#define HIGHER_4_BITS(n)                      ((n >> 4) & (0x0F))


/* ================= internal types ================= */

/* writing on GPIO stage indicator */
typedef enum
{
	LCD_Stage_WriteOnBus_Stage_DataSetup,
	LCD_Stage_WriteOnBus_Stage_DataHold,
} LCD_Stage_WriteOnBus_t;

/* early initialization stage indicator */
typedef enum
{
	LCD_Stage_Init_EarlyStage_GPIO,
	LCD_Stage_Init_EarlyStage_Power_Delay,
	LCD_Stage_Init_EarlyStage_InitSequence,
} LCD_Stage_Init_EarlyStage_t;

/* runnable task stage indicator */
typedef enum
{
	LCD_Stage_Task_StageInit,
	LCD_Stage_Task_StageWriteCMD,
	LCD_Stage_Task_StageWriteData,
	LCD_Stage_Task_StagePopQueue,
	LCD_Stage_Task_StageIdle,
} LCD_Stage_Task_t;

/* commands set indicator */
typedef enum
{
	LCD_Stage_Init_CMD_BitMode4,
	LCD_Stage_Init_CMD_BitMode8,
} LCD_Stage_Init_CMD_BitMode_t;

/* nibble stage indicator */
typedef enum
{
	LCD_Nibble_Stage_High,
	LCD_Nibble_Stage_Low,
} LCD_Nibble_Stage_t;

/* queue element type */
typedef struct
{
	/* if queue object type is command */
	u8 cmd;

	/* if queue object type is data */
	u8 data[LCD_DATA_MAX_LEN];
	u8 len;

	u8 type;
} LCD_Queue_Unit_t;

/* ====================================================== */


/* ============= private functions prototypes ============= */

static u8 LCD_IsQueueFull(void);
static u8 LCD_IsQueueEmpty(void);
static u8 LCD_PushCMD(u8 cmd);
static u8 LCD_PushData(u8 *data, u8 len);
static void LCD_Pop(void);
static void LCD_Init(void);
static void LCD_WriteCMD_Internal(u64 cmd);
static void LCD_WriteOnBus(u64 mode, u8 cmd_or_data);
static void LCD_WriteData_Internal(u8 *data, u8 len);
static void LCD_Task(void);

//static u64 LCD_RemapGPIO_Value(u8 val);

/* ======================================================== */


/* ================================================================================================================== */

/* stage indexer (higher or lower nibble) */
LCD_Nibble_Stage_t LCD_Nibble_Stage                     = LCD_Nibble_Stage_High;
/* nibble count-up counter (each nibble takes 2 sched ticks) */
u8 LCD_Nibble_Ctr                                       = 0;

/* indexer for an array of commands (as in init sequence array),
 * or for an array of data (when writing user data) */
u8 LCD_CMD_Data_Idx                                     = 0;

/* delay count-up counter (counts how many ticks required
 * given that LCD task periodicity = 2ms) */
u8 LCD_DelayCtr                                         = 0;

/* early initialization stage indexer (initializing GPIO, or power on delay, or init sequence) */
LCD_Stage_Init_EarlyStage_t LCD_Stage_Init_EarlyStage   = LCD_Stage_Init_EarlyStage_GPIO;
/* init sequence CMD stage indexer (executing 4-bit commands, or 8-bit commands) */
LCD_Stage_Init_CMD_BitMode_t LCD_Stage_Init_CMD_BitMode = LCD_Stage_Init_CMD_BitMode4;

/* global variable that holds the user CMD being executed (used by LCD task to call internal function) */
u8 LCD_UserCMD                                          = 0;
/* global variables that holds the user data and length being processed (used by LCD task to call internal function) */
u8* LCD_UserData                                        = 0;
u8 LCD_UserLen                                          = 0;

/* LCD task stage indexer (init, or write cmd, or write data, or idle) */
LCD_Stage_Task_t LCD_Stage_Task                         = LCD_Stage_Task_StageInit;

/* stage indexer used by internal function to write on the GPIO (data setup stage, or data hold stage) */
static LCD_Stage_WriteOnBus_t LCD_Stage_WriteOnBus      = LCD_Stage_WriteOnBus_Stage_DataSetup;

/* callbacks to notify the App when a CMD or data are completely written */
LCD_CMD_CB_t LCD_CMD_CB                                 = 0;
LCD_Data_CB_t LCD_Data_CB                               = 0;

/* queue indicators and array */
u8 queue_current                                        = 0; /* points at the currently ready element */
u8 queue_last                                           = 0; /* points after the last added element */
u8 queueCurrentLen                                      = 0; /* holds the current length of the queue (number of elements) */
LCD_Queue_Unit_t LCD_Queue[LCD_QUEUE_MAX_LEN];               /* queue that holds the App requests (CMD or data) */

/* ================================================================================================================== */


/* ============================= internal/private objects ============================= */

/* initialization sequence commands,
 * in the 1st set only the higher 4 bits are executed,
 * in the 2nd set the entire 8 bits are executed */
static const u8 LCD_Init_Sequence_4Bit_CMD[] = { /* only the higher nibble is executed  */
	LCD_CMD_Function_8BitMode_1Line_5x7Dots,
	LCD_CMD_Function_8BitMode_1Line_5x7Dots,
	LCD_CMD_Function_8BitMode_1Line_5x7Dots,
	LCD_CMD_Function_4BitMode_1Line_5x7Dots,
};

static const u8 LCD_Init_Sequence_8Bit_CMD[] = { /* all the command is executed  */
	LCD_CMD_Function_4BitMode_2Line_5x7Dots,
	LCD_CMD_DisplayOff_CursorOff_BlinkOff,
	LCD_CMD_Clear_Display,
	LCD_CMD_Entry_IncCursor_NoDisplayShift,
	LCD_CMD_DisplayOn_CursorOn_BlinkOff,
};

/* ==================================================================================== */


/* object from the cfg.c file */
extern const LCD_cfg_t lcd_cfg ;

/* scheduler object*/
SCHED_task_t const task_lcd = {
		.runnable = &LCD_Task,
		.periodicTimeMs = 2,
};


/* ===================================== private functions ===================================== */

static u8 LCD_IsQueueFull(void)
{
	return (queueCurrentLen == LCD_QUEUE_MAX_LEN);
}

static u8 LCD_IsQueueEmpty(void)
{
	return (queueCurrentLen == 0);
}

static u8 LCD_PushCMD(u8 cmd)
{
	u8 status;

	if (!LCD_IsQueueFull()) /* if queue is not full (can still accept data) */
	{
		queueCurrentLen++; /* increase queue length indicator */

		/* add command to the last queue element */
		LCD_Queue[queue_last].cmd  = cmd;

		/* set queue element type to CMD */
		LCD_Queue[queue_last].type = LCD_QUEUE_OBJECT_TYPE_CMD;

		/* point at next empty queue location */
		queue_last++;
		if (queue_last == LCD_QUEUE_MAX_LEN) /* circular increment */
		{
			queue_last = 0;
		}

		status = LCD_STATUS_OK;
	}
	else
	{
		status = LCD_STATUS_NOK;
	}

	return status;
}

static u8 LCD_PushData(u8 *data, u8 len)
{
	u8 status;
	if (!LCD_IsQueueFull() && (len < LCD_DATA_MAX_LEN)) /* if queue is not full (can still accept data) */
	{                                                   /* and data length is accepted */
		/* increase queue length indicator */
		queueCurrentLen++;

		/* add data and length to the last queue element */
		/* copy data to internal buffer */
		for (u8 i = 0; i < len; i++)
		{
			LCD_Queue[queue_last].data[i] = data[i];
		}
		LCD_Queue[queue_last].len  = len;

		/* set queue element type to data */
		LCD_Queue[queue_last].type = LCD_QUEUE_OBJECT_TYPE_DATA;

		/* point at next empty queue location */
		queue_last++;
		if (queue_last == LCD_QUEUE_MAX_LEN) /* circular increment */
		{
			queue_last = 0;
		}

		status = LCD_STATUS_OK;
	}
	else
	{
		status = LCD_STATUS_NOK;
	}

	return status;
}

static void LCD_Pop(void)
{
	if (!LCD_IsQueueEmpty()) /* is queue still has data (not empty) */
	{
		/* decrease queue length indicator */
		queueCurrentLen--;

		/* point at next available queue element */
		queue_current++;
		if (queue_current == LCD_QUEUE_MAX_LEN) /* circular increment */
		{
			queue_current = 0;
		}
	}
	else /* is queue is empty */
	{

	}
}

static void LCD_Init(void)
{
	switch (LCD_Stage_Init_EarlyStage)
	{
	case LCD_Stage_Init_EarlyStage_GPIO:
		/* init all GPIO pins */
		for (u8 i = 0; i < 4; i++)
		{
			GPIO_InitPin(&lcd_cfg.lcd_IO_DB[i]);
		}
		GPIO_InitPin(&lcd_cfg.lcd_IO_RS);
		GPIO_InitPin(&lcd_cfg.lcd_IO_RW);
		GPIO_InitPin(&lcd_cfg.lcd_IO_E);

		/* clear E pin */
		GPIO_WritePin(&lcd_cfg.lcd_IO_E, GPIO_PIN_ALL_VALUE_LOW);

		/* switch stage to power delay */
		LCD_Stage_Init_EarlyStage = LCD_Stage_Init_EarlyStage_Power_Delay;

		//fallthrough

		/* no break to immediately jump to next stage */

	case LCD_Stage_Init_EarlyStage_Power_Delay: /* early power on delay (60 ms) */
		LCD_DelayCtr++;
		if (LCD_DelayCtr == LCD_INIT_POWER_DELAY_MAX_CTR_VALUE) /* if power on delay is finished */
		{
			/* reset the delay ctr and switch stage to init sequence */
			LCD_DelayCtr              = 0;
			LCD_Stage_Init_EarlyStage = LCD_Stage_Init_EarlyStage_InitSequence;
		}
		else /* if power on delay is still happening */
		{

		}
	break;

	case LCD_Stage_Init_EarlyStage_InitSequence:
		switch (LCD_Stage_Init_CMD_BitMode)
		{
		case LCD_Stage_Init_CMD_BitMode4:
			/* if the 1st set of 4-bit commands are not finished */
			if ( LCD_CMD_Data_Idx < (sizeof(LCD_Init_Sequence_4Bit_CMD) / sizeof(LCD_Init_Sequence_4Bit_CMD[0])) )
			{
				if (LCD_Nibble_Ctr < LCD_NIBBLE_STAGE_COUNT) /* if we're still writing the same nibble */
				{
					LCD_WriteOnBus(LCD_RS_COMMAND_MODE, HIGHER_4_BITS(LCD_Init_Sequence_4Bit_CMD[LCD_CMD_Data_Idx]));
					LCD_Nibble_Ctr++;
				}
				else /* if nibble is completely written, then delay by 6ms */
				{
					LCD_DelayCtr++;
					if (LCD_DelayCtr == LCD_INIT_NIBBLE_DELAY_MAX_CTR_VALUE) /* if delay is finished */
					{
						LCD_DelayCtr   = 0; /* reset the delay counter */
						LCD_Nibble_Ctr = 0; /* reset nibble counter */
						LCD_CMD_Data_Idx++; /* point at next command */
					}
					else /* if delay is still happening */
					{

					}
				}
			}
			else /* if the 1st set of 4-bit commands are finished */
			{
				/* reset command/data indexer and switch to 8-bit CMDs mode */
				LCD_CMD_Data_Idx           = 0;
				LCD_Stage_Init_CMD_BitMode = LCD_Stage_Init_CMD_BitMode8;
			}
		break;

		case LCD_Stage_Init_CMD_BitMode8:
			/* if the 2nd set of 8-bit commands are not finished */
			if ( LCD_CMD_Data_Idx < (sizeof(LCD_Init_Sequence_8Bit_CMD) / sizeof(LCD_Init_Sequence_8Bit_CMD[0])) )
			{
				switch (LCD_Nibble_Stage)
				{
				case LCD_Nibble_Stage_High:
					LCD_WriteOnBus(LCD_RS_COMMAND_MODE, HIGHER_4_BITS(LCD_Init_Sequence_8Bit_CMD[LCD_CMD_Data_Idx]));
					LCD_Nibble_Ctr++;

					if (LCD_Nibble_Ctr == LCD_NIBBLE_STAGE_COUNT) /* if nibble is completely written */
					{
						LCD_Nibble_Ctr   = 0;                    /* reset nibble counter */
						LCD_Nibble_Stage = LCD_Nibble_Stage_Low; /* point at low nibble */
					}
					else /* if nibble is still being written (early check) */
					{

					}
				break;

				case LCD_Nibble_Stage_Low:
					LCD_WriteOnBus(LCD_RS_COMMAND_MODE, LOWER_4_BITS(LCD_Init_Sequence_8Bit_CMD[LCD_CMD_Data_Idx]));
					LCD_Nibble_Ctr++;

					if (LCD_Nibble_Ctr == LCD_NIBBLE_STAGE_COUNT) /* if nibble is completely written */
					{
						LCD_Nibble_Ctr   = 0;                     /* reset nibble counter */
						LCD_Nibble_Stage = LCD_Nibble_Stage_High; /* point at high nibble */
						LCD_CMD_Data_Idx++;                       /* point at next CMD */
					}
					else /* if nibble is still being written (early check) */
					{

					}
				break;
				}
			}
			else /* if 2nd set of 8-bit commands are finished (everything is finished) */
			{
				LCD_Stage_Init_CMD_BitMode = LCD_Stage_Init_CMD_BitMode4; /* reset CMD bit mode back to 4 */
				LCD_CMD_Data_Idx           = 0;                           /* reset CMD/Data indexer */
				LCD_Stage_Task             = LCD_Stage_Task_StageIdle;    /* switch task stage to idle */

				/* we don't need this because at the last command, at the last nibble when it's finished
				 * the counter is reset back to point at high nibble */
				//LCD_Nibble_Stage = LCD_Nibble_Stage_High; /* point at high nibble */
			}
		break;
		}
	break;
	}
}

static void LCD_WriteCMD_Internal(u64 cmd)
{
	switch (LCD_Nibble_Stage)
	{
	case LCD_Nibble_Stage_High:
		LCD_WriteOnBus(LCD_RS_COMMAND_MODE, HIGHER_4_BITS(cmd));
		LCD_Nibble_Ctr++;

		if (LCD_Nibble_Ctr == LCD_NIBBLE_STAGE_COUNT) /* if higher nibble is completely written */
		{
			LCD_Nibble_Ctr   = 0;                    /* reset nibble counter */
			LCD_Nibble_Stage = LCD_Nibble_Stage_Low; /* point at low nibble */
		}
		else /* if nibble is still being written (early check) */
		{

		}
	break;

	case LCD_Nibble_Stage_Low:
		LCD_WriteOnBus(LCD_RS_COMMAND_MODE, LOWER_4_BITS(cmd));
		LCD_Nibble_Ctr++;

		/* if lower nibble is completely written (all the command is written) */
		if (LCD_Nibble_Ctr == LCD_NIBBLE_STAGE_COUNT)
		{
			LCD_Nibble_Ctr   = 0;                     /* reset nibble counter */
			LCD_Nibble_Stage = LCD_Nibble_Stage_High; /* point at high nibble */

			/* set the LCD state to pop queue */
			LCD_Stage_Task   = LCD_Stage_Task_StagePopQueue;

			/* call CMD callback if it's registered */
			if (LCD_CMD_CB)
			{
				LCD_CMD_CB();
			}
			else /* if callback is empty */
			{

			}
		}
		else /* if nibble is still being written (early check) */
		{

		}
	break;
	}
}

static void LCD_WriteData_Internal(u8 *data, u8 len)
{
	if (LCD_CMD_Data_Idx < len) /* if data indexer didn't reach length */
	{
		switch (LCD_Nibble_Stage)
		{
		case LCD_Nibble_Stage_High:
			LCD_WriteOnBus(LCD_RS_DATA_MODE, HIGHER_4_BITS(data[LCD_CMD_Data_Idx]));
			LCD_Nibble_Ctr++;

			if (LCD_Nibble_Ctr == LCD_NIBBLE_STAGE_COUNT) /* if nibble is completely written */
			{
				LCD_Nibble_Ctr   = 0;                    /* reset nibble counter */
				LCD_Nibble_Stage = LCD_Nibble_Stage_Low; /* point at low nibble */
			}
			else /* if nibble is still being written (early check) */
			{

			}
		break;

		case LCD_Nibble_Stage_Low:
			LCD_WriteOnBus(LCD_RS_DATA_MODE, LOWER_4_BITS(data[LCD_CMD_Data_Idx]));
			LCD_Nibble_Ctr++;

			if (LCD_Nibble_Ctr == LCD_NIBBLE_STAGE_COUNT) /* if nibble is completely written */
			{
				LCD_Nibble_Ctr   = 0;                     /* reset nibble counter */
				LCD_Nibble_Stage = LCD_Nibble_Stage_High; /* point at high nibble */
				LCD_CMD_Data_Idx++;
			}
			else /* if nibble is still being written (early check) */
			{

			}
		break;
		}
	}
	else /* if data indexer reached length */
	{
		/* TODO: this takes a complete tick time */

		/* reset data indexer */
		LCD_CMD_Data_Idx = 0;

		/* set the LCD state to pop queue */
		LCD_Stage_Task   = LCD_Stage_Task_StagePopQueue;

		/* call data callback if it's registered */
		if (LCD_Data_CB)
		{
			LCD_Data_CB();
		}
		else /* if callback is empty */
		{

		}

		/* we don't need this because at the last command, at the last nibble when it's finished
		 * the counter is reset back to point at high nibble */
		//LCD_Nibble_Stage = LCD_Nibble_Stage_High; /* point at high nibble */
	}
}

static void LCD_WriteOnBus(u64 mode, u8 cmd_or_data)
{
	switch (LCD_Stage_WriteOnBus)
	{
	case LCD_Stage_WriteOnBus_Stage_DataSetup:
		/* 01- E        = 1 */
		GPIO_WritePin(&lcd_cfg.lcd_IO_E, GPIO_PIN_ALL_VALUE_HIGH);

		/* 02- RS       = mode */
		GPIO_WritePin(&lcd_cfg.lcd_IO_RS, mode);

		/* 03- RW       = WRITE_MODE (0) */
		GPIO_WritePin(&lcd_cfg.lcd_IO_RW, LCD_RW_WRITE_MODE);

		/* 04- DATA_BUS = LOWER/HIGHER_4_BITS(cmd_or_data) */
		for (u8 i = 0; i < 4; i++) /* in the 1st 4 bits of 'cmd_or_data' */
		{
			if (cmd_or_data & 1) /* if bit is 1 */
			{
				GPIO_WritePin(&lcd_cfg.lcd_IO_DB[i], GPIO_PIN_ALL_VALUE_HIGH);
			}
			else /* if bit is 0 */
			{
				GPIO_WritePin(&lcd_cfg.lcd_IO_DB[i], GPIO_PIN_ALL_VALUE_LOW);
			}

			cmd_or_data >>= 1; /* discard 1st bit */
		}

		LCD_Stage_WriteOnBus = LCD_Stage_WriteOnBus_Stage_DataHold;
	break;

	case LCD_Stage_WriteOnBus_Stage_DataHold:
		/* 06- E        = 0 */
		GPIO_WritePin(&lcd_cfg.lcd_IO_E, GPIO_PIN_ALL_VALUE_LOW);

		LCD_Stage_WriteOnBus = LCD_Stage_WriteOnBus_Stage_DataSetup;
	break;
	}

	return;
}

/* triggered every 2ms */
static void LCD_Task(void)
{
	switch (LCD_Stage_Task)
	{
	case LCD_Stage_Task_StageInit:
		LCD_Init();
	break;

	case LCD_Stage_Task_StageWriteCMD:
		LCD_WriteCMD_Internal(LCD_UserCMD);
	break;

	case LCD_Stage_Task_StageWriteData:
		LCD_WriteData_Internal(LCD_UserData, LCD_UserLen);
	break;

	case LCD_Stage_Task_StagePopQueue:
		LCD_Pop();

		LCD_Stage_Task = LCD_Stage_Task_StageIdle;

		//fallthrough

		/* no break to immediately jump to next stage (avoid wasting tick time) */

	case LCD_Stage_Task_StageIdle:
		if (!LCD_IsQueueEmpty()) /* if queue has data */
		{
			switch (LCD_Queue[queue_current].type)
			{
			case LCD_QUEUE_OBJECT_TYPE_CMD:
				LCD_Stage_Task = LCD_Stage_Task_StageWriteCMD;
				LCD_UserCMD    = LCD_Queue[queue_current].cmd;
				LCD_WriteCMD_Internal(LCD_UserCMD);
			break;

			case LCD_QUEUE_OBJECT_TYPE_DATA:
				LCD_Stage_Task = LCD_Stage_Task_StageWriteData;
				LCD_UserData   = LCD_Queue[queue_current].data;
				LCD_UserLen    = LCD_Queue[queue_current].len;
				LCD_WriteData_Internal(LCD_UserData, LCD_UserLen);
			break;
			}
		}
		else /* if queue is empty */
		{
			/* do nothing, as if task is suspended */
		}
	break;
	}
}

/* val = 0b00000001
 * out = 0x00000000F
 * VAL = 0b00000101
 * out = 0x000000F0F*/
/*static u64 LCD_RemapGPIO_Value(u8 val)
{
	s8 i;
	u64 out = 0;
	for (i = 7; i >= 0; i--)
	{
		out <<= 4;
		if ((val >> i) & (1))
		{
			out |= 0xF;
		}
		else
		{

		}
	}

	return out;
}*/

/* ============================================================================================= */


/* ===================================== public APIs ===================================== */

void LCD_RegisterCMD_Callback(LCD_CMD_CB_t cmdCB)
{
	LCD_CMD_CB = cmdCB;
}

void LCD_RegisterData_Callback(LCD_Data_CB_t dataCB)
{
	LCD_Data_CB = dataCB;
}

u8 LCD_WriteCMD(u8 cmd)
{
	return LCD_PushCMD(cmd);
}

u8 LCD_WriteData(u8* data, u8 len)
{
	return LCD_PushData(data, len);
}

/* ======================================================================================= */
