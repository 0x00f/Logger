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
/*****************************************************************************//**
 *  @file       serial_communication.h
 *	@author 	Michael A. Schulze
 *	@email		m.schulze@uni-bremen.de
 *	@version	v1.0, compiled with arm-none-eabi-gcc v4.9.3 20150303
 *
 *  @defgroup   serial_communication
 *  @{
 *  @brief      Receives, processes and generates all serial USART data
 *
 *              The USART receive interrupt handler, and functions to
 *              respond to any serial commands are implemented here
 ********************************************************************************/
#ifndef SERIAL_COMMUNICATION_H
#define SERIAL_COMMUNICATION_H

/* C++ detection */
#ifdef __cplusplus
extern C {
#endif

#include "main.h"
#include "file_handler.h"
#include "led.h"
#include "tm_stm32f4_usart.h"
#include "tm_stm32f4_usart_dma.h"
#include "tm_stm32f4_gpio.h"
#include "tm_stm32f4_rtc.h"
#include "stdint.h"

/**
 * @brief  The USART1 receive interrupt handler. Buffers any received data until
 * 		   a carriage return or line feed in received, after which the buffered data
 * 		   is processed. If more than the max. allows chars are received without a
 *         CR or LF, the buffer is emptied.
 * @param  c The received character
 * @retval None
 */
void TM_USART1_ReceiveHandler(uint8_t c);

/**
 * @brief  Calling this function will compare the received data against a list of known
 * 		   commands. If a match is found, the command is processed. If no match is found
 * 		   "command error" is sent to the serial port and the buffer is emptied.
 * @param  None
 * @retval None
 */
static void serial_command_handler();

/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif /* SERIAL_COMMUNICATION_H */
