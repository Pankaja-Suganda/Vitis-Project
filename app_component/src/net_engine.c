

/***************************** Include Files *******************************/
#include "net_engine.h"
#include "net_engine_hw.h"
#include "xaxidma.h"
#include "xil_cache.h"
#include "xil_io.h"
#include <stdint.h>
#include <xil_printf.h>

/***************************** Defines   *******************************/
#define RESET_TIMEOUT_COUNTER           10000

#define NET_ENGINE_CONFIG_CNN_VALUE     0xffffffff
#define NET_ENGINE_CONFIG_MAXPOOL_VALUE 0x00000000

/************************** Function Definitions ***************************/
static void row_completed_ISR(void *CallBackRef){
	static int i=4;
	int status;
    Net_Engine_Inst *instance;

    instance = (Net_Engine_Inst*) CallBackRef;

    xil_printf("imageProcISR\n");
    xil_printf("\tReg Status -  %04x\n", Xil_In32(instance->config.RegBase + (2*4)));
	XScuGic_Disable(&(instance->intc_inst), instance->config.row_complete_isr_id);
	// status = checkIdle(XPAR_AXI_DMA_0_BASEADDR,0x4);
	// while(status == 0)
	// 	status = checkIdle(XPAR_AXI_DMA_0_BASEADDR,0x4);
	if(i<514){
        // status = XAxiDma_SimpleTransfer((XAxiDma *)CallBackRef,(u32)out_buffer, 97*4,XAXIDMA_DEVICE_TO_DMA);
		status = XAxiDma_SimpleTransfer(&(instance->dma_inst),(u32)instance->cur_data.input,100 * 4,XAXIDMA_DMA_TO_DEVICE);
		i++;
	}
	XScuGic_Enable(&(instance->intc_inst), instance->config.row_complete_isr_id);
}


static void received_ISR(void *CallBackRef){
    Net_Engine_Inst *instance;
    instance = (Net_Engine_Inst*) CallBackRef;

	XAxiDma_IntrDisable(&(instance->dma_inst), XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrAckIrq(&(instance->dma_inst), XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);
	instance->cur_data.state = NET_STATE_COMPLETED;
    xil_printf("dmaReceiveISR\n");
	XAxiDma_IntrEnable(&(instance->dma_inst), XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);
}

NET_STATUS NET_ENGINE_dma_setup(Net_Engine_Inst *instance, UINTPTR dmaaddr_p){
    int ret = NET_ENGINE_OK;

    XAxiDma_Config* dma_config;

    dma_config = (XAxiDma_Config*)XAxiDma_LookupConfig(dmaaddr_p);
    if(dma_config == NULL){
        xil_printf("DMA config not found (%08x)", dmaaddr_p);
    }

	ret = XAxiDma_CfgInitialize(&instance->dma_inst, dma_config);
	if(ret != XST_SUCCESS){
		xil_printf("DMA initialization failed\n");
	}

    XAxiDma_IntrEnable(&instance->dma_inst, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);
    XAxiDma_IntrEnable(&instance->dma_inst, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DMA_TO_DEVICE);

    return ret;
}

NET_STATUS NET_ENGINE_intr_setup(Net_Engine_Inst *instance, UINTPTR intraddr_p){
    int ret = NET_ENGINE_FAIL;

    XScuGic_Config* gic_config;

    gic_config = XScuGic_LookupConfig(intraddr_p);
    if(gic_config == NULL){
        xil_printf("Interrupt (GIC) config not found (%08x)", intraddr_p);
    }

	ret = XScuGic_CfgInitialize(&instance->intc_inst, gic_config, gic_config->CpuBaseAddress);
	if(ret != XST_SUCCESS){
		xil_printf("DMA initialization failed\n");
	}

    return ret;
}

NET_STATUS NET_ENGINE_init(Net_Engine_Inst *instance, UINTPTR baseaddr_p, UINTPTR dmaaddr_p){
    int ret = NET_ENGINE_FAIL;

    instance->id                    = 1;
    instance->config.RegBase        = baseaddr_p;

    ret = NET_ENGINE_dma_setup(instance, dmaaddr_p);
	if(ret != XST_SUCCESS){
		xil_printf("DMA setup failed\n");
		return NET_ENGINE_FAIL;
	}
    
    return ret;
}


NET_STATUS NET_ENGINE_reset(Net_Engine_Inst *instance){

    XAxiDma_Reset(&(instance->dma_inst));

    int  TimeOut = RESET_TIMEOUT_COUNTER;

    while (TimeOut) {
        if (XAxiDma_ResetIsDone(&(instance->dma_inst))) {
            return NET_ENGINE_OK;
        }

        TimeOut -= 1;
    }
    return NET_ENGINE_FAIL;
}

NET_STATUS NET_ENGINE_register_intr(Net_Engine_Inst *instance, Net_Engine_Intr intr_type, int32_t intr_pin){
    int ret = NET_ENGINE_OK;
    
	XScuGic_SetPriorityTriggerType(&(instance->intc_inst), intr_pin, 0xA0 ,3);

    if(intr_type == NET_ENGINE_ROW_COMPLETE_INTR){
        ret = XScuGic_Connect(&(instance->intc_inst),  intr_pin,(Xil_InterruptHandler)row_completed_ISR,(void *)instance);
    }
    else if(intr_type == NET_ENGINE_RECEIVE_INTR){
        ret = XScuGic_Connect(&(instance->intc_inst),  intr_pin,(Xil_InterruptHandler)received_ISR,(void *)instance);
    }
    else{
        ret = NET_ENGINE_FAIL;
    }

	if(ret != NET_ENGINE_OK){
		xil_printf("Interrupt connection failed");
		ret = NET_ENGINE_FAIL;
	}

    if(intr_type == NET_ENGINE_ROW_COMPLETE_INTR){
        instance->config.row_complete_isr_id = intr_pin;
    }
    else if(intr_type == NET_ENGINE_RECEIVE_INTR){
        instance->config.receive_isr_id = intr_pin;
    }

	XScuGic_Enable(&(instance->intc_inst),intr_pin);

    Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,(Xil_ExceptionHandler)XScuGic_InterruptHandler,(void *)&(instance->intc_inst));
	Xil_ExceptionEnable();

    return ret;
}


