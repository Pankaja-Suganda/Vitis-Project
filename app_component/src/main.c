
#include <stdio.h>
#include <xil_printf.h>
#include <xil_types.h>
#include "neural_network.h"
#include "layer.h"
#include "test_sample.h"
#include "conv_layer.h"
#include "xscutimer.h"

#ifndef DDR_BASE_ADDR
#warning CHECK FOR THE VALID DDR ADDRESS IN XPARAMETERS.H, \
DEFAULT SET TO 0x01000000
#define MEM_BASE_ADDR		0x01000000
#else
#define MEM_BASE_ADDR		(DDR_BASE_ADDR + 0x1000000)
#endif

#define NN_RECEIVE_MEM_BASE (MEM_BASE_ADDR + 0x00400000)
#define NN_RECEIVE_MEM_LEN  (0xA000)
#define NN_RECEIVE_MEM_HIGH (NN_RECEIVE_MEM_BASE + NN_RECEIVE_MEM_LEN)

#define FIRST_CONV_LAYER_MEM_BASE (MEM_BASE_ADDR + 0x00500000)
#define FIRST_CONV_LAYER_MEM_LEN  (0x200000)
#define FIRST_CONV_LAYER_MEM_HIGH (FIRST_CONV_LAYER_MEM_BASE + FIRST_CONV_LAYER_MEM_LEN)


#define NN_MEM_POOL_1_BASE        (MEM_BASE_ADDR + 0x00500000)
#define NN_MEM_POOL_1_LEN         0x0005DCC8
#define NN_MEM_POOL_1_HIGH        (NN_MEM_POOL_1_BASE + NN_MEM_POOL_1_LEN)

#define NN_MEM_POOL_2_BASE        (NN_MEM_POOL_1_HIGH)
#define NN_MEM_POOL_2_LEN         NN_MEM_POOL_1_LEN
#define NN_MEM_POOL_2_HIGH        (NN_MEM_POOL_2_BASE + NN_MEM_POOL_2_LEN)

#define NN_MEM_POOL_3_BASE        (NN_MEM_POOL_2_HIGH)
#define NN_MEM_POOL_3_LEN         NN_MEM_POOL_1_LEN
#define NN_MEM_POOL_3_HIGH        (NN_MEM_POOL_3_BASE + NN_MEM_POOL_3_LEN)

#define INPUT_SIZE  100
#define OUTPUT_SIZE 98
#define CNN_INPUT_SIZE_2  49
#define CNN_OUTPUT_SIZE_2 47
#define CNN_INPUT_SIZE_3  47
#define CNN_OUTPUT_SIZE_3 45
#define CNN_OUTPUT_SIZE_4 CNN_OUTPUT_SIZE_3
#define CNN_OUTPUT_SIZE_5 CNN_OUTPUT_SIZE_3

#define MAX_POOLING_POOL_SIZE_1  2
#define MAX_POOLING_STRIDE_1     2
#define MAX_POOLING_PADDING_1    0
#define MAX_POOLING_OUT_SIZE     49
#define MAX_POOLING_OUT_CHANNELS 10

#define UNUSED(x) (void)(x)


Layer *layer_list[10] = {NULL};

