/*
 * 8888888888P        d8888 8888888b.  888b     d888 | Zentrum für
 *       d88P        d88888 888   Y88b 8888b   d8888 | Angewandte
 *      d88P        d88P888 888    888 88888b.d88888 | Raumfahrtechnologie
 *     d88P        d88P 888 888   d88P 888Y88888P888 | und Mikrogravitation
 *    d88P        d88P  888 8888888P"  888 Y888P 888 |--------------------------
 *   d88P        d88P   888 888 T88b   888  Y8P  888 | Center of Applied Space
 *  d88P        d8888888888 888  T88b  888   "   888 | Technology and
 * d8888888888 d88P     888 888   T88b 888       888 | Microgravity
 *
 *	@author 	Michael A. Schulze
 *	@email		m.schulze@uni-bremen.de
 *	@version	v1.0
 *	@gcc		v4.9.3 20150303 arm-none-eabi-gcc
 *	@ide		CooCox CoIDE v2.0.2
 */

/* ST StdPeripherial */
#include "stm32f4xx.h"
/* Newly created libraries */
#include "main.h"
#include "system.h"
#include "led.h"
#include "file_handler.h"
#include "MAX11060.h"
/* Test Framework */
#include <Unity/src/unity_internals.h>
#include <Unity/src/unity.h>
/* Included TM libraries */
#include "defines.h"
#include "tm_stm32f4_general.h"
#include "tm_stm32f4_delay.h"
#include "tm_stm32f4_gpio.h"
#include "tm_stm32f4_disco.h"
#include "tm_stm32f4_fatfs.h"
#include "tm_stm32f4_spi.h"
#include "tm_stm32f4_spi_dma.h"
#include "tm_stm32f4_usart.h"
#include "tm_stm32f4_usart_dma.h"
#include "tm_stm32f4_bkpsram.h"
#include "tm_stm32f4_exti.h"
#include "tm_stm32f4_adc.h"
/* GCC Includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <setjmp.h>



///* Fatfs object - this is the file system which will be mounted */
FATFS FatFs;
///* File object - used for individual files*/
FIL fil;


/*
 * These are the storage buffers where data is stored
 * until it is written to the external flash memory card
 */
Buffer data_buffer_A;
Buffer data_buffer_B;
Buffer data_buffer_C;

/* Pointers to the data buffers */
volatile Buffer *ACTIVE_BUFFER;
volatile Buffer *PASSIVE_BUFFER;

/* This lets the program know how much data
 * to allocate for each sample set
 */
uint8_t data_packet_size;

/* Variable to store the RTC data */
TM_RTC_Time_t datatime;


const char FW_VERSION[] = "3.0.0-01.10.2015";

char buf[50], buf2[50];

uint8_t output_buf[80];

uint8_t sprintf_buf[80];


/* Stores any received serial data */
char ser_input_buffer[50];
uint8_t ser_input_cntr = 0;

volatile uint8_t counter = 0;

uint32_t write_counter = 0;

/* These defines are needed for the data format function which encodes the
 * binary ADC data into ASCII for the log file. It needs to know how long the string that
 * is generated from the data is.
 * Example: 01.01.04.0588,         1,    -9,   -20,     0,  7122,   -27,   -40,   -38,   -32
 * This string requires 81 characters, including any spaces and disregarding the final \0.
 */
#ifdef DATA_PACKET_SIZE_8CH
  static int const data_packet_size = DATA_PACKET_SIZE_8CH;
#else
  static int const data_packet_size_8ch = 81; // default value
#endif



/* Flag variables, declared volatile as they are changed in interrupt
 * routines, which may be broken when optimizations are enabled.
 */
volatile bool sampling_state = false;
volatile bool transfer_data_flag = false;
volatile uint32_t sample_counter = 0;


static void error_blink();

static void switch_active_buffer();

/* Unity Tests - defined in myTests.c
 * Can be extended as needed, however output of the tests is only visible
 * via semihosting when a debugger is attafhed
 */
extern void testWillAlwaysPass(void);
extern void testWillAlwaysFail(void);
extern void memcpyTest(void);
extern void switchActiveBufferTest(void);

void runTest(UnityTestFunction test) {
	if(TEST_PROTECT()) {
		test();
	}
}


