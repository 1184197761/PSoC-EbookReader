/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#ifndef _BSP_H_
#define _BSP_H_

#include <cy_syslib.h>

#define NUM_ORANGE_LED    8
#define NUM_RED_LED       9

#define LED_ON            0
#define LED_OFF           1

void BSP_PreInit(void);
void BSP_PostInit(void);
void BSP_SetLEDStatus(uint8 whichno, uint8 onoff);
#endif /* _BSP_H_ */

/* [] END OF FILE */