void Test_NN_Model(NeuralNetwork *model){
    xil_printf("NN Model\r\n");
    xil_printf("Status          %d \r\n", model->status);
    xil_printf("Completed Count %d \r\n", model->completed_count);
    xil_printf("Layer Count     %d \r\n", model->layer_count);
    xil_printf("Receive Ptr     %p \r\n\n", model->receive_memory_ptr);

    NN_Layer_Node *layer = NULL;
    Channel_Node  *channel = NULL;
    Channel_Kernal_Data_Node *kernals = NULL;

    layer = model->layers;

    while (layer != NULL) {
        xil_printf("Layer %d - T(%d), S(%d), MP(%p), MT(%p), MA(%d), MU(%d) \r\n", 
            layer->layer.index,
            layer->layer.type,
            layer->layer.state,
            layer->layer.memory.memory_ptr,
            layer->layer.memory.memory_tail,
            layer->layer.memory.availale_mem_size,
            layer->layer.memory.used_mem_size
            );
        // xil_printf("\tType %p \r\n", layer->layer.type);
        // xil_printf("\tState %p \r\n", layer->layer.state);
        // xil_printf("\tInput Channel Count  %d \r\n", layer->layer.input_channels.count);
        // xil_printf("\tOutput Channel Count %d \r\n", layer->layer.output_channels.count);
        // xil_printf("\tMemory Ptr  %p \r\n", layer->layer.memory.memory_ptr);
        // xil_printf("\tMemory Tail %p \r\n", layer->layer.memory.memory_tail);
        // xil_printf("\tMemory available %d \r\n", layer->layer.memory.availale_mem_size);
        // xil_printf("\tMemory Used      %d \r\n\n", layer->layer.memory.used_mem_size);
        xil_printf("Input Channels %d\n", layer->layer.input_channels.count);
        channel = layer->layer.input_channels.channels;

        while (channel != NULL) {
            xil_printf("\tIndex %d - S(%d), T(%d), KC(%d), H(%d), W(%d), Tb(%d), IP(%p), TP(%p), OP(%p) \r\n", 
                channel->data.index,
                channel->data.state,
                channel->data.type,
                channel->data.kernal_data_count,
                channel->data.height,
                channel->data.width,
                channel->data.total_bytes,
                channel->data.input_ptr,
                channel->data.temp_ptr,
                channel->data.output_ptr
                );
            // xil_printf("\t\tSate  %d\n", channel->data.state);
            // xil_printf("\t\tChannel Count %d\n", channel->data.kernal_data_count);
            // xil_printf("\t\tHeight        %d\n", channel->data.height);
            // xil_printf("\t\tWidth         %d\n", channel->data.width);
            // xil_printf("\t\tTotal bytes   %d\n", channel->data.total_bytes);
            // xil_printf("\t\tInput Ptr     %p\n", channel->data.input_ptr);
            // xil_printf("\t\tOutput Ptr    %p\n", channel->data.output_ptr);
            // xil_printf("\t\tTemp Ptr      %p\n", channel->data.temp_ptr);
            channel = channel->next;
        }

        xil_printf("Output Channels %d\n", layer->layer.output_channels.count);
        channel = layer->layer.output_channels.channels;

        while (channel != NULL) {
            xil_printf("\tIndex %d - S(%d), T(%d), KC(%d), H(%d), W(%d), Tb(%d), IP(%p), TP(%p), OP(%p) \r\n", 
                channel->data.index,
                channel->data.state,
                channel->data.type,
                channel->data.kernal_data_count,
                channel->data.height,
                channel->data.width,
                channel->data.total_bytes,
                channel->data.input_ptr,
                channel->data.temp_ptr,
                channel->data.output_ptr
                );

            kernals = channel->data.cnn_data.kernal_node;
            while (kernals != NULL) {
                xil_printf("\t\t Kernal %d - S(%d), R(%p), K1(%08x), K2(%08x), B(%08x)\r\n",
                    kernals->data.index,
                    kernals->data.state,
                    kernals->data.reference,
                    kernals->data.Kernal.Kernal_1,
                    kernals->data.Kernal.Kernal_2,
                    // kernals->data.Kernal.Kernal_3,
                    // kernals->data.Kernal.Kernal_4,
                    // kernals->data.Kernal.Kernal_5,
                    // kernals->data.Kernal.Kernal_6,
                    // kernals->data.Kernal.Kernal_7,
                    // kernals->data.Kernal.Kernal_8,
                    // kernals->data.Kernal.Kernal_9,
                    kernals->data.Bias
                    );
                kernals = kernals->next;
            }
            // xil_printf("\t\tIndex %d\n", channel->data.index);
            // xil_printf("\t\tSate  %d\n", channel->data.state);
            // xil_printf("\t\tChannel Count %d\n", channel->data.kernal_data_count);
            // xil_printf("\t\tHeight        %d\n", channel->data.height);
            // xil_printf("\t\tWidth         %d\n", channel->data.width);
            // xil_printf("\t\tTotal bytes   %d\n", channel->data.total_bytes);
            // xil_printf("\t\tInput Ptr     %p\n", channel->data.input_ptr);
            // xil_printf("\t\tOutput Ptr    %p\n", channel->data.output_ptr);
            // xil_printf("\t\tTemp Ptr      %p\n", channel->data.temp_ptr);
            channel = channel->next;
        }


        layer = layer->next;
    }
}

