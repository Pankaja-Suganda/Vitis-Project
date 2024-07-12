#include "layer.h"
#include "net_engine.h"
#include "net_engine_hw.h"
#include "xscugic.h"
#include "xparameters.h"
#include <stdlib.h>

#define NET_ENGINE_1_AXI_DMA_BASEADDR XPAR_AXI_DMA_0_BASEADDR
#define NET_ENGINE_1_CONFIG_BASEADDR  XPAR_NET_ENGINE_0_BASEADDR


int LAYER_setup_net_engine(Net_Engine_Inst *instance){
    NET_STATUS Status;

    // net engine initializing
    Status = NET_ENGINE_init(instance, NET_ENGINE_1_CONFIG_BASEADDR, NET_ENGINE_1_AXI_DMA_BASEADDR);
    if(Status == NET_ENGINE_OK){
        xil_printf("Net engine init failed\n");
    }

    // intterupt setup
    Status = NET_ENGINE_intr_setup(instance, XPAR_XSCUGIC_0_BASEADDR);
    if(Status == NET_ENGINE_OK){
        xil_printf("Net engine interrupt setup failed\n");
    }

    // register interrupts
    Status = NET_ENGINE_register_intr(instance, NET_ENGINE_RECEIVE_INTR, XPS_FPGA1_INT_ID);
    if(Status == NET_ENGINE_OK){
        xil_printf("Net engine register NET_ENGINE_RECEIVE_INTR failed\n");
    }

    Status = NET_ENGINE_register_intr(instance, NET_ENGINE_ROW_COMPLETE_INTR, XPS_FPGA2_INT_ID);
    if(Status == NET_ENGINE_OK){
        xil_printf("Net engine register NET_ENGINE_ROW_COMPLETE_INTR failed\n");
    }
    return 0;
}

int LAYER_Max_pooling_set_data(Max_Pooling_Layer *instance, u32* data, u16 count){
    xil_printf("LAYER_Max_pooling_set_data \n");
    return 0;
}

int LAYER_Max_pooling_init(Max_Pooling_Layer *instance, u32* input, u32* output){
    int ret = 0;

    // set up max poolng details
    instance->completed_count = 0;
    instance->total_count     = 0;

    // set up layer details
    instance->layer.index  = 1;
    instance->layer.input  = input;
    instance->layer.output = output;
    instance->layer.temp   = output;
    instance->layer.state  = LAYER_STATE_NOT_STARTED;

    // set up net engine
    ret = LAYER_setup_net_engine(&(instance->layer.net_engine));

    return ret;
}

int LAYER_Max_pooling_process(Max_Pooling_Layer *instance){
    int ret = 0;

    // check data availability
    // process it
    return ret;
}

int LAYER_CNN_init(CNN_Layer *instance, u32* input, u32* output){
    int ret = 0;

    // set up max poolng details
    instance->completed_count = 0;
    instance->total_count     = 0;

    // set up layer details
    instance->layer.index  = 1;
    instance->layer.input  = input;
    instance->layer.output = output;
    instance->layer.temp   = output;
    instance->layer.state  = LAYER_STATE_NOT_STARTED;
    instance->data         = NULL;

    // set up net engine
    ret = LAYER_setup_net_engine(&(instance->layer.net_engine));

    return ret;
}



CNN_Data_Node* create_data_node(CNN_Config_Data data){
    CNN_Data_Node* new = (CNN_Data_Node*)malloc(sizeof(CNN_Config_Data));
    if(new == NULL){
        xil_printf("Node malloc error \r\n");
        return NULL;
    }
    new->config_data = data;
    new->next        = NULL;

    return new;
}

void append_node(CNN_Data_Node** head_ref, CNN_Config_Data new_data) {
    CNN_Data_Node* new = create_data_node(new_data);
    if (new == NULL) {
        return;
    }

    if (*head_ref == NULL) {
        *head_ref = new;
        return;
    }

    CNN_Data_Node* last = *head_ref;
    while (last->next != NULL) {
        last = last->next;
    }

    last->next = new;
}

void print_list(CNN_Data_Node* node) {
    while (node != NULL) {
        xil_printf("%d -> ", node->config_data.index);
        node = node->next;
    }
    xil_printf("NULL\n");
}

int LAYER_CNN_load_data(CNN_Layer *instance, CNN_Config_Data data){

    data.state = CONFIG_DATA_STATE_NOT_STARTED;

    if(instance->data == NULL){
        instance->data = create_data_node(data);
    }
    else{
        append_node(&(instance->data), data);
    }

    instance->total_count++;

    return 0;
}


int LAYER_CNN_process(CNN_Layer *instance){
    int ret = 0;
    CNN_Data_Node* node = instance->data;

    // check whether the dta loaded
    if(node == NULL){
        xil_printf("No CNN Data Available \r\n");
        return 0;
    }

    while (node != NULL) {
        xil_printf("CNN Processing %d \r\n", node->config_data.index);

        ret = NET_ENGINE_process_cnn(&(instance->layer.net_engine), instance->layer.input, instance->layer.output, node->config_data);
        node = node->next;
    }

    xil_printf("CNN Process Completed \r\n");

    return ret;
}

