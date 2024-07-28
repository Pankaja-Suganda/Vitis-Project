// #ifndef NEURAL_NETWORK_H
// #define NEURAL_NETWORK_H

// #include "layer.h"

// typedef struct NeuralNetwork {
//     CNN_Layer layers[MAX_LAYERS];  // Array of layers
//     int layer_count;                 // Current number of layers
//     Net_Engine_Inst net_engine;      // Instance of the net engine
//     u32 *input_data;                 // Pointer to input data
//     u32 *output_data;                // Pointer to output data
//     u32 *memory_pool;                // Memory pool for layer processing
//     u32 memory_size;                 // Size of the memory pool
// } NeuralNetwork;

// NET_STATUS NeuralNetwork_init(NeuralNetwork *nn, u32 *memory, u32 memory_size);

// NET_STATUS NeuralNetwork_add_layer(NeuralNetwork *nn, CNN_Layer *layer);

// NET_STATUS NeuralNetwork_process(NeuralNetwork *nn);

// void NeuralNetwork_cleanup(NeuralNetwork *nn);

// #endif // NEURAL_NETWORK_H
