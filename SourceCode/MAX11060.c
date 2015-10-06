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

#include "MAX11060.h"

void MAX11060_ADC_ReadAll(uint8_t *dataIn, uint8_t dummy, uint16_t count);


void MAX11060_ADC_Init(uint8_t num_adcs) {
	/* Ensure the spi port pins are correctly initialized */
	TM_GPIO_Init(MAX11060_CS_PORT, MAX11060_CS_PIN , TM_GPIO_Mode_OUT, TM_GPIO_OType_PP, TM_GPIO_PuPd_NOPULL, TM_GPIO_Speed_High);
	TM_GPIO_SetPinHigh(MAX11060_CS_PORT, MAX11060_CS_PIN);
	TM_GPIO_Init(MAX11060_DRDYOUT_PORT, MAX11060_DRDYOUT_PIN, TM_GPIO_Mode_IN, TM_GPIO_OType_PP, TM_GPIO_PuPd_NOPULL, TM_GPIO_Speed_High);

    TM_GPIO_SetPinLow(MAX11060_CS_PORT, MAX11060_CS_PIN);
    TM_SPI_Send(SPI2, MAX11060_WRITE_CR_CMD);
    TM_SPI_Send(SPI2, MAX11060_MASTER_CONFIG_BYTE); //1st adc needs its oscillator enabled
    for(int i=1; i < num_adcs; i++) {
		TM_SPI_Send(SPI2, 0x00); // subsequent slave adcs use clkin and have other config word
    }
    TM_GPIO_SetPinHigh(MAX11060_CS_PORT, MAX11060_CS_PIN);
}

void MAX11060_Set_SampleRate(MAX11060_ADC_SampleRate_t sample_rate) {
	/* Begin communication - CS low */
    TM_GPIO_SetPinLow(MAX11060_CS_PORT, MAX11060_CS_PIN);
    /* Send the write data rate control register command */
    TM_SPI_Send(SPI2, MAX11060_WRITE_DRCR_CMD);
    /* Send upper 8 bits */
	TM_SPI_Send(SPI2, sample_rate>>8);
	/* Send lower 8 bits */
	TM_SPI_Send(SPI2, sample_rate);
	/* Bus arbitration, set CS high, comm. are done */
    TM_GPIO_SetPinHigh(MAX11060_CS_PORT, MAX11060_CS_PIN);
}

uint16_t MAX11060_Read_SampleRate() {
	uint16_t temp;

    TM_GPIO_SetPinLow(MAX11060_CS_PORT, MAX11060_CS_PIN);
    TM_SPI_Send(SPI2, MAX11060_READ_DRCR_CMD);
	TM_SPI_ReadMulti16(SPI2, &temp, 0x00, 1);
    TM_GPIO_SetPinHigh(MAX11060_CS_PORT, MAX11060_CS_PIN);

	return temp;
}

void MAX11060_ADC_ReadAll(uint8_t *dataIn, uint8_t dummy, uint16_t count) {

#ifdef MAX11060_Polling_MODE
	/* If this is == 1 ie PIN is high, ADC data is not ready to be read, so spinlock */
	while(TM_GPIO_GetInputPinValue(MAX11060_DRDYOUT_PORT, MAX11060_DRDYOUT_PIN));
#endif
    TM_GPIO_SetPinLow(MAX11060_CS_PORT, MAX11060_CS_PIN);
    TM_SPI_Send(SPI2, MAX11060_READ_DR_CMD);
    TM_SPI_ReadMulti(SPI2, dataIn,  dummy, count);
    //TM_SPI_DMA_Transmit(SPI2, NULL, dataIn, count);
    //while (TM_SPI_DMA_Working(SPI2));
    TM_GPIO_SetPinHigh(MAX11060_CS_PORT, MAX11060_CS_PIN);
}

uint8_t MAX11060_ADC_ReadConfig(uint8_t nums_adcs) {
	uint8_t temp;

    TM_GPIO_SetPinLow(MAX11060_CS_PORT, MAX11060_CS_PIN);
    TM_SPI_Send(SPI2, MAX11060_READ_CR_CMD);
    TM_SPI_ReadMulti(SPI2, &temp, 0x00, 1 );
    //TM_SPI_ReadMulti(SPI2, &temp, 0x00, 1 );
    TM_GPIO_SetPinHigh(MAX11060_CS_PORT, MAX11060_CS_PIN);

    return temp;
}
