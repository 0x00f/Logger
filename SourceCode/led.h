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
 *  @file       led.h
 *	@author 	Michael A. Schulze
 *	@email		m.schulze@uni-bremen.de
 *	@version	v1.0, compiled with arm-none-eabi-gcc v4.9.3 20150303
 *
 *  @defgroup   led
 *  @{
 *  @brief      Functions to control the RGB status LED
 *
 *              This module provides functions to set the RGB LED color, and
 *              read the currently displayed color.
 ********************************************************************************/


#ifndef LED_H
#define LED_H

/* C++ detection */
#ifdef __cplusplus
extern C {
#endif

#include "stm32f4xx.h"
#include "defines.h"
#include "tm_stm32f4_gpio.h"


typedef enum LED_COLOUR {
	BLACK = 0,  /* All off */
	RED,
	GREEN,
	BLUE,
	CYAN,       /* BLUE+GREEN */
	MAGENTA,    /* RED+BLUE */
	YELLOW,     /* RED+GREEN */
	WHITE	    /* RED+GREEN+BLUE */
} led_colour_t;


/*
 * The DEBUG_LED's refer to the 4 led's soldered next to the SD card holder
 * on the PCB. They are used primarily as debug indicators during development
 * and testing.
 * They are ordered ascending from left to right.
 *   LED1 | LED2 | LED3 | LED4
 *   PD15 | PD14 | PD13 | PD12
 */
#define DEBUG_LED_1                 GPIO_PIN_15
#define DEBUG_LED_2					GPIO_PIN_14
#define DEBUG_LED_3					GPIO_PIN_13
#define DEBUG_LED_4					GPIO_PIN_12
#define DEBUG_LED_PORT				GPIOD


/*
 * The STATUS_LED refers to the 5mm RGB led mounted to the front panel
 * It is used to display current system status to the user during operation.
 * Being RGB it is able to create many different colours.
 *                  __________
 *     ------------|  PD11    \
 *   --------------|  PD10     | RGB LED
 * ----------------|  GND      | orientation on PCB
 *     ------------|__PD9_____/
 */
#define STATUS_LED_RED				GPIO_PIN_9
#define STATUS_LED_BLUE				GPIO_PIN_10
#define STATUS_LED_GREEN			GPIO_PIN_11
#define STATUS_LED_ALL				STATUS_LED_RED | STATUS_LED_BLUE | STATUS_LED_GREEN
#define STATUS_LED_PINS				STATUS_LED_RED | STATUS_LED_BLUE | STATUS_LED_GREEN
#define STATUS_LED_PORT				GPIOD


/**
 * @brief  Initialize Status led input output pins
 * @param  None
 * @retval None
 */
void STATUS_LedInit(void);


/**
 * @brief  Sets status led color, indicating current system state
 * @param  colour: LED colour you want to set
 *            - BLACK: All leds are off
 *            - RED: Only Red led is lit
 *            - GREEN: Only Green led is lit
 *            - BLUE: Only Blue led is lit
 *            - CYAN: Blue and Green leds are lit
 *            - MAGENTA: Red and Blue leds are lit
 *            - YELLOW: Red and Green leds are lit
 *            - WHITE: Read, Green and Blue leds are lit
 * @retval None
 */
void SET_STATUS_LED(led_colour_t colour);


/**
 * @brief  Returns the currently displayed led color
 * @param  None
 * @retval led_colour_t
 */
led_colour_t GET_STATUS_LED(void);


/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif /* LED_H */
