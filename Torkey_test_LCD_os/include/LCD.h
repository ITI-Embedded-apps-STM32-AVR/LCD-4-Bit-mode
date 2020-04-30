/*
 *
 *  Created on: Apr 2, 2020
 *      Author: SARA_MINA
 */

#ifndef LCD_H_
#define LCD_H_

#define LCD_STATUS_OK  0
#define LCD_STATUS_NOK 1

#define LCD_CMD_Clear_Display                      0x01

#define LCD_CMD_Return_Home 			               0x02

#define LCD_CMD_Entry_DecCursor_NoDisplayShift 	   0x04
#define LCD_CMD_Entry_DecCursor_DisplayShift 	   0x05
#define LCD_CMD_Entry_IncCursor_NoDisplayShift 	   0x06
#define LCD_CMD_Entry_IncCursor_DisplayShift       0x07

#define LCD_CMD_DisplayOff_CursorOff_BlinkOff      0x08
#define LCD_CMD_DisplayOff_CursorOff_BlinkOn       0x09
#define LCD_CMD_DisplayOff_CursorOn_BlinkOff       0x0A
#define LCD_CMD_DisplayOff_CursorOn_BlinkOn        0x0B
#define LCD_CMD_DisplayOn_CursorOff_BlinkOff       0x0C
#define LCD_CMD_DisplayOn_CursorOff_BlinkOn        0x0D
#define LCD_CMD_DisplayOn_CursorOn_BlinkOff        0x0E
#define LCD_CMD_DisplayOn_CursorOn_BlinkOn         0x0F

#define LCD_CMD_ShiftCursor_Left                   0x10
#define LCD_CMD_ShiftCursor_Right                  0x14
#define LCD_CMD_ShiftDisplay_Left                  0x18
#define LCD_CMD_ShiftDisplay_Right                 0x1C

#define LCD_CMD_Function_4BitMode_1Line_5x7Dots    0x20
#define LCD_CMD_Function_4BitMode_1Line_5x10Dots   0x24
#define LCD_CMD_Function_4BitMode_2Line_5x7Dots    0x28
#define LCD_CMD_Function_4BitMode_2Line_5x10Dots   0x2C
#define LCD_CMD_Function_8BitMode_1Line_5x7Dots    0x30
#define LCD_CMD_Function_8BitMode_1Line_5x10Dots   0x34
#define LCD_CMD_Function_8BitMode_2Line_5x7Dots    0x38
#define LCD_CMD_Function_8BitMode_2Line_5x10Dots   0x3C


#define LCD_CMD_Set_CGRAM_Addr                     0x40
#define LCD_CMD_Set_DDRAM_Addr                     0x80


/**
 * @brief LCD command callback type.
 *
 */
typedef void (*LCD_CMD_CB_t)(void);

/**
 * @brief LCD data callback type.
 *
 */
typedef void (*LCD_Data_CB_t)(void);


void LCD_RegisterCMD_Callback(LCD_CMD_CB_t cmdCB);

void LCD_RegisterData_Callback(LCD_Data_CB_t dataCB);


u8 LCD_WriteCMD(u8 cmd);

u8 LCD_WriteData(u8* data, u8 len);

#endif /* LCD_H_ */

