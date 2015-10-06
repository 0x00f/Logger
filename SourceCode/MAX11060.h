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
 *  @file       max11060.h
 *	@author 	Michael A. Schulze
 *	@email		m.schulze@uni-bremen.de
 *	@version	v1.0, compiled with arm-none-eabi-gcc v4.9.3 20150303
 *
 *  @defgroup   max11060
 *  @{
 *  @brief      Driver module for the max11060 analog to digital converters
 *
 *              Includes functions for the initialization, configuration, and
 *              data transfers to and from the MAX11060 ADCs
 ********************************************************************************/


#ifndef MAX11060_H
#define MAX11060_H

/* C++ detection */
#ifdef __cplusplus
extern C {
#endif


#include "stm32f4xx.h"
#include "tm_stm32f4_gpio.h"
#include "tm_stm32f4_spi.h"
#include "stm32f4xx_dma.h"
#include "defines.h"
#include "tm_stm32f4_spi_dma.h"
#include "stm32f4xx_spi.h"
#include "stdlib.h"


/*
 * Serial Peripheral Interface pin defines for the PCB. Must be adjusted
 * if different hardware is used. ifndef as this may also be defined in
 * the source file defines.h
 */
#ifndef MAX11060_SPI
#define MAX11060_SPI         				SPI2
#define MAX11060_SPI_PINSPACK		        TM_SPI_PinsPack_1
#define MAX11060_CS_PORT					GPIOB
#define MAX11060_CS_PIN						GPIO_Pin_12
#define MAX11060_DRDYOUT_PORT				GPIOE
#define MAX11060_DRDYOUT_PIN				GPIO_Pin_15
#endif

/* This enables the MASTER ADC's oscillator that is driven by the external crystal */
#define MAX11060_MASTER_CONFIG_BYTE         (uint8_t)0x10

/**
 * These are the numbers which must be written into the ADC data rate control
 * registers to generate the corresponding sample rates
 */
typedef enum {
	MAX11060_ADC_SPS_250 = ((uint16_t)0x25FF),
	MAX11060_ADC_SPS_500 = ((uint16_t)0x2000),
	MAX11060_ADC_SPS_1K = ((uint16_t)0x4000),
	MAX11060_ADC_SPS_2K = ((uint16_t)0x6000),
	MAX11060_ADC_SPS_4K = ((uint16_t)0x8000),
	MAX11060_ADC_SPS_5K = ((uint16_t)0xA39A),
	MAX11060_ADC_SPS_8K = ((uint16_t)0xA000),
	MAX11060_ADC_SPS_10K = ((uint16_t)0x0399),
	MAX11060_ADC_SPS_12K = ((uint16_t)0x01FF),
	MAX11060_ADC_SPS_16K = ((uint16_t)0x0000),
	MAX11060_ADC_SPS_20K = ((uint16_t)0xC1CD),
	MAX11060_ADC_SPS_32K = ((uint16_t)0xC000),
	MAX11060_ADC_SPS_64K = ((uint16_t)0xE000)
}MAX11060_ADC_SampleRate_t;


/*
 * MAX11060 internal registers
 * The device includes four registers accessible by 7 command bytes
 * The command bytes provide read and write access to the Data Rate Control
 * register, the Sampling Instant Control register, the Configuration register,
 * and read access to the Data register.
 */

/*
 * Command Bytes - provide r/w access to internal registers
 * All serial interface commands begin with a command byte
 * SICR - Sampling Instant Control Register (32*n bits)
 * DRCR - Data Rate Control Register (16 bits)
 * CR   - Configuration Register (8*n bits)
 * DR   - Data Register (96*n bits)
 */
#define MAX11060_WRITE_SICR_CMD       0x40
#define MAX11060_READ_SICR_CMD        0xC0
#define MAX11060_WRITE_DRCR_CMD       0x50
#define MAX11060_READ_DRCR_CMD        0xD0
#define MAX11060_WRITE_CR_CMD         0x60
#define MAX11060_READ_CR_CMD          0xE0
#define MAX11060_READ_DR_CMD          0xF0


/*
 * Sampling Instant Control Register (32*n bits)
 * By default, the devices sample all 4 input channel simultaneously.
 * To delay the sampling instant on one or more channels, program the appropri-
 * ate byte in the Sampling Instant Control Register.
 * The delay of the actual sampling instant of each individ. channel from the
 * default sampling instant (PHI_[7:0]=0x00) is adjustable between 32 to
 * 819,121 XIN clock cycles ( 1,3µs to 333µs with Fxinclk at 24,576MHz).
 */

/*
 * Configuration Register (8 Bits)
 * The config. register contains 5 bits that control the functionality of the
 * device. Default state is 0x00.
 * Bit 7 - SHDN - Set high to shutdown, low for regular operation
 * Bit 6 - RST - Set high to reset all registers to default, and realign
 * 				 sampling clocks and output data
 * Bit 5 - EN24BIT - Set to 0 for MAX11060
 * Bit 4 - XTALEN - Int osc. enable bit. To use an external clock, set to 0
 * Bit 3 - FAULTDIS - Overvoltage fault protection disable bit. Set high to
 * 					  disable overvoltage fault protection.
 * Bit 2 - PDBUF - Set high to disable internal reference buffer (when an
 *  			   external ref is used. Set to 0 to enable internal ref.
 * Bit [1:0] - Reserved. MUST BE SET TO 0.
 */
#define SHDN       (0x01 >> 7) // Set high to shutdown
#define ACTIVE     (0x00 >> 7)
#define RST        (0x01 >> 1)


/**
 * @brief   Initializes the ADC(s)
 * @param   The number of MAX11060 in the system
 * @retval  None
 */
void MAX11060_ADC_Init(uint8_t num_adcs);


/**
 * @brief   Sets the ADC sample rate of the ADCs
 * @param	samplerate enum corresponding to the required sample rate
 * @retval  None
 */
void MAX11060_Set_SampleRate(MAX11060_ADC_SampleRate_t samplerate);


/**
 * @brief  Returns the currently configured ADC sample rate
 * @param  None
 * @retval samplerate as a uint16_t configuration word.
 */
uint16_t MAX11060_Read_SampleRate();


void MAX11060_ADC_Reset(uint8_t adc_id);

/**
 * @brief  Returns the current conversion data of the ADCs
 * @param  *dataIn The destination array where the data will be written
 * @param  dummy Default dummy data that will be written out over the SPI while
 * 		   reading in the data (SPI is full duplex and can send and receive
 * 		   simultanously.
 * @param  count The number of 8 bit reads required to read all the data.
 * 			Each ADC has 96bits of information, so to read the data from 2
 * 			ADCs will require 96bits * 2 = 192 / 8 = 24 reads
 */
void MAX11060_ADC_ReadAll(uint8_t *dataIn, uint8_t dummy, uint16_t count);

/**
 * @brief  Returns current ADC configuration word
 * @param  Number of ADCs
 * @retval The current configuration
 */
uint8_t MAX11060_ADC_ReadConfig(uint8_t nums_adcs);


uint16_t MAX11060_ADC_ReadChx(uint8_t channel);





/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif // MAX11060_H
