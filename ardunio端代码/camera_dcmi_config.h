#ifndef __DCMI_H
#define __DCMI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stm32f4xx_hal.h>

void DCMI_Init(void);
void DCMI_DMA_Init(uint32_t memaddr,uint16_t memsize,uint32_t memblen,uint32_t meminc);
void DCMI_Start(void);
void DCMI_Stop(void);
void DCMI_Frame_Event();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