int main(void) {
	/* Initialize the data buffers */
	data_buffer_A.isReady = true;
	data_buffer_A.w_index = 0;
	data_buffer_B.isReady = true;
	data_buffer_B.w_index = 0;
	data_buffer_C.isReady = true;
	data_buffer_C.w_index = 0;

	/* Set buffer pointers */
	ACTIVE_BUFFER = &data_buffer_A;
	PASSIVE_BUFFER = &data_buffer_B;

	SYSTEM_STATE status;
	/* Initialize the internal and external HW */
	status = Logger_System_Init();

	/* Test that external HW is ready and responding */
	status = Logger_System_POST();

	if(status == HW_FAULT) {
		TM_USART_Puts(USART1, "HW FAULT!\n");
		error_blink();
	}
	else {
		TM_USART_Puts(USART1, "POST success!\n");
	}



	//---------------------------------------
	UnityBegin("myTests.c");
	RUN_TEST(testWillAlwaysPass, __LINE__);
	RUN_TEST(testWillAlwaysFail, __LINE__);
	RUN_TEST(memcpyTest, __LINE__);
	//RUN_TEST(switchActiveBufferTest, __LINE__);
	UnityEnd();
	//--------------------------------------


    /* Attach interrupt on pin PA0 = External line 0 . This starts and stops a sample run */
    if (TM_EXTI_Attach(GPIOA, GPIO_Pin_0, TM_EXTI_Trigger_Rising) == TM_EXTI_Result_Ok) {
    	TM_USART_Puts(USART1, "Sampling start/stop interrupt pin connected\n");
    }

    /* Attach interrupt on pin PE15. A falling edge = adc data ready. This creates an interrupt
     * whenever the ADC has new data.
     */
    if (TM_EXTI_Attach(GPIOE, GPIO_Pin_15, TM_EXTI_Trigger_Falling) == TM_EXTI_Result_Ok) {
    	TM_USART_Puts(USART1, "ADC Data Ready interrupt pin connected\n");
    }


    /* Turn on ALL leds */
    TM_DISCO_LedOn(LED_GREEN | LED_RED | LED_BLUE | LED_ORANGE);
    Delayms(1000);
    /* Turn off ALL leds */
    TM_DISCO_LedOff(LED_GREEN | LED_RED | LED_BLUE | LED_ORANGE);
    SET_STATUS_LED(RED);
    Delayms(200);
    SET_STATUS_LED(GREEN);
    Delayms(200);
    SET_STATUS_LED(BLUE);
    Delayms(1000);
	TM_GPIO_SetPinLow(STATUS_LED_PORT, STATUS_LED_RED);
	TM_GPIO_SetPinHigh(STATUS_LED_PORT, STATUS_LED_GREEN);
	TM_GPIO_SetPinLow(STATUS_LED_PORT, STATUS_LED_BLUE);



	 /* Mount Drive */
	while(f_mount(&FatFs, "", 1)==FR_OK) {

		//Mounted OK, turn on RED LED
		TM_DISCO_LedIsOn(LED_RED);

		// Try to open file
		if(f_open(&fil, Gen_Filename(), FA_OPEN_ALWAYS | FA_WRITE ) == FR_OK) {
			f_sync(&fil);
			// File opened, turn off RED and turn on GREEN led
			TM_DISCO_LedOn(LED_GREEN);
			TM_DISCO_LedOff(LED_RED);

		    /* Spinlock here, waiting for sampling to begin */
		    while(sampling_state == false);

		    /* This initial read clears the current ADC data, which sends the !DTRDY signal
		     * high. The next time data is ready this signal will fall, triggering the
		     * interrupt routine. All subsequent reads occur in the interrupt routine
		     */
			MAX11060_ADC_ReadAll(NULL, 0x00, 24);

				while(sampling_state == true) {

					if(transfer_data_flag == true) {

							TM_DISCO_LedOn(LED_BLUE);
							f_write(&fil, PASSIVE_BUFFER->buffer, sizeof(PASSIVE_BUFFER->buffer), &write_counter);
							TM_DISCO_LedOff(LED_BLUE);

							PASSIVE_BUFFER->w_index = 0;
							PASSIVE_BUFFER->isReady = true;
							transfer_data_flag = false;
							f_sync(&fil);
					}
				}

				/* Turn on all leds - success! */
				TM_DISCO_LedOn(LED_GREEN | LED_RED | LED_ORANGE | LED_BLUE);
				sample_counter = 0;

			// close file, don't forget this
			f_close(&fil);
			// Arbitrary delay so we can see led status leds before beginning new measurement
			Delayms(1000);

			TM_DISCO_LedOff(LED_GREEN | LED_RED | LED_ORANGE | LED_BLUE);
			/* Reset buffers and clear out any old data */
			PASSIVE_BUFFER->w_index = 0;
			PASSIVE_BUFFER->isReady = true;
			ACTIVE_BUFFER->w_index = 0;
			ACTIVE_BUFFER->isReady = true;
		}
		write_counter = 0;
	}
	/* We should never reach here - error blink - indicated SD is not mounted*/
	error_blink();

	return 0;
}



