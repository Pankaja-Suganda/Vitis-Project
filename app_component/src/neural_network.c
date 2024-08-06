#include "neural_network.h"
#include "xscugic.h"
#include "xparameters.h"

#define NET_ENGINE_1_AXI_DMA_BASEADDR XPAR_AXI_DMA_0_BASEADDR
#define NET_ENGINE_1_CONFIG_BASEADDR  XPAR_NET_ENGINE_0_BASEADDR

int NEURAL_NETWORK_setup_net_engine(Net_Engine_Inst *instance){
    NET_STATUS Status;

    // net engine initializing
    Status = NET_ENGINE_init(instance, NET_ENGINE_1_CONFIG_BASEADDR, NET_ENGINE_1_AXI_DMA_BASEADDR);
    if(Status != NET_ENGINE_OK){
        xil_printf("Net engine init failed\n");
    }

    // intterupt setup
    Status = NET_ENGINE_intr_setup(instance, XPAR_XSCUGIC_0_BASEADDR);
    if(Status != NET_ENGINE_OK){
        xil_printf("Net engine interrupt setup failed\n");
    }

    // register interrupts
    Status = NET_ENGINE_register_intr(instance, NET_ENGINE_RECEIVE_INTR, XPS_FPGA1_INT_ID);
    if(Status != NET_ENGINE_OK){
        xil_printf("Net engine register NET_ENGINE_RECEIVE_INTR failed\n");
    }

    Status = NET_ENGINE_register_intr(instance, NET_ENGINE_ROW_COMPLETE_INTR, XPS_FPGA2_INT_ID);
    if(Status != NET_ENGINE_OK){
        xil_printf("Net engine register NET_ENGINE_ROW_COMPLETE_INTR failed\n");
    }
    return 0;
}

int NEURAL_NETWORK_init(NeuralNetwork **instance, u32 *receive_memory_ptr){
    int ret = 0;
    *instance = (NeuralNetwork *)malloc(sizeof(NeuralNetwork));
    if (instance == NULL) {
        printf("Layer malloc failed\n");
        return -1;
    }

    (*instance)->layers          = NULL;
    (*instance)->layer_count     = 0;
    (*instance)->completed_count = 0;
    (*instance)->status          = NN_STATE_NOT_STARTED;
    (*instance)->receive_memory_ptr    = receive_memory_ptr;

    ret = NEURAL_NETWORK_setup_net_engine(&(*instance)->net_engine);

    return ret;
}

static NN_Layer_Node* create_layer_node(Layer layer){
    NN_Layer_Node* new = (NN_Layer_Node*)malloc(sizeof(Layer));
    if(new == NULL){
        xil_printf("Node malloc error \r\n");
        return NULL;
    }
    new->layer = layer;
    new->next  = NULL;
    new->prev  = NULL;
    return new;
}

static void append_layer_node(NN_Layer_Node** head_ref, Layer new_data) {
    NN_Layer_Node* new = create_layer_node(new_data);
    if (new == NULL) {
        return;
    }

    if (*head_ref == NULL) {
        *head_ref = new;
        return;
    }

    NN_Layer_Node* last = *head_ref;
    while (last->next != NULL) {
        last = last->next;
    }

    last->prev = last->next;
    last->next = new;
}

Layer* NEURAL_NETWORK_add_layer(NeuralNetwork *instance, LAYER_TYPE type, Layer_init_cb init_cb, Layer *prev_layer, u32* memory_ptr, u32 memory_len, LAYER_ACTIVATION activation){
    Layer* new_layer;

    if(instance == NULL){
        return NULL;
    }

    new_layer = LAYER_init(type,  activation, memory_ptr,  memory_len);
    if(new_layer == NULL){
        return NULL;
    }

    init_cb(new_layer, *prev_layer);

    new_layer->index = instance->layer_count;
    // allocating memory locations for each output channels
    Channel_Node* cur_channel     = new_layer->output_channels.channels;
    new_layer->memory.memory_ptr  = memory_ptr;
    new_layer->memory.memory_tail = memory_ptr;
    new_layer->memory.availale_mem_size = memory_len;
    new_layer->memory.used_mem_size     = 0;




    // check whether the channel loaded
    if(cur_channel == NULL){
        xil_printf("No output channel available \r\n");
    }

    while (cur_channel != NULL){
        // calling preprocess function
        // cur_channel->data.output_ptr          = instance->memory_tail;
        // cur_channel->data.temp_ptr            = instance->receive_memory_ptr;

        // jumping to next channel
        cur_channel = cur_channel->next;
    }

    if(instance->layers == NULL){
        instance->layers = create_layer_node(*new_layer);
    }
    else{
        append_layer_node(&(instance->layers), *new_layer);
    }

    instance->layer_count++;
    return new_layer;
}

int NEURAL_NETWORK_layer_link(NeuralNetwork *instance){
    NN_Layer_Node *layer_cur  = NULL;
    NN_Layer_Node *layer_prev = NULL;

    layer_cur = instance->layers;
    while(layer_cur != NULL){
        if(layer_prev != NULL){
            LAYER_link(&(layer_prev->layer), &(layer_cur->layer));
        }
        
        layer_prev = layer_cur;
        layer_cur  = layer_cur->next;
    }
    return 0;
}

int NEURAL_NETWORK_process(NeuralNetwork *instance){
    // allocating memory locations for each output channels
    NN_Layer_Node* cur_layer = instance->layers;

    // check whether the layer loaded
    if(cur_layer == NULL){
        xil_printf("No layer available \r\n");
        return 0;
    }

    while (cur_layer != NULL){
        LAYER_process(&(cur_layer->layer), &(instance->net_engine));
        // LAYER_CNN_process(&(cur_layer->layer));

        // jumping to next laer
        cur_layer = cur_layer->next;
    }

    return 0;
}

int NEURAL_NETWORK_update(NeuralNetwork *instance, int height, int width){
    int height_, width_;
    NN_Layer_Node* cur_layer  = instance->layers;
    NN_Layer_Node* prev_layer = instance->layers;

    // check whether the layer loaded
    if(cur_layer == NULL){
        xil_printf("No layer available \r\n");
        return 0;
    }

    while (cur_layer != NULL){

        LAYER_update(&(cur_layer->layer), &(prev_layer->layer), height, width);

        // jumping to next laer
        prev_layer = cur_layer;
        cur_layer  = cur_layer->next;
        
    }
}

// int NEURAL_NETWORK_predict(NeuralNetwork *instance){
//     return 0;
// }

// void NEURAL_NETWORK_cleanup(NeuralNetwork *instance){
    
// }