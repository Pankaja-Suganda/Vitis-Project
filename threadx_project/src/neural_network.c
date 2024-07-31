#include "neural_network.h"

int NEURAL_NETWORK_init(NeuralNetwork *instance, u32 *memory, u32 memory_size, u32 *receive_memory_ptr){
    instance->layers          = NULL;
    instance->layer_count     = 0;
    instance->completed_count = 0;
    instance->memory_pool     = memory;
    instance->memory_tail     = memory;
    instance->status          = NN_STATE_NOT_STARTED;

    instance->used_memory_size      = 0;
    instance->available_memory_size = memory_size;
    instance->receive_memory_ptr    = receive_memory_ptr;
    return 0;
}

static NN_Layer_Node* create_layer_node(CNN_Layer layer){
    NN_Layer_Node* new = (NN_Layer_Node*)malloc(sizeof(NN_Layer_Node));
    if(new == NULL){
        xil_printf("Node malloc error \r\n");
        return NULL;
    }
    new->layer = layer;
    new->next  = NULL;
    new->prev  = NULL;
    return new;
}

static void append_layer_node(NN_Layer_Node** head_ref, CNN_Layer new_data) {
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

int NEURAL_NETWORK_add_layer(NeuralNetwork *instance, CNN_Layer layer){

    // allocating memory locations for each output channels
    Channel_Node* cur_channel = layer.output_channels;
    layer.memory_ptr          = instance->memory_tail;

    // check whether the channel loaded
    if(cur_channel == NULL){
        xil_printf("No output channel available \r\n");
    }

    while (cur_channel != NULL){
        // calling preprocess function
        // cur_channel->data.output_ptr = instance->memory_tail;
        cur_channel->data.output_ptr = 0
        cur_channel->data.temp_ptr   = instance->receive_memory_ptr;
        instance->memory_tail       += cur_channel->data.total_bytes;

        // jumping to next channel
        cur_channel = cur_channel->next;
    }

    if(instance->layers == NULL){
        instance->layers = create_layer_node(layer);
    }
    else{
        append_layer_node(&(instance->layers), layer);
    }
    instance->layer_count++;
    return 0;
}

int NEURAL_NETWORK_process(NeuralNetwork *instance){
    // allocating memory locations for each output channels
    NN_Layer_Node* cur_layer = instance->layers;

    // check whether the channel loaded
    if(cur_layer == NULL){
        xil_printf("No layer available \r\n");
        return 0;
    }

    while (cur_layer != NULL){
        LAYER_CNN_process(&(cur_layer->layer));

        // jumping to next channel
        cur_layer = cur_layer->next;
    }
    return 0;
}

int NEURAL_NETWORK_get_output(NeuralNetwork *instance){
    return 0;
}

void NEURAL_NETWORK_cleanup(NeuralNetwork *instance){
    
}