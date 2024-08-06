
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
    LAYER_STATE_BUSY,
    LAYER_STATE_COMPLETED,
} LAYER_STATE;

typedef enum{
    LAYER_TYPE_CNN_1X1,
    LAYER_TYPE_CNN_2X2,
    LAYER_TYPE_CNN_3X3,
    LAYER_TYPE_MAXPOOLING,
} LAYER_TYPE;


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
    u8          index;
    LAYER_STATE state;
    LAYER_TYPE  type;
    LAYER_ACTIVATION activation;
    struct {
        u32 *input;
        u32 *output;
        u32 *temp;
    } ptr;
    struct {
        Layer_Data_Post_Process *post_process;
        Layer_Data_Pre_Process  *pre_process;    
    } func;
    struct{
        Channel_Node *channels;
        u8           count;
    }input_channels;
    struct{
        Channel_Node *channels;
        u8           count;
    }output_channels;
    struct{
        u32* memory_ptr;
        u32* memory_tail;
        u32  used_mem_size;
        u32  availale_mem_size;
    } memory;
    union{
        struct{
            CNN_Data_Node * data;
        } cnn_data;
        struct{
            Max_Pooling_Config_Data *data; 
        } mx_data;
        struct{
            float data;
        } relu_data;
        struct{
            u32 data;
        } softmax_data;
    } data;
} Layer;

typedef void *(Layer_init_cb)(Layer *layer, Layer prev_layer);

// typedef struct CNN_Layer_{
//     Layer layer;
//     CNN_Data_Node *data;
// } CNN_Layer;

// typedef struct MXPOOLING_Layer_{
//     Layer layer;
//     Max_Pooling_Config_Data data; 
// } MXPOOLING_Layer;

// typedef struct RELU_Layer_{
//     Layer layer;
// } RELU_Layer;

// typedef struct SOFTMAX_Layer_{
//     Layer layer;
// } SOFTMAX_Layer;

Layer* LAYER_init(LAYER_TYPE type, LAYER_ACTIVATION activation, u32* memory_ptr, u32 memory_len);

int LAYER_process(Layer *instance, void *optional);

// int LAYER_CNN_load_data(CNN_Layer *instance, CNN_Config_Data data);

// void LAYER_CNN_set_callbacks(CNN_Layer *instance, Layer_Data_Post_Process *post_process, Layer_Data_Pre_Process  *pre_process);

// int LAYER_CNN_process(CNN_Layer *instance);

// int LAYER_add_channel(CNN_Layer *instance, Channel channel);

int LAYER_add_input_channel(Layer *instance, u32 height, u32 width, u32 *input_ptr);

int LAYER_add_cnn_output_channels(Layer **instance, void* ptr, void* ptr_activation, int channel_count, u32 height, u32 width);

int LAYER_add_cnn_1x1_output_channels(Layer **instance, void *weights, void *bias, int weights_count, int channel_count, u32 height, u32 width);

int LAYER_add_maxpool_output_channels(Layer **instance, u32 pool_size, u32 stride, u32 padding, u32 channel_count, u32 height, u32 width);

int LAYER_link(Layer *input_layer, Layer *output_layer);

int LAYER_update(Layer *input_layer, Layer *prev_layer, int height, int width);

#endif // NET_ENGINE_LAYER_H