void LAYER_CNN_1_init_cb(Layer *layer, Layer prev_layer){
    UNUSED(prev_layer);

    // adding input channels
    LAYER_add_input_channel(layer, INPUT_SIZE, INPUT_SIZE, (u32*)&image_channel_red);
    LAYER_add_input_channel(layer, INPUT_SIZE, INPUT_SIZE, (u32*)&image_channel_green);
    LAYER_add_input_channel(layer, INPUT_SIZE, INPUT_SIZE, (u32*)&image_channel_blue);

    // adding output channels
    LAYER_add_cnn_output_channels(&layer, (void*)&layer_1_f10_weights, (void*)&PRelu_Layer_2_10_weights, 10, (INPUT_SIZE-2), (INPUT_SIZE-2));
}

void LAYER_CNN_2_init_cb(Layer *layer, Layer prev_layer){
    Channel_Node * prev_output_channels = NULL;

    prev_output_channels = prev_layer.output_channels.channels;

    while(prev_output_channels != NULL){
        LAYER_add_input_channel(layer, prev_output_channels->data.height, prev_output_channels->data.width, prev_output_channels->data.output_ptr);

        prev_output_channels = (Channel_Node *)prev_output_channels->next;
    }

    layer->input_channels.count    = prev_layer.output_channels.count;

    // adding output channels
    LAYER_add_cnn_output_channels(&layer, (void*)&layer_4_f16_weights, (void*)&PRelu_Layer_5_16_weights, 16, CNN_OUTPUT_SIZE_2, CNN_OUTPUT_SIZE_2);
}

void LAYER_CNN_3_init_cb(Layer *layer, Layer prev_layer){
    Channel_Node * prev_output_channels = NULL;

    prev_output_channels = prev_layer.output_channels.channels;

    while(prev_output_channels != NULL){
        LAYER_add_input_channel(layer, prev_output_channels->data.height, prev_output_channels->data.width, prev_output_channels->data.output_ptr);

        prev_output_channels = (Channel_Node *)prev_output_channels->next;
    }

    layer->input_channels.count = prev_layer.output_channels.count;

    // adding output channels
    LAYER_add_cnn_output_channels(&layer, (void*)&layer_6_f32_weights, (void*)&PRelu_Layer_7_32_weights, 32, CNN_OUTPUT_SIZE_3, CNN_OUTPUT_SIZE_3);

}

void LAYER_CNN_4_init_cb(Layer *layer, Layer prev_layer){
    Channel_Node * prev_output_channels = NULL;

    prev_output_channels = prev_layer.output_channels.channels;

    while(prev_output_channels != NULL){
        LAYER_add_input_channel(layer, prev_output_channels->data.height, prev_output_channels->data.width, prev_output_channels->data.output_ptr);

        prev_output_channels = (Channel_Node *)prev_output_channels->next;
    }

    layer->input_channels.count = prev_layer.output_channels.count;

    // adding output channels
    LAYER_add_cnn_1x1_output_channels(&layer, (void*)&layer_8_f2_weights, (void*)&layer_8_f2_bias, 64, 2, CNN_OUTPUT_SIZE_4, CNN_OUTPUT_SIZE_4);

}

void LAYER_CNN_5_init_cb(Layer *layer, Layer prev_layer){
    Channel_Node * prev_output_channels = NULL;

    prev_output_channels = prev_layer.output_channels.channels;

    while(prev_output_channels != NULL){
        LAYER_add_input_channel(layer, prev_output_channels->data.height, prev_output_channels->data.width, prev_output_channels->data.output_ptr);

        prev_output_channels = (Channel_Node *)prev_output_channels->next;
    }

    layer->input_channels.count = prev_layer.output_channels.count;

    // adding output channels
    LAYER_add_cnn_1x1_output_channels(&layer, (void*)&layer_9_f4_weights, (void*)&layer_9_f4_bias, 128, 4, CNN_OUTPUT_SIZE_4, CNN_OUTPUT_SIZE_4);

}