NET_STATUS NET_ENGINE_config(Net_Engine_Inst *instance, NET_CONFIG config){
    if(config == NET_CONFIG_CNN){
        instance->config.config = NET_CONFIG_CNN;
        NET_ENGINE_mWriteReg(instance->config.RegBase, NET_ENGINE_S00_AXI_SLV_REG5_OFFSET, NET_ENGINE_CONFIG_CNN_VALUE);
    }
    else if(config == NET_CONFIG_MAXPOOLING){
        instance->config.config = NET_CONFIG_MAXPOOLING;
        NET_ENGINE_mWriteReg(instance->config.RegBase, NET_ENGINE_S00_AXI_SLV_REG5_OFFSET, NET_ENGINE_CONFIG_MAXPOOL_VALUE);
    }
    else{
        xil_printf("Net engine config error");
    }

    return NET_ENGINE_OK;
}

NET_STATUS NET_ENGINE_process_maxpooling(Net_Engine_Inst *instance, Net_Engine_Img *input, Net_Engine_Img *output){
    NET_STATUS ret = NET_ENGINE_OK;

    ret = NET_ENGINE_config(instance, NET_CONFIG_MAXPOOLING);

    instance->cur_data.input  = input;
    instance->cur_data.output = output;
    instance->cur_data.state  = NET_STATE_IDLE;
    instance->cur_data.received_row_count = 0;
    instance->cur_data.send_row_count = 0;

    Xil_DCacheFlushRange((UINTPTR)input, 97*100*4);

	ret = XAxiDma_SimpleTransfer(&(instance->dma_inst), (u32)output, 97*40*4,XAXIDMA_DEVICE_TO_DMA);
	if(ret != XST_SUCCESS){
		xil_printf("DMA initialization failed\n");
		return NET_ENGINE_FAIL;
	}

	ret = XAxiDma_SimpleTransfer(&(instance->dma_inst), (u32)input, 4*100*4,XAXIDMA_DMA_TO_DEVICE);
	if(ret != XST_SUCCESS){
		xil_printf("DMA initialization failed\n");
		return NET_ENGINE_FAIL;
	}
    int i = 0;
    while(instance->cur_data.state != NET_STATE_COMPLETED){
        xil_printf("I - %d \r\n", i);
        i++;
    }

    xil_printf("Completed \r\n");

    return ret;
}