void TM_EXTI_Handler(uint16_t GPIO_Pin) {

	//bool status = true;
	TM_RTC_GetDateTime(&datatime, TM_RTC_Format_BIN);

	//uint16_t milliseconds = 1000 -((datatime.subseconds * 1000) / 1023 );
	uint8_t adc_data[24];
	uint8_t *adc_data_pntr = adc_data;

	volatile int16_t ADC_DATA_CH[8];

	uint8_t temp_buffer[100];
	/* Tells us how many chars are in the string the sprintf creates */
	volatile uint8_t num_bytes_generated = 0;

	/* used if data needs to be split between 2 separate buffers.
	 * We keep track of how much has been written, switch to a new
	 * buffer and fill in the remaining data there
	 */
	volatile uint8_t num_bytes_remaining = 0;
	volatile uint8_t num_bytes_to_write = 0;

	/* Handle external line 0 interrupts
	 * Used to start and stop sampling
	 * */
	if (GPIO_Pin == GPIO_Pin_0) {
		/* Toggle RED led */
		TM_DISCO_LedToggle(LED_GREEN);

		if((TM_GPIO_GetInputPinValue(GPIOA, GPIO_Pin_0)) == 1) {
			if(sampling_state == false) {
				sampling_state = true;
				TM_USART_DMA_Puts(USART1, "Now sampling\n");
			}
			else if (sampling_state == true) {
				sampling_state = false;
				TM_GPIO_SetPinLow(STATUS_LED_PORT, STATUS_LED_RED);
				TM_GPIO_SetPinHigh(STATUS_LED_PORT, STATUS_LED_GREEN);
				TM_USART_DMA_Puts(USART1, "Sampling finished\n");
			}
		}
	}

	/* Handle external line 15 interrupts
	 * Falling edge means ADC data is ready
	 * Only read and save if in sampling state
	 * */
	if ((GPIO_Pin == GPIO_Pin_15) & sampling_state) {
		/* Check that !DTRDY is really 0 and there really is new data */
		if((TM_GPIO_GetInputPinValue(GPIOE, GPIO_Pin_15)) == 0)
			TM_DISCO_LedOn(LED_ORANGE);
		sample_counter++;
		//TM_GPIO_SetPinHigh(DEBUG_LED_PORT, DEBUG_LED_1);
		/*Each ADC has 96bits of information, so to read the data from 2
		  ADCs will require 96bits * 2 = 192 / 8 = 24 reads */
		MAX11060_ADC_ReadAll(adc_data, 0x00, 24);

		ADC_DATA_CH[0] = (adc_data[0] << 8) | (adc_data[1]);
		ADC_DATA_CH[1] = (adc_data[3] << 8) | (adc_data[4]);
		ADC_DATA_CH[2] = (adc_data[6] << 8) | (adc_data[7]);
		ADC_DATA_CH[3] = (adc_data[9] << 8) | (adc_data[10]);
		ADC_DATA_CH[4] = (adc_data[12] << 8) | (adc_data[13]);
		ADC_DATA_CH[5] = (adc_data[15] << 8) | (adc_data[16]);
		ADC_DATA_CH[6] = (adc_data[18] << 8) | (adc_data[19]);
		ADC_DATA_CH[7] = (adc_data[21] << 8) | (adc_data[22]);

		/* Offset compensation */
//		ADC_DATA_CH[0] += 31;
//		ADC_DATA_CH[1] -= 37;
//		ADC_DATA_CH[2] += 46;
//		ADC_DATA_CH[3] += 14;
//		ADC_DATA_CH[4] += 10;
//		ADC_DATA_CH[5] += 27;
//		ADC_DATA_CH[6] += 0;
//		ADC_DATA_CH[7] += 110;

		/* Check specific channels for overvoltage conditions */
		if((ADC_DATA_CH[3] == 32767) | (ADC_DATA_CH[3] == -32768) /*| (ADC_DATA_CH[2] == 32767) | (ADC_DATA_CH[1] == 32767) | (ADC_DATA_CH[0] == 32767)*/)  {
			TM_DISCO_LedToggle(LED_RED);
		}
	}


	/* if we can fit all current data, the sprintf() will send its data directly to the buffer.
	 * Saves a memcpy() operation in comparison to the case where all data cannot fit, and the
	 * sprintf instead writes to a temp. buffer, and we must memcpy() the data to its respective
	 * buffers. */
	if(((ACTIVE_BUFFER->w_index) + data_packet_size_8ch) < sizeof(ACTIVE_BUFFER->buffer)) {
		/* sprintf for timestamp , sample number, and data for all 8 channels */
		num_bytes_generated = sprintf(&ACTIVE_BUFFER->buffer[ACTIVE_BUFFER->w_index],
				"%02d.%02d.%02d.%04d,%10d,%6" PRId16 ",%6" PRId16 ",%6" PRId16 ",%6" PRId16",%6"
				PRId16 ",%6" PRId16 ",%6" PRId16 ",%6"
				PRId16 "\n", datatime.hours, datatime.minutes,datatime.seconds, datatime.subseconds,
				sample_counter, ADC_DATA_CH[0],ADC_DATA_CH[1],ADC_DATA_CH[2],ADC_DATA_CH[3],ADC_DATA_CH[4],
				ADC_DATA_CH[5],ADC_DATA_CH[6],ADC_DATA_CH[7]);

		/* Increase the write pointer */
		ACTIVE_BUFFER->w_index += (num_bytes_generated);
	}

	/* if we can only fit partial data, write the formatted data to a temporary buffer, then
	 * memcpy() the data to the appropriate buffers
	 */
	else {
		/* sprintf for 8ch to a temporary buffer */
		num_bytes_generated = sprintf(temp_buffer, "%02d.%02d.%02d.%04d,%10d,%6" PRId16 ",%6" PRId16
				",%6" PRId16 ",%6" PRId16",%6" PRId16 ",%6" PRId16 ",%6" PRId16 ",%6"
				PRId16 "\n", datatime.hours, datatime.minutes,datatime.seconds, datatime.subseconds,
				sample_counter, ADC_DATA_CH[0],ADC_DATA_CH[1],ADC_DATA_CH[2],ADC_DATA_CH[3],ADC_DATA_CH[4],
				ADC_DATA_CH[5],ADC_DATA_CH[6],ADC_DATA_CH[7]);


		num_bytes_to_write = (sizeof(ACTIVE_BUFFER->buffer) - ACTIVE_BUFFER->w_index);
		/* What is left to write in temp_buffer is the total number of bytes from the sprintf
		 * minus what we have already written
		 */
		num_bytes_remaining = num_bytes_generated - num_bytes_to_write;
		/* copy enough bytes to fill the buffer  */
		memcpy(&ACTIVE_BUFFER->buffer[ACTIVE_BUFFER->w_index], temp_buffer, num_bytes_to_write);
		/* Buffer is full, ready for saving to SD */
		transfer_data_flag = true;
		/* Get new active buffer pointer */
		switch_active_buffer();
		/* Ensure we begin at start of array */
		ACTIVE_BUFFER->w_index = 0;
		/* Copy the remaining data to the new active buffer */
		memcpy(&ACTIVE_BUFFER->buffer[ACTIVE_BUFFER->w_index], &temp_buffer[num_bytes_to_write], num_bytes_remaining);

		ACTIVE_BUFFER->w_index += num_bytes_remaining;
	}

	TM_DISCO_LedOff(LED_ORANGE);



		/* DMM Mode - Serial voltage output. When this is active (uncommented), writes to the SD will fail,
		 * however all data is output in real time to the serial console. This is useful for
		 * live viewing of system data when testing a new sensor or debugging.
		 */
//		sprintf(output_buf, "%02d.%02d.%02d.%04d, %10d, %6" PRId16 ", %6" PRId16 ", %6" PRId16 ", %6" PRId16
//				", %6" PRId16 ", %6" PRId16 ", %6" PRId16 ", %6" PRId16 "\n", datatime.hours,
//				datatime.minutes,datatime.seconds, datatime.subseconds, sample_counter, ADC_DATA_CH[0],
//				ADC_DATA_CH[1],ADC_DATA_CH[2],ADC_DATA_CH[3],ADC_DATA_CH[4],ADC_DATA_CH[5],
//				ADC_DATA_CH[6],ADC_DATA_CH[7]);
//
//		while (TM_USART_DMA_Sending(USART1));
//		TM_USART_DMA_Puts(USART1, output_buf);

}