void LAYER_MAXPOOLING_1_init_cb(Layer *layer, Layer prev_layer){
    UNUSED(prev_layer);

    LAYER_link(&prev_layer, layer);

    // adding output channels
    LAYER_add_maxpool_output_channels(
        &layer, 
        MAX_POOLING_POOL_SIZE_1, 
        MAX_POOLING_STRIDE_1, 
        MAX_POOLING_PADDING_1, 
        MAX_POOLING_OUT_CHANNELS,
        MAX_POOLING_OUT_SIZE, 
        MAX_POOLING_OUT_SIZE);
}

int main() {

    NeuralNetwork *pnet_model = NULL;
    Layer *prev_layer = NULL;
    Layer *prev_layer_1 = NULL;
    Layer *prev_layer_2 = NULL;
    int ret = 0;

    // Task code
    xil_printf("System Task\r\n");

    NEURAL_NETWORK_init(&pnet_model, (u32*)NN_RECEIVE_MEM_BASE);
    prev_layer = NEURAL_NETWORK_add_layer(pnet_model, LAYER_TYPE_CNN_3X3,        (Layer_init_cb*)LAYER_CNN_1_init_cb,        NULL,       (u32*) NN_MEM_POOL_1_BASE, NN_MEM_POOL_1_LEN, LAYER_ACTIVATION_RELU);
    prev_layer = NEURAL_NETWORK_add_layer(pnet_model, LAYER_TYPE_MAXPOOLING,     (Layer_init_cb*)LAYER_MAXPOOLING_1_init_cb, prev_layer, (u32*) NN_MEM_POOL_2_BASE, NN_MEM_POOL_2_LEN, LAYER_ACTIVATION_NOT_REQUIRED);
    prev_layer = NEURAL_NETWORK_add_layer(pnet_model, LAYER_TYPE_CNN_3X3,        (Layer_init_cb*)LAYER_CNN_2_init_cb,        prev_layer, (u32*) NN_MEM_POOL_1_BASE, NN_MEM_POOL_1_LEN, LAYER_ACTIVATION_RELU);
    prev_layer = NEURAL_NETWORK_add_layer(pnet_model, LAYER_TYPE_CNN_3X3,        (Layer_init_cb*)LAYER_CNN_3_init_cb,        prev_layer, (u32*) NN_MEM_POOL_2_BASE, NN_MEM_POOL_2_LEN, LAYER_ACTIVATION_RELU);
    // branch 1
    prev_layer_1 = NEURAL_NETWORK_add_layer(pnet_model, LAYER_TYPE_CNN_1X1,      (Layer_init_cb*)LAYER_CNN_4_init_cb,        prev_layer, (u32*) NN_MEM_POOL_3_BASE, NN_MEM_POOL_3_LEN, LAYER_ACTIVATION_SOFTMAX);
    // branch 2
    prev_layer_2 = NEURAL_NETWORK_add_layer(pnet_model, LAYER_TYPE_CNN_1X1,      (Layer_init_cb*)LAYER_CNN_5_init_cb,        prev_layer, (u32*) NN_MEM_POOL_1_BASE, NN_MEM_POOL_1_LEN, LAYER_ACTIVATION_NOT_REQUIRED);

    //NEURAL_NETWORK_layer_link(pnet_model);

    // Test_NN_Model(pnet_model);

    // TickType_t tickCount = xTaskGetTickCount();
    NEURAL_NETWORK_process(pnet_model);
    // printf("\n\n time %d \n", (xTaskGetTickCount() - tickCount) );
    // vTaskDelay(pdMS_TO_TICKS(100));
    // NEURAL_NETWORK_get_output(&NN_model);
    xil_printf("completed \n");

    // Test_NN_Model(pnet_model);

    while(TRUE){
        xil_printf("System Task Running\r\n");
        // vTaskDelay(100);
    }

    return 0;
}

