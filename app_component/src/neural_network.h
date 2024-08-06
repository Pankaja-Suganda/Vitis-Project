#ifndef NEURAL_NETWORK_H
#define NEURAL_NETWORK_H

#include "xil_types.h"
#include "layer.h"
#include "net_engine.h"

typedef enum{
    NN_STATE_NOT_STARTED,
    NN_STATE_BUSY,
    NN_STATE_COMPLETED,
    NN_STATE_ERROR,
}NN_STATE;

typedef struct NN_Layer_Node_{
    struct NN_Layer_Node *next;
    struct NN_Layer_Node *prev;
    Layer                layer;
} NN_Layer_Node;


typedef struct NeuralNetwork {
    NN_Layer_Node *layers;
    int layer_count;
    int completed_count;               
    u32 *receive_memory_ptr;        
    NN_STATE status;
    Net_Engine_Inst net_engine;
} NeuralNetwork;

int NEURAL_NETWORK_init(NeuralNetwork **instance, u32 *receive_memory_ptr);

Layer* NEURAL_NETWORK_add_layer(NeuralNetwork *instance, LAYER_TYPE type, Layer_init_cb init_cb, Layer *prev_layer, u32* memory_ptr, u32 memory_len, LAYER_ACTIVATION activation);

int NEURAL_NETWORK_layer_link(NeuralNetwork *instance);

int NEURAL_NETWORK_update(NeuralNetwork *instance, int height, int width);

// int NEURAL_NETWORK_process(NeuralNetwork *instance);

// int NEURAL_NETWORK_get_output(NeuralNetwork *instance);

// void NEURAL_NETWORK_cleanup(NeuralNetwork *instance);

#endif // NEURAL_NETWORK_H
