
#ifndef NET_ENGINE_TYPE_H
#define NET_ENGINE_TYPE_H


/****************** Include Files ********************/
#include "xil_types.h"
#include "xaxidma.h"
#include "xscugic.h"
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
    volatile float Bias;
    volatile float Kernal_1;
    volatile float Kernal_2;
    volatile float Kernal_3;
    volatile float Kernal_4;
    volatile float Kernal_5;
    volatile float Kernal_6;
    volatile float Kernal_7;
    volatile float Kernal_8;
    volatile float Kernal_9;
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
    NET_CONFIG_MAXPOOLING,
    NET_CONFIG_NOT_SET
} NET_CONFIG;


typedef enum {
    NET_STATE_IDLE,
    NET_STATE_BUSY,
    NET_STATE_COMPLETED,
    NET_STATE_ERROR
} NET_STATE;

typedef u32 Net_Engine_ID;
typedef u32 Net_Engine_Intr_Id;
typedef u32 Net_Engine_Img;

typedef struct Net_Engine_Data_{
    NET_STATE state;
    u32 *input;
    u32 *output;
    u32 send_row_count;
    u32 received_row_count;
} Net_Engine_Data;

typedef struct Net_Engine_Config__{
    UINTPTR    RegBase; 
    NET_CONFIG config; 
    Net_Engine_Intr_Id row_complete_isr_id;
    Net_Engine_Intr_Id receive_isr_id;
} Net_Engine_Config;

typedef enum{
    NET_ENGINE_ROW_COMPLETE_INTR,
    NET_ENGINE_RECEIVE_INTR
} Net_Engine_Intr;

typedef struct Net_Engine_Inst_{
    Net_Engine_ID id;
    Net_Engine *net_engine_regs;
    Net_Engine_Config config;
	XAxiDma          dma_inst;
    XScuGic          intc_inst;
    Net_Engine_Data  cur_data;
} Net_Engine_Inst;

typedef enum{
    CONFIG_DATA_STATE_NOT_STARTED,
    CONFIG_DATA_STATE_BUSY,
    CONFIG_DATA_STATE_COMPLETED,
} CONFIG_DATA_STATE;


typedef struct CNN_Config_Data_{
    u8 index;
    struct {
        u32 Kernal_1;
        u32 Kernal_2;
        u32 Kernal_3;
        u32 Kernal_4;
        u32 Kernal_5;
        u32 Kernal_6;
        u32 Kernal_7;
        u32 Kernal_8;
        u32 Kernal_9;
    } Kernal;
    u32 Bias;
    u32 Reserved_1;
    u32 Reserved_2;
    CONFIG_DATA_STATE state;
}CNN_Config_Data;

typedef struct CNN_Data_Node_{
    struct CNN_Data_Node_ *next;
    CNN_Config_Data        config_data;
} CNN_Data_Node;

typedef struct CNN_1x1_Data_{
    u32 *kernal_data;
    u32 kernal_count;
    float bias;
}CNN_1x1_Data;

/**************************** Type Definitions *****************************/

#endif // NET_ENGINE_TYPE_H
