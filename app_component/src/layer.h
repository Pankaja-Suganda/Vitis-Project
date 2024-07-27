
#ifndef NET_ENGINE_LAYER_H
#define NET_ENGINE_LAYER_H


/****************** Include Files ********************/
#include "net_engine.h"
#include "channels.h"

/**************************** Type Definitions *****************************/
#define MAX_ROW_SIZE        100
#define MAX_IMAGE_SIZE      100

/************************** Function Prototypes ****************************/

typedef enum{
    LAYER_STATE_NOT_STARTED,
    LAYER_STATE_STATE_BUSY,
    LAYER_STATE_STATE_COMPLETED,
} LAYER_STATE;




typedef struct Max_Pooling_Config_Data_{
    u8 index;
    u32 Reserved_1;
    u32 Reserved_2;
    CONFIG_DATA_STATE state;
}Max_Pooling_Config_Data;

typedef struct Layer_ Layer;
typedef void *(Layer_Data_Post_Process)(Layer layer);
typedef void *(Layer_Data_Pre_Process)(Layer layer);

typedef struct Layer_{
    u8 index;
    u32 *input;
    u32 *output;
    u32 *temp;
    Layer_Data_Post_Process *post_process;
    Layer_Data_Pre_Process  *pre_process;
    Net_Engine_Inst net_engine;
    int filter_count;
    LAYER_STATE state;
} Layer;


typedef struct CNN_Layer_{
    Layer layer;
    CNN_Data_Node *data;
    u32 completed_count;
    u32 total_count;
    Channel_Node *input_channels;
    u8           input_channels_count;
    Channel_Node *output_channels;
    u8           output_channels_count;
    u32* memory_ptr;
    u32  used_mem_size;
    u32  availale_mem_size;
} CNN_Layer;

typedef struct Max_Pooling_Layer_{
    Layer layer;
    Max_Pooling_Config_Data data;    
    u32 completed_count;
    u32 total_count
} Max_Pooling_Layer;

int LAYER_CNN_init(CNN_Layer *instance, u32* memory_ptr, u32 memory_len);

int LAYER_Max_pooling_init(Max_Pooling_Layer *instance, u32* input, u32* output);

int LAYER_process(Net_Engine_Inst *instance, u32* input, u32* output);

int LAYER_CNN_load_data(CNN_Layer *instance, CNN_Config_Data data);

void LAYER_CNN_set_callbacks(CNN_Layer *instance, Layer_Data_Post_Process *post_process, Layer_Data_Pre_Process  *pre_process);

int LAYER_CNN_process(CNN_Layer *instance);

int LAYER_add_channel(CNN_Layer *instance, Channel channel);

#endif // NET_ENGINE_LAYER_H
