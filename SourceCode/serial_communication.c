/**
 * 8888888888P        d8888 8888888b.  888b     d888 | Zentrum für
 *       d88P        d88888 888   Y88b 8888b   d8888 | Angewandte
 *      d88P        d88P888 888    888 88888b.d88888 | Raumfahrtechnologie
 *     d88P        d88P 888 888   d88P 888Y88888P888 | und Mikrogravitation
 *    d88P        d88P  888 8888888P"  888 Y888P 888 |--------------------------
 *   d88P        d88P   888 888 T88b   888  Y8P  888 | Center of Applied Space
 *  d88P        d8888888888 888  T88b  888   "   888 | Technology and
 * d8888888888 d88P     888 888   T88b 888       888 | Microgravity
 */

#include "serial_communication.h"
#include "system.h"
#include "tm_stm32f4_adc.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <inttypes.h>


//! counts the number of received characters
static uint16_t ser_input_cntr = 0;
static uint16_t vbat = 0;
//! buffer for the received serial data
static char ser_input_buffer[512];
//! general buffer used to store formated strings for serial transmission
static char buf[80];

static
const char HelpMsg[] =
	"Available system commands:\n"
	" GET_RTC - Returns current RTC time\n"
	" SET_RTC [%02d.%02d.%02d %02d:%02d:%02d]Day.Month.Year H:M:S - Update the RTC time\n"
    " GET_VBK - Returns current voltage of the backup SRAM power source\n"
	" GET_FILENAME - Returns current filename string stored in SRAM\n"
	" SET_FILENAME [string] - Updates the filename string in SRAM\n"
	"\n";

/*
 * Custom USART receiver handler function
 * Called on receive interrupt
 */
void TM_USART1_ReceiveHandler(uint8_t c) {
	/* If we receive CR or LF, process the buffered data */
	if ((c == 0x0D) | (c == 0x0A)) {
		ser_input_buffer[ser_input_cntr] = c;
		ser_input_buffer[ser_input_cntr+1] = '\n';
		ser_input_cntr = 0;
		TM_GPIO_TogglePinValue(DEBUG_LED_PORT, DEBUG_LED_1);
		serial_command_handler();
	}
	/* Buffer is full, discard all stored data */
	else if (ser_input_cntr == sizeof(ser_input_buffer)) {
		ser_input_cntr = 0;
	}
	/* Just buffer the received char */
	else {
		ser_input_buffer[ser_input_cntr] = c;
		ser_input_cntr++;
	}
}

/*
 * The received buffer data is compared against known commands using strncmp().
 */
static void serial_command_handler() {

	if (!(strncmp("SET_RTC", ser_input_buffer, 7))) {
		/* Set new time */
		if (TM_RTC_SetDateTimeString(&ser_input_buffer[8]) == TM_RTC_Result_Ok) {
			TM_RTC_GetDateTime(&datatime, TM_RTC_Format_BIN);
			sprintf(buf, "New RTC time: %02d.%02d.%02d %02d:%02d:%02d\n", datatime.date,
					datatime.month, datatime.year, datatime.hours, datatime.minutes,
					datatime.seconds);

			//Send to USART
			TM_USART_DMA_Puts(USART1, buf);
		}
		else {
			TM_USART_DMA_Puts(USART1, "RTC format Error\n");
		}
	}

	else if (!(strncmp("GET_RTC", ser_input_buffer, 7))) {
		TM_RTC_GetDateTime(&datatime, TM_RTC_Format_BIN);
		sprintf(buf, "%02d.%02d.%02d %02d:%02d:%02d\n", datatime.date,
				datatime.month, datatime.year, datatime.hours, datatime.minutes,
				datatime.seconds);
		TM_USART_DMA_Puts(USART1, buf);
	}

	else if (!(strncmp("GET_VBK", ser_input_buffer, 7))) {
		TM_ADC_EnableVbat();
		vbat = TM_ADC_ReadVbat(ADC1);
		sprintf(buf, "%04d\n", vbat);
		TM_USART_DMA_Puts(USART1, buf);
	}

	else if (!(strncmp("GET_FWV", ser_input_buffer, 7))) {
		sprintf(buf, "%s\n", FW_VERSION);
		TM_USART_DMA_Puts(USART1, buf);
	}

	else if (!(strncmp("SET_SAMPLERATE", ser_input_buffer, 14))) {
		uint8_t i = 15;
		uint16_t value = 0;
		uint8_t c = ser_input_buffer[i];
		uint8_t ascii_buffer[7] = {0};

		while((c != 0x0D) & (c != 0x0A) & (i<(15+6))) {
			ascii_buffer[i-15] = c;
			i++;
			c = ser_input_buffer[i];
		}
		ascii_buffer[6] = '\0';
		value = atoi(ascii_buffer);
		Set_Sample_Rate(value);
		sprintf(buf, "New sample rate: %5" PRIu16 "\n", value);
		TM_USART_DMA_Puts(USART1, buf);
	}

	else if (!(strncmp("GET_SAMPLERATE", ser_input_buffer, 14))) {
		sprintf(buf, "Current sample rate: %5d\n", Get_Sample_Rate());
		TM_USART_DMA_Puts(USART1, buf);
	}

	else if (!(strncmp("GET_VOLUME", ser_input_buffer, 10))) {

		TM_USART_DMA_Puts(USART1, volume);
	}

	else if (!(strncmp("GET_CLUSTERSIZE", ser_input_buffer, 10))) {

		TM_USART_DMA_Puts(USART1, volume);
	}

	else if (!(strncmp("SET_FILENAME", ser_input_buffer, 12))) {
		/* i points to char after serial command which is first letter of filename */
		uint8_t i = 13;
		uint8_t c = ser_input_buffer[i];
		while((c != 0x0D) & (c != 0x0A) & (i<(13+20))) {
			buf[i-13] = c;
			i++;
			c = ser_input_buffer[i];
		}
		buf[i-13] = '\0';
		Set_Measurement_Name(buf);
		TM_USART_DMA_Puts(USART1, "New filename: ");
		TM_USART_DMA_Puts(USART1, buf);
		TM_USART_DMA_Puts(USART1, "\n");
	}

	else if (!(strncmp("GET_FILENAME", ser_input_buffer, 12))) {
		/* i points to char after serial command which is first letter of filename */
		uint8_t name_buffer[21]= {0};

		Get_Measurement_Name((char *)name_buffer);
		sprintf(buf, "Current filename: %s\n", name_buffer);
		TM_USART_DMA_Puts(USART1, buf);

	}

	/* Input command not recognized - discard buffer */
	else {
		TM_USART_DMA_Puts(USART1, "Command error\n");
		ser_input_cntr = 0;
	}
}
