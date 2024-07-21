

#include "xaxidma.h"
#include "xparameters.h"
#include "sleep.h"
#include "xil_cache.h"
#include "xil_io.h"
#include "xil_util.h"
#include "xscugic.h"
#include "test_sample.h"
#include "xuartps.h"

#include "net_engine.h"
#include "layer.h"
#include "conv_layer.h"
#include <stdio.h>
#include <xil_printf.h>
#include "channels.h"


#define NET_ENGINE_1_AXI_DMA_BASEADDR XPAR_AXI_DMA_0_BASEADDR
#define NET_ENGINE_1_CONFIG_BASEADDR  XPAR_NET_ENGINE_0_BASEADDR

// #ifndef DDR_BASE_ADDR
// #warning CHECK FOR THE VALID DDR ADDRESS IN XPARAMETERS.H, \
// DEFAULT SET TO 0x01000000
// #define MEM_BASE_ADDR		0x01000000
// #else
// #define MEM_BASE_ADDR		(DDR_BASE_ADDR + 0x1000000)
// #endif

// #define DEFAULT_ROW_LEN     100
// #define DEFAULT_BUFFER_LEN  (DEFAULT_ROW_LEN * DEFAULT_ROW_LEN * 4)

// #define TX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00100000)
// #define RX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00300000)
// #define RX_BUFFER_HIGH		(MEM_BASE_ADDR + 0x004FFFFF)

#define ALL_BUFFERS_START   (MEM_BASE_ADDR + 0x00100000)
#define ALL_BUFFERS_LEN     (DEFAULT_BUFFER_LEN * 4)
#define ALL_BUFFERS_END     (ALL_BUFFERS_START + ALL_BUFFERS_LEN)

#define INPUT_BUFFER_START  ALL_BUFFERS_START
#define INPUT_BUFFER_LEN    DEFAULT_ROW_LEN
#define INPUT_BUFFER_END    (INPUT_BUFFER_START + INPUT_BUFFER_LEN)

#define OUTPUT_BUFFER_START INPUT_BUFFER_END
#define OUTPUT_BUFFER_LEN   DEFAULT_ROW_LEN
#define OUTPUT_BUFFER_END   (OUTPUT_BUFFER_START + OUTPUT_BUFFER_LEN)
 
#define TEMP_BUFFER_START   OUTPUT_BUFFER_END
#define TEMP_BUFFER_LEN     DEFAULT_ROW_LEN
#define TEMP_BUFFER_END     (TEMP_BUFFER_START + TEMP_BUFFER_LEN)



static __attribute__((aligned(32))) u32 out_buffer[97*97*4] = {0};
// static u32 out_buffer[97*97*4] = {0};
// static u32 in_buffer[100*100*4] = {0};
static __attribute__((aligned(32))) u32 in_buffer[100*100*4] = {0};

typedef u64 XTime;
XTime time_Global_start;
XTime time_Global_end;


void post_process(Layer layer){
    float *out_ptr = (float*)layer.output;
    float *temp_ptr = (float*)layer.temp;
    int Index = 0;
    printf("Post Process\r\n");
    // for (int Index = 0; Index < 10; Index++) {
    //     printf("%f, ", (float)out_ptr[Index]);
    //     temp_ptr[Index] = (out_ptr[Index]>0)? out_ptr[Index]: 0;
    // }
    printf("\n");
}

void pre_process(Layer layer){
    xil_printf("Pre Process\r\n");
}

