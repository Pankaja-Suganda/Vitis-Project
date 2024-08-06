#ifndef NET_ENGINE_CHANNEL_H
#define NET_ENGINE_CHANNEL_H


/****************** Include Files ********************/
#include "xil_types.h"
#include "net_engine.h"

/**************************** Type Definitions *****************************/

/************************** Function Prototypes ****************************/

typedef enum{
    CHANNEL_STATE_NOT_STARTED,
    CHANNEL_STATE_BUSY,
    CHANNEL_STATE_COMPLETED,
} CHANNEL_STATE;

typedef enum{
    CHANNEL_TYPE_INPUT,
    CHANNEL_TYPE_OUTPUT,
} CHANNEL_TYPE;

typedef enum{
    LAYER_ACTIVATION_RELU,
    LAYER_ACTIVATION_SOFTMAX,
    LAYER_ACTIVATION_SIGMOID,
    LAYER_ACTIVATION_NOT_REQUIRED,
} LAYER_ACTIVATION;


typedef struct Channel_Kernal_Data_{
    u32 index;
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
    LAYER_ACTIVATION activation;
    struct{
        Channel_Kernal_Data_Node *kernal_node;
    } cnn_data;
    struct{
        CNN_1x1_Data * data;
    } cnn_1x1_data;
    union{
        struct{
            u32 pool_size;   
            u32 stride;      
            u32 padding;    
        } mx_data;
        struct{
            float alpha;
            u32   pad[2];
        } relu_data;
        struct{
            u32   data;
            u32   pad[2];
        } softmax_data;
    } data;
} Channel;

typedef struct Channel_Node_{
    struct Channel_Node *next;
    Channel              data;
} Channel_Node;

Channel* CHANNEL_init(CHANNEL_TYPE type, u32 height, u32 width, u32 *input_ptr);

int CHANNEL_load_kernal(Channel *instance, Channel_Kernal_Data data, Channel *reference);

int CHANNEL_CNN_process(Channel *instance, Net_Engine_Inst* net_engine);

int CHANNEL_RELU_process(Channel *instance);

int CHANNEL_MAXPOOLING_process(Channel *instance);

int CHANNEL_update(Channel *instance, int height, int width);

#endif // NET_ENGINE_CHANNEL_H
