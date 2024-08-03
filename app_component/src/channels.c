#include "channels.h"
#include <math.h>
#include <stdio.h>

#ifndef DDR_BASE_ADDR
#warning CHECK FOR THE VALID DDR ADDRESS IN XPARAMETERS.H, \
DEFAULT SET TO 0x01000000
#define MEM_BASE_ADDR		0x01000000
#else
#define MEM_BASE_ADDR		(DDR_BASE_ADDR + 0x1000000)
#endif

#define DEFAULT_ROW_LEN     100
#define DEFAULT_BUFFER_LEN  (DEFAULT_ROW_LEN * DEFAULT_ROW_LEN * 4)

#define TX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00100000)
#define RX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00300000)
#define RX_BUFFER_HIGH		(MEM_BASE_ADDR + 0x004FFFFF)


// add channel size
Channel* CHANNEL_init(CHANNEL_TYPE type, u32 height, u32 width, u32 *input_ptr){
    Channel *instance = NULL;
    instance = (Channel *)malloc(sizeof(Channel));
    if (instance == NULL) {
        printf("Channel malloc failed\n");
        return NULL;
    }

    instance->type              = type;
    instance->height            = height;
    instance->width             = width;
    instance->total_bytes       = height*width;
    instance->state             = CHANNEL_STATE_NOT_STARTED;
    instance->kernal_data_count = 0;

    instance->cnn_data.kernal_node       = NULL;


    if(type == CHANNEL_TYPE_INPUT){
        instance->input_ptr  = input_ptr;
        instance->output_ptr = NULL;
        instance->temp_ptr   = NULL;
    }
    else{
        instance->input_ptr  = NULL;
        instance->output_ptr = NULL;
        instance->temp_ptr   = NULL;
    }
    return instance;
}

static Channel_Kernal_Data_Node* create_kernal_node(Channel_Kernal_Data data){
    Channel_Kernal_Data_Node* new = (Channel_Kernal_Data_Node*)malloc(sizeof(Channel_Kernal_Data_Node));
    if(new == NULL){
        xil_printf("Node malloc error \r\n");
        return NULL;
    }
    new->data = data;
    new->next = NULL;

    return new;
}

static void append_kernal_node(Channel_Kernal_Data_Node** head_ref, Channel_Kernal_Data new_data) {
    Channel_Kernal_Data_Node* new = create_kernal_node(new_data);
    if (new == NULL) {
        return;
    }

    if (*head_ref == NULL) {
        *head_ref = new;
        return;
    }

    Channel_Kernal_Data_Node* last = *head_ref;
    while (last->next != NULL) {
        last = last->next;
    }

    last->next = new;
}

int CHANNEL_load_kernal(Channel *instance, Channel_Kernal_Data data, Channel *reference){
    if(instance->type == CHANNEL_TYPE_OUTPUT){
        data.reference = (Channel*)reference;
        data.state     = CHANNEL_STATE_NOT_STARTED;
        data.index     = instance->kernal_data_count;

        Channel *test = (Channel*)data.reference;

        if(instance->cnn_data.kernal_node == NULL){
            instance->cnn_data.kernal_node = create_kernal_node(data);
        }
        else{
            append_kernal_node(&(instance->cnn_data.kernal_node), data);
        }
        instance->kernal_data_count++;
    }
}

static void CHANNEL_post_process(Channel *instance){
    float *temp_ptr = (float*)instance->temp_ptr;
    float *out_ptr  = (float*)instance->output_ptr;

    for (int Index = 0; Index < instance->total_bytes; Index++) {
        out_ptr[Index] = out_ptr[Index] + temp_ptr[Index];
        // if(Index < 10)
        //     printf("temp %f, out %f \\r\n", temp_ptr[Index], out_ptr[Index]);
    }
}

static float CHANNEL_RELU_activation(float value, float alpha){
    return value > 0? value : value * alpha;
}


// static float CHANNEL_SOFTMAX_activation(float value, float max_value, float exp_sum) {
//     return exp((double) value - max_value) / exp_sum;
// }

