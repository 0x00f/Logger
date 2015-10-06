/*********************************************************************************
 * 8888888888P        d8888 8888888b.  888b     d888 | Zentrum für
 *       d88P        d88888 888   Y88b 8888b   d8888 | Angewandte
 *      d88P        d88P888 888    888 88888b.d88888 | Raumfahrtechnologie
 *     d88P        d88P 888 888   d88P 888Y88888P888 | und Mikrogravitation
 *    d88P        d88P  888 8888888P"  888 Y888P 888 |--------------------------
 *   d88P        d88P   888 888 T88b   888  Y8P  888 | Center of Applied Space
 *  d88P        d8888888888 888  T88b  888   "   888 | Technology and
 * d8888888888 d88P     888 888   T88b 888       888 | Microgravity
 */
/*****************************************************************************//**
 *  @file       system.h
 *	@author 	Michael A. Schulze
 *	@email		m.schulze@uni-bremen.de
 *	@version	v1.0, compiled with arm-none-eabi-gcc v4.9.3 20150303
 *
 *  @defgroup   system
 *  @{
 *  @brief      Implements system init(), post() and NVSRAM access functions
 *
 *              All reads and write to NVSRAM are handled here.
 ********************************************************************************/

#ifndef SYSTEM_H
#define SYSTEM_H

/* C++ detection */
#ifdef __cplusplus
extern C {
#endif

#include "tm_stm32f4_bkpsram.h"
#include "tm_stm32f4_rtc.h"
#include "MAX11060.h"
#include "file_handler.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "stdbool.h"


/* These are the addresses of Data saved in the Backup NVSRAM */
/* Sample rate is stored as a uint16_t with values between 250 and 64000 */
#define  sample_rate_address 		   0x00
/* Used as an 8 bit long bit array for CH7 (MSB) to CH0 (LSB)
 * If set the channel is logged, if not, it is not logged
 */
#define  logged_channels_address	   0x02
/* The measurement name is a char array with a maximum size of 20 chars */
#define  measurement_name_address 	   0x03
/* The maximum number of characters in the filename (20 chars)*/
#define  max_filename_len              0x14

/**
 * Enum of possible system states. They are cycled in state machine fashion
 * Startup is the state after power up and before init() and post() functions
 * are
 */
typedef enum {
	STARTUP,
	POST,
	STANDBY,
	HW_FAULT
}SYSTEM_STATE;


/**
 * @brief  .
 * A single function to initialize the system, all peripherals and GPIO with
 * a single function call.
 * @param  None
 * @retval current state from SYSTEM_STATE enum
 */
uint16_t Get_Sample_Rate();


MAX11060_ADC_SampleRate_t Get_Sample_Rate_Conf();

/**
 * @brief  System Init().
 * A single function to initialize the system, all peripherals and GPIO with
 * a single function call.
 * @param  None
 * @retval current state from SYSTEM_STATE enum
 */
void Set_Sample_Rate(uint16_t sample_rate);


/**
 * @brief  System Init().
 * A single function to initialize the system, all peripherals and GPIO with
 * a single function call.
 * @param  None
 * @retval current state from SYSTEM_STATE enum
 */
SYSTEM_STATE Logger_System_Init();


/**
 * @brief  POST - Power On Self Test Checks that SD card is mounted and ADC's
 *         are properly configured and responding This is called after
 *         Logger_System_Init()!
 * @param  None
 * @retval HW_FAULT if hardware does not respond as expected
 * @retval STANDYBY if POST is successful
 */
SYSTEM_STATE Logger_System_POST();


/**
 * @brief  Generates a filename string from user configurable NVSRAM data
 * @param  Pointer to char array where the data will be written
 * @retval number of characters in the generated filename string
 */
uint8_t Get_Measurement_Name(char *dest_buffer);


/**
 * @brief  Writes new filename string into the NVSRAM
 * @param  Pointer to array containing '\0' terminated string to be written
 * @retval Number of characters written to NVSRAM.
 */
uint8_t Set_Measurement_Name(const char *name);


/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_H */

