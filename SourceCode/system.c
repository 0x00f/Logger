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


#include "system.h"
#include "tm_stm32f4_usart.h"

static SYSTEM_STATE current_system_state;

SYSTEM_STATE Get_System_State() {
	return current_system_state;
}


/*
 * Initializes the internal (peripherals) and external hw of
 * the system
 */
SYSTEM_STATE Logger_System_Init() {

	current_system_state = STARTUP;

	/* Init the CPU, Flash and Clocks - 168MHz clock */
	SystemInit();
	/* Backup SRAM Init */
	TM_BKPSRAM_Init();
	/* Initialize delay */
	TM_DELAY_Init();
	// Initialize LEDs
	TM_DISCO_LedInit();
	STATUS_LedInit();
	/* Initialize ADC SPI2 */
    /* SCK = PB13, MOSI = PB15, MISO = PB14 */
	TM_SPI_Init(SPI2, TM_SPI_PinsPack_2);
    /* Init SPI DMA */
    TM_SPI_DMA_Init(SPI2);
    /* Initialize USART1, TX: PA9 */
    TM_USART_Init(USART1, TM_USART_PinsPack_1, 921600);
    /* Init TX DMA for USART1 */
    TM_USART_DMA_Init(USART1);
    /* Initialize ADC1 so we can measure VBAT*/
    TM_ADC_InitADC(ADC1);

    /* Initialize RTC with external 32768Hz clock
     * Only executed if the RTC is uninitialized ie. the supercap is dead
     * Hence the !
     */
    if (!TM_RTC_Init(TM_RTC_ClockSource_External)) {
        /* Subseconds are ignored when writing new time */

    	/* These are default values. Used should update them over usart to
    	 * the current date and time
    	 */
        datatime.hours = 1;
        datatime.minutes = 1;
        datatime.seconds = 0;
        datatime.year = 15;
        datatime.month = 1;
        datatime.date = 1;
        datatime.day = 1;

        /* Set default time */
        TM_RTC_SetDateTime(&datatime, TM_RTC_Format_BIN);
    }

    /* Init the ADCS */
    MAX11060_ADC_Init(2);  // We have 2 ADCs
    //MAX11060_Set_SampleRate(MAX11060_ADC_SPS_10K);
    MAX11060_Set_SampleRate(Get_Sample_Rate_Conf());

    // Internal initializations successful
    current_system_state = POST;

    return(current_system_state);
}

/*
 * Power On System Test
 * Used to ensure external HW is woring properly
 */
SYSTEM_STATE Logger_System_POST() {
	uint8_t temp = 0xFF;
	char string[10];

	/* All HW is initialized though Logger_System_Init() */
	if (current_system_state == POST) {
		TM_USART_Puts(USART1, "Running POST test...\n");
		/* Able to access the external flash memory? */
		if (Mount_File_System()) {
			TM_USART_Puts(USART1, "SD flash memory is mounted!\n");
			/* Able to read config data from external ADCs? */
			if( (MAX11060_ADC_ReadConfig(1) == MAX11060_MASTER_CONFIG_BYTE)) {
				TM_USART_Puts(USART1, "External ADCs are configured!\n");
				current_system_state = STANDBY;
			}
			else {
				current_system_state = HW_FAULT;
			}
		}
	}
	return(current_system_state);
}

/**
 * The Sample_Rate_t is needed by the the ADC driver, which requires a
 * specially calculated configuration word, not just a regular number.
 * Default rate if an unsupported rate is read from NVSRAM is 10ksps.
 */
MAX11060_ADC_SampleRate_t Get_Sample_Rate_Conf() {

	switch (TM_BKPSRAM_Read16(sample_rate_address)) {
		case(250):   return (MAX11060_ADC_SPS_250); break;
		case(500):   return (MAX11060_ADC_SPS_500); break;
		case(1000):  return (MAX11060_ADC_SPS_1K);  break;
		case(2000):  return (MAX11060_ADC_SPS_1K);  break;
		case(4000):  return (MAX11060_ADC_SPS_2K);  break;
		case(5000):  return (MAX11060_ADC_SPS_5K);  break;
		case(8000):  return (MAX11060_ADC_SPS_8K);  break;
		case(10000): return (MAX11060_ADC_SPS_10K); break;
		case(12000): return (MAX11060_ADC_SPS_12K); break;
		case(16000): return (MAX11060_ADC_SPS_16K); break;
		case(20000): return (MAX11060_ADC_SPS_20K); break;
		case(32000): return (MAX11060_ADC_SPS_32K); break;
		case(64000): return (MAX11060_ADC_SPS_64K); break;
		default:     return (MAX11060_ADC_SPS_10K); break;
	}
}


uint16_t Get_Sample_Rate() {
	return (TM_BKPSRAM_Read16(sample_rate_address));
}

void Set_Sample_Rate(uint16_t sample_rate) {
	TM_BKPSRAM_Write16(sample_rate_address, sample_rate);
}


/*
 * Returns the current measurement name from SRAM
 */
uint8_t Get_Measurement_Name(char *dest_buffer) {
	/* Plus 1 to allow for EOL character */
	char temp_buf[max_filename_len+1];
	char temp_char = '0';
	uint8_t i =  0;

	temp_char = TM_BKPSRAM_Read8(measurement_name_address);
	/* The isalnum() checks to make sure we are reading valid data. ie a-z, A-Z, and 0-9.
	 * This is important for uninitialized systems where the backup ram does not contain
	 * valid information as yet. In this case default filenames are used
	 */
	if(!isalnum(temp_char)) {
		/* Non-initialized BKPSRAM */
		return 0;
	}

	else {
		/* Loop until we reach the EOF char or the filename is too long and concatenate it */
		while((temp_char != '\0') & (i < max_filename_len)) {
			temp_char = TM_BKPSRAM_Read8(measurement_name_address+i);
			dest_buffer[i] = temp_char;
			i++;
		}
		/* If i is 0, we haven't read any valid data, no need to set final \0 */
		if(i!=0) {
			dest_buffer[i] = '\0';
		}
		return i;
	}
}

/*
 * Updates the measurement name in NVSRAM
 */
uint8_t Set_Measurement_Name(const char *name) {
	uint8_t i = 0;

	/* clear the original data */
	for(int j = 0; j<max_filename_len; j++) {
		TM_BKPSRAM_Write8(measurement_name_address+j, '\0');
	}

	/* write the new data */
	while((name[i]) != '\0') {
		TM_BKPSRAM_Write8(measurement_name_address+i, name[i]);
		i++;
	}
	TM_BKPSRAM_Write8(measurement_name_address+i, '\0');

	return i;
}
