
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


#define NET_ENGINE_1_AXI_DMA_BASEADDR XPAR_AXI_DMA_0_BASEADDR
#define NET_ENGINE_1_CONFIG_BASEADDR  XPAR_NET_ENGINE_0_BASEADDR


__attribute__((aligned(32))) u32 out_buffer[97*97*4] = {0};

typedef u64 XTime;

int main(){
    NET_STATUS Status;
    Net_Engine_Inst net_engine;

    // net engine initializing
    Status = NET_ENGINE_init(&net_engine, NET_ENGINE_1_CONFIG_BASEADDR, NET_ENGINE_1_AXI_DMA_BASEADDR);
    if(Status == NET_ENGINE_OK){
        xil_printf("Net engine init failed\n");
    }

    // intterupt setup
    Status = NET_ENGINE_intr_setup(&net_engine, XPAR_XSCUGIC_0_BASEADDR);
    if(Status == NET_ENGINE_OK){
        xil_printf("Net engine interrupt setup failed\n");
    }

    // register interrupts
    Status = NET_ENGINE_register_intr(&net_engine, NET_ENGINE_RECEIVE_INTR, XPS_FPGA1_INT_ID);
    if(Status == NET_ENGINE_OK){
        xil_printf("Net engine register NET_ENGINE_RECEIVE_INTR failed\n");
    }

    Status = NET_ENGINE_register_intr(&net_engine, NET_ENGINE_ROW_COMPLETE_INTR, XPS_FPGA2_INT_ID);
    if(Status == NET_ENGINE_OK){
        xil_printf("Net engine register NET_ENGINE_ROW_COMPLETE_INTR failed\n");
    }

    XTime time_Global;
    XTime_GetTime(&time_Global);
    xil_printf("Before Time - %d\n", time_Global);
    Status = NET_ENGINE_process_maxpooling(&net_engine, imageData, out_buffer);
    XTime_GetTime(&time_Global);
    xil_printf("After Time - %d\n", time_Global);

    // NET_ENGINE_CNN_layer_Process(&net_engine, imageData, out_buffer)
}





