/*
 * 8888888888P        d8888 8888888b.  888b     d888 | Zentrum für
 *       d88P        d88888 888   Y88b 8888b   d8888 | Angewandte
 *      d88P        d88P888 888    888 88888b.d88888 | Raumfahrtechnologie
 *     d88P        d88P 888 888   d88P 888Y88888P888 | und Mikrogravitation
 *    d88P        d88P  888 8888888P"  888 Y888P 888 |--------------------------
 *   d88P        d88P   888 888 T88b   888  Y8P  888 | Center of Applied Space
 *  d88P        d8888888888 888  T88b  888   "   888 | Technology and
 * d8888888888 d88P     888 888   T88b 888       888 | Microgravity
 */

#include "led.h"


static volatile led_colour_t current_colour = 0;

void STATUS_LedInit(void) {
	/* Set pins as output */
	//TM_GPIO_Init(STATUS_LED_PORT, STATUS_LED_PINS, TM_GPIO_Mode_OUT, TM_GPIO_OType_PP, TM_GPIO_PuPd_NOPULL, TM_GPIO_Speed_Low);
	TM_GPIO_Init(STATUS_LED_PORT, STATUS_LED_RED, TM_GPIO_Mode_OUT, TM_GPIO_OType_PP, TM_GPIO_PuPd_NOPULL, TM_GPIO_Speed_High);
	TM_GPIO_Init(STATUS_LED_PORT, STATUS_LED_GREEN, TM_GPIO_Mode_OUT, TM_GPIO_OType_PP, TM_GPIO_PuPd_NOPULL, TM_GPIO_Speed_High);
	TM_GPIO_Init(STATUS_LED_PORT, STATUS_LED_BLUE, TM_GPIO_Mode_OUT, TM_GPIO_OType_PP, TM_GPIO_PuPd_NOPULL, TM_GPIO_Speed_High);
	/* Turn leds off */
	TM_GPIO_SetPinLow(STATUS_LED_PORT, STATUS_LED_PINS);
}

void SET_STATUS_LED(led_colour_t colour) {
	current_colour = colour;
	switch (colour) {
		case BLACK:
			TM_GPIO_SetPinLow(STATUS_LED_PORT, STATUS_LED_PINS);
		case RED:
			TM_GPIO_SetPinLow(STATUS_LED_PORT, STATUS_LED_PINS);
			TM_GPIO_SetPinHigh(STATUS_LED_PORT, STATUS_LED_RED);
			break;
		case GREEN:
			TM_GPIO_SetPinLow(STATUS_LED_PORT, STATUS_LED_PINS);
			TM_GPIO_SetPinHigh(STATUS_LED_PORT, (STATUS_LED_GREEN));
		case BLUE:
			TM_GPIO_SetPinLow(STATUS_LED_PORT, STATUS_LED_PINS);
			TM_GPIO_SetPinHigh(STATUS_LED_PORT, STATUS_LED_BLUE);
		case CYAN:
			TM_GPIO_SetPinLow(STATUS_LED_PORT, STATUS_LED_PINS);
			TM_GPIO_SetPinHigh(STATUS_LED_PORT, STATUS_LED_BLUE | STATUS_LED_GREEN);
		case MAGENTA:
			TM_GPIO_SetPinLow(STATUS_LED_PORT, STATUS_LED_PINS);
			TM_GPIO_SetPinHigh(STATUS_LED_PORT, STATUS_LED_RED | STATUS_LED_BLUE);
		case YELLOW:
			TM_GPIO_SetPinLow(STATUS_LED_PORT, STATUS_LED_PINS);
			TM_GPIO_SetPinHigh(STATUS_LED_PORT, STATUS_LED_RED | STATUS_LED_GREEN);
		case WHITE:
			TM_GPIO_SetPinLow(STATUS_LED_PORT, STATUS_LED_PINS);
			TM_GPIO_SetPinHigh(STATUS_LED_PORT, STATUS_LED_RED | STATUS_LED_GREEN | STATUS_LED_BLUE);
		default:
			TM_GPIO_SetPinLow(STATUS_LED_PORT, STATUS_LED_PINS);
			break;
	}
}

led_colour_t GET_STATUS_LED(void) {
	return(current_colour);
}
