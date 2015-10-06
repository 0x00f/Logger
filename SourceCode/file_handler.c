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

#include "file_handler.h"
#include "system.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/* Char array which will store generated filenames */
static char filename[max_filename_len];

/* Char array which will store generated measurement footer */
static char footer[30];

/* Keeps track of the number of measurements (and resulting files) since startup
 * Is incremented before each new measurement
 */
static uint8_t measurement_counter;

/* Flag indicating mount status of the SD card file system
 * true = successfully mounted
 * false = not mounted / file system error
 */
static bool mounted_status = false;

///* Free and total space on SD card */
uint32_t total, free_space;

///* Buffer to store SD card volume name */
char volume[15] = {0};

/*
 * This function generates incremented filenames (name_1, name_2, ...) for each new measurement.
 * The names are generated from user configurable data in NVSRAM. If no valid data is found, the
 * default name of "measurement_x" will be used
 */
char *Gen_Filename() {
	uint8_t name_buffer[21]= {0};
	uint8_t name_pointer = 0;
	const uint8_t default_string[] = "measurement";

	name_pointer = Get_Measurement_Name((char *)name_buffer);

	/* We have received valid data - user defined filename */
	if (name_pointer > 0) {
		sprintf(filename, "%s_%d.csv", name_buffer, measurement_counter);
	}
	/* BKPSRAM was uninitialized, use default filename */
	else {
		sprintf(filename, "%s_%d.csv", default_string, measurement_counter);
	}

	measurement_counter++;

	return(filename);
}


/*
 * Resets measurement counter to 0, in the case that a set of measurements needs
 * to be overwritten
 */
void File_Handler_Init() {
	measurement_counter = 0;
}

/*
 * Generates a string containing the sample rate of the data that can be appended
 * to the end of a file.
 */
char *Gen_File_Footer() {
	sprintf(footer, "\nSampled at %dHz\n", Get_Sample_Rate());
	return(footer);
}

/*
 * returns the name of the sd card file system
 */
char* Get_FileSystem_Label() {
	char *str_pntr;

	f_getlabel("", volume, 0);
	str_pntr = &volume;

	return (str_pntr);
}

/*
 * mount test for external flash. used by the POST test to ensure
 * the flash is working.
 */
bool Mount_File_System() {
    /* Mount Drive */
	if(f_mount(&FatFs, "", 1)==FR_OK) {
		mounted_status = true;
	}
	else {
		mounted_status = false;
	}

	return(mounted_status);
}


bool Is_File_System_Mounted() {
	 return(mounted_status);
}
