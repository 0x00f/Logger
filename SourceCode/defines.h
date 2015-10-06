#ifndef TM_DEFINES_H
#define TM_DEFINES_H

//Put your global defines for all libraries here

/***
 * Defines for the SDIO-FatFS
 */

/* To use SPI communication with SDCard set to 0
 * To use SDIO communication with SDCard set to 1
 * */
#define    FATFS_USE_SDIO        1
/* Select your SPI settings */
//#define FATFS_SPI                SPI2
//#define FATFS_SPI_PINSPACK       TM_SPI_PinsPack_2

/* Custom CS pin for SPI communication */
//#define FATFS_CS_PORT        	 GPIOB
//#define FATFS_CS_PIN        	 GPIO_Pin_12

/* Activate SDIO 1-bit mode */
//#define FATFS_SDIO_4BIT        0
/* Activate SDIO 4-bit mode */
#define FATFS_SDIO_4BIT        1

///* Use detect pin */
//#define FATFS_USE_DETECT_PIN          0
///* Use writeprotect pin */
//#define FATFS_USE_WRITEPROTECT_PIN    0
//
///* If you want to overwrite default CD pin, then change this settings */
//#define FATFS_USE_DETECT_PIN_PORT     GPIOB
//#define FATFS_USE_DETECT_PIN_PIN      GPIO_PIN_6
//
///* If you want to overwrite default WP pin, then change this settings */
//#define FATFS_USE_WRITEPROTECT_PIN_PORT   GPIOB
//#define FATFS_USE_WRITEPROTECT_PIN_PIN    GPIO_PIN_7


/* SPI Prescaler Settings:
 * Prescaler_2 = 100MHz clock
 * Prescaler_4 = 10MHz clock
 * Prescaler_8 = 5MHz clock
 */
#define TM_SPI2_PRESCALER	SPI_BaudRatePrescaler_4
//Specify datasize
#define TM_SPI2_DATASIZE 	SPI_DataSize_8b
//Specify which bit is first
#define TM_SPI2_FIRSTBIT 	SPI_FirstBit_MSB
//Mode, master or slave
#define TM_SPI2_MASTERSLAVE SPI_Mode_Master
//Specify mode of operation, clock polarity and clock phase
//Modes 0 to 3 are possible
#define TM_SPI2_MODE		TM_SPI_Mode_2


#define TM_USART_BUFFER_SIZE 				512
#define TM_USART1_USE_CUSTOM_IRQ

/* Increase default RTC_SUB_SECOND resolution
 * 0x4E1F = 19999, 20000 steps in one second
 */
//#define RTC_SYNC_PREDIV 	0x4E20

//Use custom get_fattime() function
#define FATFS_CUSTOM_FATTIME			1

/* If ADC reads are interrupt driven leave this commented */
//#define MAX11060_Polling_MODE

#endif
