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
 *  @file       main.h
 *	@author 	Michael A. Schulze
 *	@email		m.schulze@uni-bremen.de
 *	@version	v1.0, compiled with arm-none-eabi-gcc v4.9.3 20150303
 *
 *  @defgroup   main
 *  @{
 *  @brief      Main thread of the system. External user input and adc data
 *              are processed here
 *
 *              This is an interrupt driven loop, with user input toggling
 *              between sampling and idle, and the ADCs signaling the availability
 *              of new data which is then buffered and written to flash.
 ********************************************************************************/

#ifndef MAIN_H
#define MAIN_H

/* C++ detection */
#ifdef __cplusplus
extern C {
#endif

#include "tm_stm32f4_rtc.h"
#include "tm_stm32f4_fatfs.h"
#include "stdlib.h"
#include "stdint.h"
#include "stdbool.h"

extern const char FW_VERSION[];
extern TM_RTC_Time_t datatime;
extern FATFS FatFs;


#define UNITY_OUTPUT_CHAR(a) SH_SendChar(a)
//#define UNITY_OUTPUT_CHAR(a) PutChar(a)

/**
 * This structure is used to store date before it is written to external flash.
 */
typedef struct {
	uint8_t buffer[32768] __attribute__ ((aligned (4))) ;	/* Working buffer */
	uint16_t w_index;
	bool isReady;
} Buffer;



/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif /* MAIN_H */


