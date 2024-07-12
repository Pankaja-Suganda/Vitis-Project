
#ifndef NET_ENGINE_LAYER_H
#define NET_ENGINE_LAYER_H


/****************** Include Files ********************/
#include "net_engine.h"

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

typedef struct Layer_{
    u8 index;
    u32 *input;
    u32 *output;
    u32 *temp;
    Net_Engine_Inst net_engine;
    LAYER_STATE state;
} Layer;

typedef struct CNN_Layer_{
    Layer layer;
    CNN_Data_Node *data;
    u32 completed_count;
    u32 total_count;
} CNN_Layer;

typedef struct Max_Pooling_Layer_{
    Layer layer;
    Max_Pooling_Config_Data data;    
    u32 completed_count;
    u32 total_count
} Max_Pooling_Layer;

int LAYER_CNN_init(CNN_Layer *instance, u32* input, u32* output);

int LAYER_Max_pooling_init(Max_Pooling_Layer *instance, u32* input, u32* output);

int LAYER_process(Net_Engine_Inst *instance, u32* input, u32* output);

int LAYER_CNN_load_data(CNN_Layer *instance, CNN_Config_Data data);

int LAYER_CNN_process(CNN_Layer *instance);

#endif // NET_ENGINE_LAYER_H
