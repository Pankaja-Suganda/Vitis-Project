
/*
 * imageIpTest.c
 *
 *  Created on: Apr 4, 2020
 *      Author: VIPIN
 */

#include "xaxidma.h"
#include "xparameters.h"
#include "sleep.h"
#include "xil_cache.h"
#include "xil_io.h"
#include "xscugic.h"
#include "test_sample.h"
#include "xuartps.h"

#include "net_engine.h"
#include "layer.h"


#define NET_ENGINE_1_AXI_DMA_BASEADDR XPAR_AXI_DMA_0_BASEADDR
#define NET_ENGINE_1_CONFIG_BASEADDR  XPAR_NET_ENGINE_0_BASEADDR


__attribute__((aligned(32))) u32 out_buffer[97*97*4] = {0};
__attribute__((aligned(32))) u32 in_buffer[100*100*4] = {0};

typedef u64 XTime;
XTime time_Global_start;
XTime time_Global_end;

int main(){
    int ret = 0;
    CNN_Layer cnn_layer;

    ret = LAYER_CNN_init(&cnn_layer, &in_buffer, &out_buffer);

    CNN_Config_Data data_1, data_2;

    data_1.Bias = 1;
    data_1.Kernal.Kernal_1 = 1;
    data_1.Kernal.Kernal_2 = 1;
    data_1.Kernal.Kernal_3 = 1;
    data_1.Kernal.Kernal_4 = 1;
    data_1.Kernal.Kernal_5 = 1;
    data_1.Kernal.Kernal_6 = 1;
    data_1.Kernal.Kernal_7 = 1;
    data_1.Kernal.Kernal_8 = 1;
    data_1.Kernal.Kernal_9 = 1;
    data_1.index = 0;

    data_2.Bias = 2;
    data_2.Kernal.Kernal_1 = 2;
    data_2.Kernal.Kernal_2 = 2;
    data_2.Kernal.Kernal_3 = 2;
    data_2.Kernal.Kernal_4 = 2;
    data_2.Kernal.Kernal_5 = 2;
    data_2.Kernal.Kernal_6 = 2;
    data_2.Kernal.Kernal_7 = 2;
    data_2.Kernal.Kernal_8 = 2;
    data_2.Kernal.Kernal_9 = 2;
    data_2.index = 1;

    for(int i = 0 ; i < 1000; i++){    
        data_1.index  = i;
        ret = LAYER_CNN_load_data(&cnn_layer, data_1);
        // ret = LAYER_CNN_load_data(&cnn_layer, data_2);
        // ret = LAYER_CNN_load_data(&cnn_layer, data_1);
        // ret = LAYER_CNN_load_data(&cnn_layer, data_2);
    }
    
    XTime_GetTime(&time_Global_start);
    xil_printf("Before Time - %d\n", time_Global_start);
    ret = LAYER_CNN_process(&cnn_layer);
    XTime_GetTime(&time_Global_end);
    xil_printf("After Time - %d\n", time_Global_end);
    
    return 0;
}

// int main(){
//     NET_STATUS Status;
//     Net_Engine_Inst net_engine;

//     // net engine initializing
//     Status = NET_ENGINE_init(&net_engine, NET_ENGINE_1_CONFIG_BASEADDR, NET_ENGINE_1_AXI_DMA_BASEADDR);
//     if(Status == NET_ENGINE_OK){
//         xil_printf("Net engine init failed\n");
//     }

//     // intterupt setup
//     Status = NET_ENGINE_intr_setup(&net_engine, XPAR_XSCUGIC_0_BASEADDR);
//     if(Status == NET_ENGINE_OK){
//         xil_printf("Net engine interrupt setup failed\n");
//     }

//     // register interrupts
//     Status = NET_ENGINE_register_intr(&net_engine, NET_ENGINE_RECEIVE_INTR, XPS_FPGA1_INT_ID);
//     if(Status == NET_ENGINE_OK){
//         xil_printf("Net engine register NET_ENGINE_RECEIVE_INTR failed\n");
//     }

//     Status = NET_ENGINE_register_intr(&net_engine, NET_ENGINE_ROW_COMPLETE_INTR, XPS_FPGA2_INT_ID);
//     if(Status == NET_ENGINE_OK){
//         xil_printf("Net engine register NET_ENGINE_ROW_COMPLETE_INTR failed\n");
//     }

//     XTime time_Global_start;
//     XTime time_Global_end;
//     XTime_GetTime(&time_Global_start);
//     xil_printf("Before Time - %d\n", time_Global_start);
//     Status = NET_ENGINE_process_maxpooling(&net_engine, imageData, out_buffer);
//     NET_ENGINE_reset(&net_engine);
//     // Status = NET_ENGINE_process_cnn(&net_engine, imageData, out_buffer, imageData, imageData[0]);
//     XTime_GetTime(&time_Global_end);
//     xil_printf("After Time - %d\n", time_Global_end - time_Global_start);

//     // NET_ENGINE_reset(&net_engine);

//     // XTime_GetTime(&time_Global_start);
//     // xil_printf("Before Time - %d\n", time_Global_start);
//     // Status = NET_ENGINE_process_maxpooling(&net_engine, imageData, out_buffer);
//     // XTime_GetTime(&time_Global_end);
//     // xil_printf("After Time - %d\n", time_Global_end - time_Global_start);

//     // NET_ENGINE_CNN_layer_Process(&net_engine, imageData, out_buffer)
// }
