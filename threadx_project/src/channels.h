
#ifndef NET_ENGINE_CHANNEL_H
#define NET_ENGINE_CHANNEL_H


/****************** Include Files ********************/
#include "xil_types.h"
#include "net_engine.h"

/**************************** Type Definitions *****************************/

/************************** Function Prototypes ****************************/

typedef enum{
    CHANNEL_STATE_NOT_STARTED,
    CHANNEL_STATE_STATE_BUSY,
    CHANNEL_STATE_STATE_COMPLETED,
} CHANNEL_STATE;

typedef enum{
    CHANNEL_TYPE_INPUT,
    CHANNEL_TYPE_OUTPUT,
} CHANNEL_TYPE;


typedef struct Channel_Kernal_Data_{
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
    CHANNEL_STATE state;
    struct Channel  *reference;
}Channel_Kernal_Data;

typedef struct Channel_Kernal_Data_Node_{
    struct Channel_Kernal_Data_Node *next;
    Channel_Kernal_Data              data;
} Channel_Kernal_Data_Node;


typedef struct Channel_{
    u8  index;
    u8  kernal_data_count;
    u32 height;
    u32 width;
    u32 total_bytes;
    u32* input_ptr;
    u32* output_ptr;
    u32* temp_ptr;
    CHANNEL_TYPE  type;
    CHANNEL_STATE state;
    Channel_Kernal_Data_Node *kernal_node;
} Channel;

typedef struct Channel_Node_{
    struct Channel_Node *next;
    Channel              data;
} Channel_Node;

int CHANNEL_init(Channel *instance, CHANNEL_TYPE type, u32 height, u32 width, u32 *input_ptr);

int CHANNEL_load_kernal(Channel *instance, Channel_Kernal_Data data, Channel *reference);

void CHANNEL_process_channel(Channel *instance, Net_Engine_Inst* net_engine);

#endif // NET_ENGINE_CHANNEL_H
