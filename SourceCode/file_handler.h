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
 *  @file       file_handler.h
 *	@author 	Michael A. Schulze
 *	@email		m.schulze@uni-bremen.de
 *	@version	v1.0, compiled with arm-none-eabi-gcc v4.9.3 20150303
 *
 *  @defgroup   file_handler
 *  @{
 *  @brief      Generates filename strings for each new measurement file
 *
 *              Uses the system module to get user filename data from NVSRAM
 ********************************************************************************/

#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

/* C++ detection */
#ifdef __cplusplus
extern C {
#endif

#include "main.h"

/**
 *  Give external modules access to the volume name
 */
extern char volume[15];

/**
 * @brief  Checks if a file system can be mounted on external flash
 * @param  None
 * @retval TRUE if mount is successful
 * @retval FALSE if not
 */
bool Mount_File_System();

/**
 * @brief  Returns the name of the file system (label) sd card
 * @param  None
 * @retval Pointer to null terminated string containing the name
 */
char* Get_FileSystem_Label();


/**
 * @brief  Generates a new filename for a new measurement. It either retrieves
 *         the filename from backup sram, or if sram is not initialized, uses
 *         the default "measurement". The filename has the format:
 *         "filename"_n, where n is an integer that is incremented for each new
 *         measurement
 * @param  None
 * @retval Pointer to the new char array containing the filename
 */
char *Gen_Filename();


/**
 * @brief  Generates a list of filenames of all found files
 * @param  None
 * @retval Pointer to the new char array
 */
char *Get_All_Filenames();


/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif /* FILE_HANDLER_H */