static void CHANNEL_activation(Channel *instance){
    float *out_ptr  = (float*)instance->output_ptr;
    float max_value = -1e10;
    float exp_sum   = 0.0f;

    if(instance->activation == LAYER_ACTIVATION_RELU){
        for (int Index = 0; Index < instance->total_bytes; Index++) {
            // if(Index < 10)
            //     printf(" pre  %d out %f * %f \r\n", Index, out_ptr[Index], instance->data.relu_data.alpha);

            out_ptr[Index] = CHANNEL_RELU_activation(out_ptr[Index], instance->data.relu_data.alpha);
            // if(Index < 10)
            //     printf(" post %d out %f \r\n", Index, out_ptr[Index]);
        }
    }   
    else if(instance->activation == LAYER_ACTIVATION_SOFTMAX){

        // for (int Index = 0; Index < instance->total_bytes; Index++) {
        //     if (out_ptr[Index] > max_value) {
        //         max_value = out_ptr[Index];
        //     }
        // }
        
        // for (int Index = 0; Index < instance->total_bytes; Index++) {
        //     exp_sum += exp((double)(out_ptr[Index] - max_value)); // Compute sum of exponentials
        // }

        // for (int Index = 0; Index < instance->total_bytes; Index++) {
        //     out_ptr[Index] = CHANNEL_SOFTMAX_activation(out_ptr[Index], max_value, exp_sum);
        // }    
    }

}

static void CHANNEL_kernal_to_net_config(Channel_Kernal_Data kernal_data, CNN_Config_Data* net_config_data){
    net_config_data->Kernal.Kernal_1 = kernal_data.Kernal.Kernal_1;
    net_config_data->Kernal.Kernal_2 = kernal_data.Kernal.Kernal_2;
    net_config_data->Kernal.Kernal_3 = kernal_data.Kernal.Kernal_3;
    net_config_data->Kernal.Kernal_4 = kernal_data.Kernal.Kernal_4;
    net_config_data->Kernal.Kernal_5 = kernal_data.Kernal.Kernal_5;
    net_config_data->Kernal.Kernal_6 = kernal_data.Kernal.Kernal_6;
    net_config_data->Kernal.Kernal_7 = kernal_data.Kernal.Kernal_7;
    net_config_data->Kernal.Kernal_8 = kernal_data.Kernal.Kernal_8;
    net_config_data->Kernal.Kernal_9 = kernal_data.Kernal.Kernal_9;
    net_config_data->Bias            = kernal_data.Bias;

    net_config_data->state = CONFIG_DATA_STATE_NOT_STARTED;
}

int CHANNEL_CNN_process(Channel *instance, Net_Engine_Inst* net_engine){
    // xil_printf("Channel %d Processing \r\n", instance->index);

    Channel_Kernal_Data_Node* cur_kernal = instance->cnn_data.kernal_node;
    Channel *channel = NULL;
    CNN_Config_Data net_config_data;

    // check whether the channel loaded
    if(cur_kernal == NULL){
        xil_printf("No output channel available \r\n");
        return 0;
    }

    for(int index = 0; index < instance->total_bytes; index++){
        instance->output_ptr[index] = 0.0;
    }

    NET_ENGINE_config_row_length(net_engine, (instance->height + 2));

    while (cur_kernal != NULL){
        channel = (Channel*)cur_kernal->data.reference;

        CHANNEL_kernal_to_net_config(cur_kernal->data, &net_config_data);

        // xil_printf("\tKernal %d Processing %d, row length %d \r\n", cur_kernal->data.index, channel->index, (instance->height + 2));;

        if(channel->input_ptr != NULL){
            NET_ENGINE_process_cnn(net_engine, (u32*)channel->input_ptr, (u32*)instance->temp_ptr, net_config_data, instance->height);
        }
        // calling post process function -> can add postprocess function here
        CHANNEL_post_process(instance);

        NET_ENGINE_reset(net_engine);

        // jumping to next channel
        cur_kernal = cur_kernal->next;
    }

    if(instance->activation != LAYER_ACTIVATION_NOT_REQUIRED){
        CHANNEL_activation(instance);
    }
}