/**
 * @brief   Simple endless loop that flashed leds to signal an error
 * @param   None
 * @retval  None
 */
static void error_blink() {
	while(1) {
		Delayms(500);
		TM_DISCO_LedToggle(LED_GREEN | LED_RED | LED_ORANGE | LED_BLUE);
	}
}

/**
 * This function selects which buffer will be receiving new data once the
 * current buffer is full.
 */
static void switch_active_buffer() {


	if (ACTIVE_BUFFER == &data_buffer_A) {
		/* buffer A is now full, needs processing */
		data_buffer_A.isReady = false;
		if(data_buffer_B.isReady) {
			ACTIVE_BUFFER = &data_buffer_B;
			PASSIVE_BUFFER = &data_buffer_A;
			//TM_USART_Puts(USART1, "A->B\n");
		}
		else if(data_buffer_C.isReady) {
			ACTIVE_BUFFER = &data_buffer_C;
			PASSIVE_BUFFER = &data_buffer_A;
			//TM_USART_Puts(USART1, "A->C\n");
		}
		else {
			TM_USART_Puts(USART1, "All buffers full!\n");
			TM_GPIO_SetPinHigh(STATUS_LED_PORT, STATUS_LED_RED);
			TM_GPIO_SetPinLow(STATUS_LED_PORT, STATUS_LED_GREEN);
		}
	}

	else if(ACTIVE_BUFFER == &data_buffer_B) {
		/* buffer B is now full, needs processing */
		data_buffer_B.isReady = false;
		if(data_buffer_C.isReady) {
			ACTIVE_BUFFER = &data_buffer_C;
			PASSIVE_BUFFER = &data_buffer_B;
			//TM_USART_Puts(USART1, "B->C\n");
		}
		else if(data_buffer_A.isReady){
			ACTIVE_BUFFER = &data_buffer_A;
			PASSIVE_BUFFER = &data_buffer_B;
			//TM_USART_Puts(USART1, "B-A\n");
		}
		else {
			TM_USART_Puts(USART1, "All buffers full!\n");
			TM_GPIO_SetPinHigh(STATUS_LED_PORT, STATUS_LED_RED);
			TM_GPIO_SetPinLow(STATUS_LED_PORT, STATUS_LED_GREEN);
		}
	}

	else if(ACTIVE_BUFFER == &data_buffer_C) {
		/* buffer B is now full, needs processing */
		data_buffer_C.isReady = false;
		if(data_buffer_A.isReady) {
			ACTIVE_BUFFER = &data_buffer_A;
			PASSIVE_BUFFER = &data_buffer_C;
			//TM_USART_Puts(USART1, "C->A\n");
		}
		else if(data_buffer_B.isReady) {
			ACTIVE_BUFFER = &data_buffer_B;
			PASSIVE_BUFFER = &data_buffer_C;
			//TM_USART_Puts(USART1, "C->B\n");
		}
		else {
			TM_USART_Puts(USART1, "All buffers full!\n");
			TM_GPIO_SetPinHigh(STATUS_LED_PORT, STATUS_LED_RED);
			TM_GPIO_SetPinLow(STATUS_LED_PORT, STATUS_LED_GREEN);
		}
	}

}
