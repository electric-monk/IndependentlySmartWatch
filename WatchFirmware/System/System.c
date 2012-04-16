#include <FreeRTOS.h>
#include <task.h>
#include "System.h"
#include "hal_lcd.h"
#include <string.h>
#include "bttask.h"

typedef struct
{
  unsigned char Command;
  tLcdLine line;
} __attribute__((__packed__)) TestLcdCommand;

void InitialiseSystem(void)
{
	int i;
	TestLcdCommand command;

	memset(&command, 0, sizeof(command));
	command.Command = LCD_WRITE_CMD;
	for (i = 0; i < 96; i++)
	{
		command.line.Row = i + FIRST_LCD_LINE_OFFSET;
		command.line.Data[i / 8] |= 1 << (i % 8);
		WriteLcd((unsigned char*)&command, sizeof(command));
	}
}
