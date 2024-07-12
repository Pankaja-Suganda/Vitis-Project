

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

#define NET_ENGINE_INPUT_ROW_LENGTH   100
#define NET_ENGINE_OUTPUT_ROW_LENGTH  97

#define NET_ENGINE_TOTAL_DMA_SEND_LENGTH     (NET_ENGINE_INPUT_ROW_LENGTH * NET_ENGINE_INPUT_ROW_LENGTH * 4)
#define NET_ENGINE_TOTAL_DMA_RECEIVE_LENGTH  (NET_ENGINE_OUTPUT_ROW_LENGTH * NET_ENGINE_OUTPUT_ROW_LENGTH * 4)
#define NET_ENGINE_INITIAL_SEND_LENGTH       (NET_ENGINE_INPUT_ROW_LENGTH * 4 * 3)
#define NET_ENGINE_SEND_LENGTH               (NET_ENGINE_INPUT_ROW_LENGTH * 4)


#define REG_DUMP(reg, value) xil_printf("\tReg %s - %08X \r\n", #reg, value )


/************************** Function Definitions ***************************/
static void row_completed_ISR(void *CallBackRef){
	static int i=4;
	int status;
    Net_Engine_Inst *instance;

    instance = (Net_Engine_Inst*) CallBackRef;

    // xil_printf("imageProcISR\n");
    xil_printf("\tReg Status -  %04x\n", instance->net_engine_regs->Status_3);
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

static void NET_ENGINE_dump_regs(Net_Engine_Inst *instance){

	xil_printf("******************************\n\r");
	xil_printf("*  Net Engine Register Dump  *\n\r");
	xil_printf("******************************\n\n\r");

	xil_printf("Register File :\r\n");
 
    REG_DUMP(NET_ENGINE_STATUS_REG_1, instance->net_engine_regs->Status_1);
    REG_DUMP(NET_ENGINE_STATUS_REG_2, instance->net_engine_regs->Status_2);
    REG_DUMP(NET_ENGINE_STATUS_REG_3, instance->net_engine_regs->Status_3);
    REG_DUMP(NET_ENGINE_STATUS_REG_4, instance->net_engine_regs->Status_4);
    REG_DUMP(NET_ENGINE_STATUS_REG_5, instance->net_engine_regs->Status_5);
    REG_DUMP(NET_ENGINE_STATUS_REG_6, instance->net_engine_regs->Status_6);
    REG_DUMP(NET_ENGINE_CONFIG_REG_1, instance->net_engine_regs->Config_1);
    REG_DUMP(NET_ENGINE_CONFIG_REG_2, instance->net_engine_regs->Config_2);
    REG_DUMP(NET_ENGINE_CONFIG_REG_3, instance->net_engine_regs->Config_3);
    REG_DUMP(NET_ENGINE_CONFIG_REG_4, instance->net_engine_regs->Config_4);
    REG_DUMP(NET_ENGINE_KERNAL_REG_1, instance->net_engine_regs->Kernal_1);
    REG_DUMP(NET_ENGINE_KERNAL_REG_2, instance->net_engine_regs->Kernal_2);
    REG_DUMP(NET_ENGINE_KERNAL_REG_3, instance->net_engine_regs->Kernal_3);
    REG_DUMP(NET_ENGINE_KERNAL_REG_4, instance->net_engine_regs->Kernal_4);
    REG_DUMP(NET_ENGINE_KERNAL_REG_5, instance->net_engine_regs->Kernal_5);
    REG_DUMP(NET_ENGINE_KERNAL_REG_6, instance->net_engine_regs->Kernal_6);
    REG_DUMP(NET_ENGINE_KERNAL_REG_7, instance->net_engine_regs->Kernal_7);
    REG_DUMP(NET_ENGINE_KERNAL_REG_8, instance->net_engine_regs->Kernal_8);
    REG_DUMP(NET_ENGINE_KERNAL_REG_9, instance->net_engine_regs->Kernal_9);
    REG_DUMP(NET_ENGINE_BIAS_REG,     instance->net_engine_regs->Bias);
}


NET_STATUS NET_ENGINE_init(Net_Engine_Inst *instance, UINTPTR baseaddr_p, UINTPTR dmaaddr_p){
    int ret = NET_ENGINE_FAIL;
    Net_Engine* net_reg = (Net_Engine*) baseaddr_p;

    instance->id               = 1;
    instance->config.RegBase   = baseaddr_p;
    instance->net_engine_regs  = net_reg;

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

    NET_ENGINE_dump_regs(instance);
    
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
        NET_ENGINE_mWriteReg(instance->config.RegBase, NET_ENGINE_S00_AXI_SLV_REG6_OFFSET, NET_ENGINE_CONFIG_CNN_VALUE);
    }
    else if(config == NET_CONFIG_MAXPOOLING){
        instance->config.config = NET_CONFIG_MAXPOOLING;
        NET_ENGINE_mWriteReg(instance->config.RegBase, NET_ENGINE_S00_AXI_SLV_REG6_OFFSET, NET_ENGINE_CONFIG_MAXPOOL_VALUE);
    }
    else{
        xil_printf("Net engine config error");
    }

    return NET_ENGINE_OK;
}

