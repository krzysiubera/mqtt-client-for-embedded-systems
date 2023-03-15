/*
 * svv_print.h
 *
 *  Created on: 15 Mar 2023
 *      Author: krzysiu
 */

#ifndef INC_SWV_PRINT_H_
#define INC_SWV_PRINT_H_

#include <stdio.h>

int _write(int file, char* ptr, int len)
{
	for (int i = 0; i < len; i++)
	{
		ITM_SendChar((*ptr++));
	}
	return len;
}

#endif /* INC_SWV_PRINT_H_ */