int main(){
    int ret = 0;
    CNN_Layer conv_layer_1;
    Channel   input_R, input_G, input_B;
    Channel   output_1, output_2;

    Channel_Kernal_Data data_1 = {1}, data_2 = {2}, data_3 = {3};
    Channel_Kernal_Data data_4 = {4}, data_5 = {5}, data_6 = {6};
    ret = LAYER_CNN_init(&conv_layer_1);

    CHANNEL_init(&input_R, CHANNEL_TYPE_INPUT, (float*)&image_data);
    CHANNEL_init(&input_G, CHANNEL_TYPE_INPUT, (float*)&image_data);
    CHANNEL_init(&input_B, CHANNEL_TYPE_INPUT, (float*)&image_data);

    LAYER_add_channel(&conv_layer_1, input_R);
    LAYER_add_channel(&conv_layer_1, input_B);
    LAYER_add_channel(&conv_layer_1, input_G);

    CHANNEL_init(&output_1, CHANNEL_TYPE_OUTPUT, NULL);
    CHANNEL_init(&output_2, CHANNEL_TYPE_OUTPUT, NULL);

    CHANNEL_load_kernal(&output_1, data_1, &input_R);
    CHANNEL_load_kernal(&output_1, data_2, &input_B);
    CHANNEL_load_kernal(&output_1, data_3, &input_G);

    data_6.Kernal.Kernal_1 =  0x3F800000;
    data_6.Bias =  0x3F800000;

    CHANNEL_load_kernal(&output_2, data_4, &input_R);
    CHANNEL_load_kernal(&output_2, data_5, &input_B);
    CHANNEL_load_kernal(&output_2, data_6, &input_G);
    
    LAYER_add_channel(&conv_layer_1, output_1);
    LAYER_add_channel(&conv_layer_1, output_2);

    LAYER_CNN_set_callbacks(&conv_layer_1, (Layer_Data_Post_Process*)&post_process, (Layer_Data_Pre_Process*)&pre_process);

    XTime_GetTime(&time_Global_start);
    xil_printf("Before Time - %d\n", time_Global_start);
    LAYER_CNN_process(&conv_layer_1);
    XTime_GetTime(&time_Global_end);
    xil_printf("After Time - %d - %d\n", time_Global_end, ret);

    return 0;
}
// typedef input_channel{
//     height
//     width
//     in_data*
//     total_bytes
//     status
// }

// typedef output_channel{
//     height
//     width
//     out_data*
//     kernal_data
//     total_bytes
//     status
// }

// typedef struct Layer_{
//     input_channel *in_channels
//     input_channels  3
//     output_channel *out_channels
//     output_channels 10
// } Layer;

// // add_input_channel(height, width, data); R
// // add_input_channel(height, width, data); G
// // add_input_channel(height, width, data); B

// add_input_channel(height, width, data); 1 channel

// add_output_channel(); 1 channel

// init_layer

// add_input_channels
// add_input_channels
// add_input_channels

// add_output_channels // should have kenal // need to manage memory
// add_output_channels

// }

// int main(){
//     int ret = 0;
//     CNN_Layer cnn_layer;
//     float *out_buffer_p;
//     float *in_buffer_p;  

//     // out_buffer_p = (u32*) &out_buffer;//RX_BUFFER_BASE;
//     in_buffer_p  = (float*) &image_data; //TX_BUFFER_BASE;
//     out_buffer_p = (float*) RX_BUFFER_BASE;
//     // in_buffer_p  = (float*) TX_BUFFER_BASE;

//     // for(int j = 0; j < 100 ; j++){
//     //     for(int i = 0; i < 100 ; i++){
//     //         in_buffer_p[(j*100)+i] = i ;//+ 1.1;
//     //         // xil_printf("j-%d, i-%d, pos-%d, %08x\r\n", j, i, (j*100)+i, &in_buffer_p[(j*100)+i]);
//     //     }
//     // }

//     ret = LAYER_CNN_init(&cnn_layer, in_buffer_p, out_buffer_p);

//     // for(int i = 0 ; i < 9; i++){   
//     //     ret = LAYER_CNN_load_data(&cnn_layer, cnn_config_data[i]);
//     // }

//     LAYER_CNN_set_callbacks(&cnn_layer, (Layer_Data_Post_Process*)&post_process, (Layer_Data_Pre_Process*)&pre_process);
//     // ret = LAYER_CNN_load_data(&cnn_layer, cnn_config_data[1]);
//     for(int i = 0 ; i < 3; i++){   
//         ret = LAYER_CNN_load_data(&cnn_layer, cnn_config_data[i]);
//     }
    
//     XTime_GetTime(&time_Global_start);
//     xil_printf("Before Time - %d\n", time_Global_start);
//     ret = LAYER_CNN_process(&cnn_layer);
//     XTime_GetTime(&time_Global_end);
//     xil_printf("After Time - %d - %d\n", time_Global_end, ret);

//     int row = 0;

// 	// for (int Index = 0; Index < (99); Index++) {
// 	// 	if(Index%10==0){
//     //         printf("\n");
//     //     }
// 	// 	if(Index%98==0){
//     //         printf(" (%d)\n", row);
//     //         row++;
//     //     }
// 		// printf("%f, ", out_buffer_p[Index]);
//         // printf("\t%p, %f, %d - %p, %f, %d \n", &cnn_layer.layer.output[Index], (float)cnn_layer.layer.output[Index], sizeof(cnn_layer.layer.output[Index]), &out_buffer_p[Index], out_buffer_p[Index], sizeof(out_buffer_p[Index]));
// 	// }

//     return 0;
// }
