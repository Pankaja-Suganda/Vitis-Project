#ifndef NEURAL_NETWORK_H
#define NEURAL_NETWORK_H

#include "xil_types.h"
#include "layer.h"

typedef enum{
    NN_STATE_NOT_STARTED,
    NN_STATE_BUSY,
    NN_STATE_COMPLETED,
}NN_STATE;

typedef struct NN_Layer_Node_{
    struct NN_Layer_Node *next;
    struct NN_Layer_Node *prev;
    CNN_Layer             layer;
} NN_Layer_Node;


typedef struct NeuralNetwork {
    NN_Layer_Node *layers;
    int layer_count;
    int completed_count;                
    u32 *memory_pool;
    u32 *memory_tail;                
    u32 used_memory_size;
    u32 available_memory_size;         
    u32 *receive_memory_ptr;        
    NN_STATE status;
} NeuralNetwork;

int NEURAL_NETWORK_init(NeuralNetwork *instance, u32 *memory, u32 memory_size, u32 *receive_memory_ptr);

int NEURAL_NETWORK_add_layer(NeuralNetwork *instance, CNN_Layer layer);

int NEURAL_NETWORK_process(NeuralNetwork *instance);

int NEURAL_NETWORK_get_output(NeuralNetwork *instance);

void NEURAL_NETWORK_cleanup(NeuralNetwork *instance);

#endif // NEURAL_NETWORK_H