static NET_STATUS NET_ENGINE_process(Net_Engine_Inst *instance, Net_Engine_Img *input, Net_Engine_Img *output){
    NET_STATUS ret = NET_ENGINE_OK;

    instance->cur_data.input  = input;
    instance->cur_data.output = output;
    instance->cur_data.state  = NET_STATE_BUSY;
    instance->cur_data.received_row_count = 0;
    instance->cur_data.send_row_count = 0;

    Xil_DCacheFlushRange((UINTPTR)input, 97*100*4);
    Xil_DCacheFlushRange((UINTPTR)output, 97*97 * 4);

    NET_ENGINE_dump_regs(instance);

	ret = XAxiDma_SimpleTransfer(&(instance->dma_inst), (u32)output, NET_ENGINE_TOTAL_DMA_RECEIVE_LENGTH, XAXIDMA_DEVICE_TO_DMA);
	if(ret != XST_SUCCESS){
		xil_printf("DMA initialization failed\n");
		return NET_ENGINE_FAIL;
	}

	ret = XAxiDma_SimpleTransfer(&(instance->dma_inst), (u32)input,  NET_ENGINE_INITIAL_SEND_LENGTH, XAXIDMA_DMA_TO_DEVICE);
	if(ret != XST_SUCCESS){
		xil_printf("DMA initialization failed\n");
		return NET_ENGINE_FAIL;
	}

    while(instance->cur_data.state != NET_STATE_COMPLETED){}

    xil_printf("Completed \r\nOut : \n");
    Xil_DCacheInvalidateRange((UINTPTR)output, 97 * 97 * 4);


    NET_ENGINE_dump_regs(instance);

    return NET_ENGINE_OK;
}

NET_STATUS NET_ENGINE_process_maxpooling(Net_Engine_Inst *instance, Net_Engine_Img *input, Net_Engine_Img *output){
    NET_STATUS ret = NET_ENGINE_OK;

    ret = NET_ENGINE_config(instance, NET_CONFIG_MAXPOOLING);
	if(ret != XST_SUCCESS){
		xil_printf("Net Engine Config failed\n");
		return NET_ENGINE_FAIL;
	}

    ret = NET_ENGINE_process(instance, input, output);
    if(ret != XST_SUCCESS){
		xil_printf("Net Engine Process failed\n");
		return NET_ENGINE_FAIL;
	}

    return ret;
}


static NET_STATUS NET_ENGINE_set_cnn_values(Net_Engine_Inst *instance, u32 *kernal, u32 bias){

    NET_ENGINE_mWriteReg(instance->config.RegBase, NET_ENGINE_KERNAL_REG_1, kernal[0]);
    NET_ENGINE_mWriteReg(instance->config.RegBase, NET_ENGINE_KERNAL_REG_2, kernal[1]);
    NET_ENGINE_mWriteReg(instance->config.RegBase, NET_ENGINE_KERNAL_REG_3, kernal[2]);

    NET_ENGINE_mWriteReg(instance->config.RegBase, NET_ENGINE_KERNAL_REG_4, kernal[3]);
    NET_ENGINE_mWriteReg(instance->config.RegBase, NET_ENGINE_KERNAL_REG_5, kernal[4]);
    NET_ENGINE_mWriteReg(instance->config.RegBase, NET_ENGINE_KERNAL_REG_6, kernal[5]);

    NET_ENGINE_mWriteReg(instance->config.RegBase, NET_ENGINE_KERNAL_REG_7, kernal[6]);
    NET_ENGINE_mWriteReg(instance->config.RegBase, NET_ENGINE_KERNAL_REG_8, kernal[7]);
    NET_ENGINE_mWriteReg(instance->config.RegBase, NET_ENGINE_KERNAL_REG_9, kernal[8]);

    return NET_ENGINE_OK;

}



NET_STATUS NET_ENGINE_process_cnn(Net_Engine_Inst *instance, Net_Engine_Img *input, Net_Engine_Img *output, CNN_Config_Data data){
    NET_STATUS ret = NET_ENGINE_OK;

    ret = NET_ENGINE_config(instance, NET_CONFIG_CNN);
	if(ret != XST_SUCCESS){
		xil_printf("Net Engine Config failed\n");
		return NET_ENGINE_FAIL;
	}

    // ret = NET_ENGINE_set_cnn_values(instance, kernal, bias);
	// if(ret != XST_SUCCESS){
	// 	xil_printf("Net Engine value setting failed\n");
	// 	return NET_ENGINE_FAIL;
	// }

    ret = NET_ENGINE_process(instance, input, output);
    if(ret != XST_SUCCESS){
		xil_printf("Net Engine Process failed\n");
		return NET_ENGINE_FAIL;
	}

    return ret;
}

