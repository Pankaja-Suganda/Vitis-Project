
#ifndef NET_ENGINE_TYPE_H
#define NET_ENGINE_TYPE_H


/****************** Include Files ********************/
#include "xil_types.h"
#include "xstatus.h"

typedef struct Net_Engine_{
	volatile u32 Status_1;
    volatile u32 Status_2;
    volatile u32 Status_3;
    volatile u32 Status_4;
    volatile u32 Status_5;
    volatile u32 Status_6;
    volatile u32 Config_1;
    volatile u32 Config_2;
    volatile u32 Config_3;
    volatile u32 Config_4;
    volatile u32 Bias;
    volatile u32 Kernal_1;
    volatile u32 Kernal_2;
    volatile u32 Kernal_3;
    volatile u32 Kernal_4;
    volatile u32 Kernal_5;
    volatile u32 Kernal_6;
    volatile u32 Kernal_7;
    volatile u32 Kernal_8;
    volatile u32 Kernal_9;
} Net_Engine;

typedef enum {
    NET_ENGINE_OK           = 0,
    NET_ENGINE_FAIL         = -1,
    NET_ENGINE_LAST_STATUS  = -10,
} NET_STATUS;

typedef struct Net_Image_ {
    NET_STATUS status;
    u32 width;
    u32 height;
    u32 *buffer_ptr;
} Net_Image;

typedef enum {
    NET_CONFIG_CNN,
    NET_CONFIG_MAXPOOLING
} NET_CONFIG;

typedef u32 Net_Engine_ID;

typedef struct Net_Engine_Config_{
    UINTPTR    RegBase; 
    NET_CONFIG config;  
} Net_Engine_Config;

typedef enum{
    NET_ENGINE_ROW_COMPLETE_INTR,
    NET_ENGINE_RECEIVE_INTR
} Net_Engine_Intr;

typedef void (*Net_Engine_ISR_t)(void *CallBackRef);

typedef struct Net_Engine_Inst_{
    Net_Engine       *net_engine_regs;
    Net_Engine_Config config;
    Net_Engine_ISR_t  row_completed_isr;
    Net_Engine_ISR_t  receive_completed_isr;
} Net_Engine_Inst;
/**************************** Type Definitions *****************************/

#endif // NET_ENGINE_TYPE_H
