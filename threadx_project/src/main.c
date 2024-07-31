#include "FreeRTOS.h"
#include "task.h"
#include "task_priority.h"
#include <xil_printf.h>
#include <xil_types.h>
#include "neural_network.h"
#include "layer.h"
#include "memory_map.h"
#include "test_sample.h"
#include "conv_layer.h"

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

#define INPUT_SIZE  100
#define OUTPUT_SIZE 98

static TaskHandle_t system_task_handler;
static NeuralNetwork NN_model;

static CNN_Layer CNN_Layer_1;
static CNN_Layer CNN_Layer_2;
static CNN_Layer CNN_Layer_3;

static Channel   input_R = {0}, input_G = {0}, input_B = {0};
static Channel   out_chan[10];

void LAYER_1_init(CNN_Layer *layer){
    int ret = 0;
    ret = LAYER_CNN_init(layer, (u32*)0, 0);

    // creating input channels
    CHANNEL_init(&input_R, CHANNEL_TYPE_INPUT, INPUT_SIZE, INPUT_SIZE, (u32*)&image_channel_red);
    CHANNEL_init(&input_G, CHANNEL_TYPE_INPUT, INPUT_SIZE, INPUT_SIZE, (u32*)&image_channel_green);
    CHANNEL_init(&input_B, CHANNEL_TYPE_INPUT, INPUT_SIZE, INPUT_SIZE, (u32*)&image_channel_blue);

    // adding input channels
    LAYER_add_channel(&CNN_Layer_1, input_R);
    LAYER_add_channel(&CNN_Layer_1, input_G);
    LAYER_add_channel(&CNN_Layer_1, input_B);

    // creating output channels
    for(int i = 0; i < 1; i++){
        CHANNEL_init(&out_chan[i], CHANNEL_TYPE_OUTPUT, OUTPUT_SIZE, OUTPUT_SIZE, NULL);

        CHANNEL_load_kernal(&out_chan[i], layer_1_f10_weights[(i*3) + 0], &input_R);
        CHANNEL_load_kernal(&out_chan[i], layer_1_f10_weights[(i*3) + 1], &input_G);
        CHANNEL_load_kernal(&out_chan[i], layer_1_f10_weights[(i*3) + 2], &input_B);

        LAYER_add_channel(&CNN_Layer_1, out_chan[i]);
    }
}

void LAYER_2_init(CNN_Layer *layer){
    
}

void LAYER_3_init(CNN_Layer *layer){
    
}

void vSystemTask(void *pvParameters) {
    int ret = 0;
    // Task code
    xil_printf("System Task\r\n");

    LAYER_1_init(&CNN_Layer_1);
    LAYER_2_init(&CNN_Layer_2);
    LAYER_3_init(&CNN_Layer_3);

    NEURAL_NETWORK_init(&NN_model, (u32*)FIRST_CONV_LAYER_MEM_BASE, FIRST_CONV_LAYER_MEM_LEN, (u32*)NN_RECEIVE_MEM_BASE);
    NEURAL_NETWORK_add_layer(&NN_model, CNN_Layer_1);
    // NEURAL_NETWORK_add_layer(&NN_model, CNN_Layer_2);
    // NEURAL_NETWORK_add_layer(&NN_model, CNN_Layer_3);
    TickType_t tickCount = xTaskGetTickCount();
    NEURAL_NETWORK_process(&NN_model);
    vTaskDelay(pdMS_TO_TICKS(100));
    printf("\n\n time %d \n", (xTaskGetTickCount() - tickCount) );
    // vTaskDelay(pdMS_TO_TICKS(100));
    // NEURAL_NETWORK_get_output(&NN_model);

    while(TRUE){
        xil_printf("System Task Running\r\n");
        vTaskDelay(100);
    }

    vTaskDelete(system_task_handler);
}

int main(void) {
    xTaskCreate(vSystemTask, "Task1", 20000, NULL, SYSTEM_TASK_PRIORITY, &system_task_handler);
    
    vTaskStartScheduler();

    for( ;; );
}
