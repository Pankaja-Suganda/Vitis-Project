

/***************************** Include Files *******************************/
#include "net_engine.h"
#include "net_engine_hw.h"
#include "xaxidma.h"
#include "xil_cache.h"
#include "xil_io.h"
#include <stdint.h>
#include <xil_printf.h>
#include <xil_types.h>

/***************************** Defines   *******************************/
#define RESET_TIMEOUT_COUNTER           10000

#define NET_ENGINE_CONFIG_CNN_VALUE     0xffffffff
#define NET_ENGINE_CONFIG_MAXPOOL_VALUE 0x00000000

#define NET_ENGINE_ENABLE_VALUE         0xffffffff
#define NET_ENGINE_DISABLE_VALUE        0x00000000

#define NET_ENGINE_INPUT_ROW_LENGTH     100
#define NET_ENGINE_OUTPUT_ROW_LENGTH    98

#define NET_ENGINE_TOTAL_DMA_SEND_LENGTH(x)     ((x+2) * (x+2) * 4)
#define NET_ENGINE_TOTAL_DMA_RECEIVE_LENGTH(x)  (x * x * 4)
#define NET_ENGINE_INITIAL_SEND_LENGTH(x)       ((x+2) * 4 * 3)
#define NET_ENGINE_SEND_LENGTH(x)               ((x+2) * 4)

#define DCACHE_FLUSH_INPUT_LENGTH(x)            ((x+2) *(x+2) * 4)
#define DCACHE_FLUSH_OUTPUT_LENGTH(x)           (x     *   x  * 4)

#define REG_DUMP(reg, value) xil_printf("\tReg %s - %08X \r\n", #reg, value )



static u32* dma_input_ptr;
static u32  global_row_length;

/************************** Function Definitions ***************************/
u32 checkIdle(u32 baseAddress,u32 offset){
	u32 status;
	status = (XAxiDma_ReadReg(baseAddress,offset))&XAXIDMA_IDLE_MASK;
	return status;
}

int count = 0;
int img_received = 1;

static void row_completed_ISR(void *CallBackRef){
	u32 IrqStatus;
	int status;
    Net_Engine_Inst *instance;
    instance = (Net_Engine_Inst*) CallBackRef;

    u32* temp = dma_input_ptr;

    // xil_printf("%d image pointer %p \r\n", count, temp);

    // status = checkIdle(instance->dma_inst.RegBase,0x4);
	// while(status == 0){
	// 	status = checkIdle(instance->dma_inst.RegBase,0x4);
    //     xil_printf("\timageProcISR status %d\n", status);
    // }

    instance = (Net_Engine_Inst*) CallBackRef;

    count++;

    // xil_printf("imageProcISR %08x, %d\n", instance->cur_data.input, count);
    // xil_printf("\tReg Status -  %04x\n", instance->net_engine_regs->Status_3);
	XScuGic_Disable(&(instance->intc_inst), instance->config.row_complete_isr_id);
    if(img_received){
        status = XAxiDma_SimpleTransfer(&(instance->dma_inst), dma_input_ptr + 6 , NET_ENGINE_SEND_LENGTH(global_row_length), XAXIDMA_DMA_TO_DEVICE);
        dma_input_ptr = dma_input_ptr + (global_row_length + 2);
	}
	XScuGic_Enable(&(instance->intc_inst), instance->config.row_complete_isr_id);
}


void check_dma_status(XAxiDma *dma_inst);
static void received_ISR(void *CallBackRef){
    // u32 IrqStatus;
    Net_Engine_Inst *instance;
    instance = (Net_Engine_Inst*) CallBackRef;

	// XAxiDma_IntrDisable(&(instance->dma_inst), XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
	// XAxiDma_IntrAckIrq(&(instance->dma_inst), XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);

    // IrqStatus = XAxiDma_IntrGetIrq(&(instance->dma_inst), XAXIDMA_DEVICE_TO_DMA);
    // XAxiDma_IntrAckIrq(&(instance->dma_inst), IrqStatus, XAXIDMA_DEVICE_TO_DMA);
    // check_dma_status(&(instance->dma_inst));
    // xil_printf("RxIntrHandle 1 %04X \r\n", IrqStatus);
    // XScuGic_Disable(&(instance->intc_inst), instance->config.receive_isr_id);
	/* Acknowledge pending interrupts */
	instance->cur_data.state = NET_STATE_COMPLETED;
    img_received = 0;
    // xil_printf("dmaReceiveISR\n");


    // XScuGic_Enable(&(instance->intc_inst), instance->config.receive_isr_id);
	// XAxiDma_IntrEnable(&(instance->dma_inst), XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
    // XScuGic_Enable(&(instance->intc_inst), instance->config.receive_isr_id);
    // IrqStatus = XAxiDma_IntrGetIrq(&(instance->dma_inst), XAXIDMA_DEVICE_TO_DMA);
    // xil_printf("RxIntrHandle 2 %04X \r\n", IrqStatus);
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
    // XAxiDma_IntrEnable(&instance->dma_inst, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DMA_TO_DEVICE);

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
    REG_DUMP(NET_ENGINE_STATUS_REG_1, NET_ENGINE_mReadReg(instance->config.RegBase, NET_ENGINE_STATUS_REG_1));
    REG_DUMP(NET_ENGINE_STATUS_REG_2, NET_ENGINE_mReadReg(instance->config.RegBase, NET_ENGINE_STATUS_REG_2));
    REG_DUMP(NET_ENGINE_STATUS_REG_3, NET_ENGINE_mReadReg(instance->config.RegBase, NET_ENGINE_STATUS_REG_3));
    REG_DUMP(NET_ENGINE_STATUS_REG_4, NET_ENGINE_mReadReg(instance->config.RegBase, NET_ENGINE_STATUS_REG_4));
    REG_DUMP(NET_ENGINE_STATUS_REG_5, NET_ENGINE_mReadReg(instance->config.RegBase, NET_ENGINE_STATUS_REG_5));
    REG_DUMP(NET_ENGINE_STATUS_REG_6, NET_ENGINE_mReadReg(instance->config.RegBase, NET_ENGINE_STATUS_REG_6));
    REG_DUMP(NET_ENGINE_CONFIG_REG_1, NET_ENGINE_mReadReg(instance->config.RegBase, NET_ENGINE_CONFIG_REG_1));
    REG_DUMP(NET_ENGINE_CONFIG_REG_2, NET_ENGINE_mReadReg(instance->config.RegBase, NET_ENGINE_CONFIG_REG_2));
    REG_DUMP(NET_ENGINE_CONFIG_REG_3, NET_ENGINE_mReadReg(instance->config.RegBase, NET_ENGINE_CONFIG_REG_3));
    REG_DUMP(NET_ENGINE_CONFIG_REG_4, NET_ENGINE_mReadReg(instance->config.RegBase, NET_ENGINE_CONFIG_REG_4));
    REG_DUMP(NET_ENGINE_KERNAL_REG_1, NET_ENGINE_mReadReg(instance->config.RegBase, NET_ENGINE_KERNAL_REG_1));
    REG_DUMP(NET_ENGINE_KERNAL_REG_2, NET_ENGINE_mReadReg(instance->config.RegBase, NET_ENGINE_KERNAL_REG_2));
    REG_DUMP(NET_ENGINE_KERNAL_REG_3, NET_ENGINE_mReadReg(instance->config.RegBase, NET_ENGINE_KERNAL_REG_3));
    REG_DUMP(NET_ENGINE_KERNAL_REG_4, NET_ENGINE_mReadReg(instance->config.RegBase, NET_ENGINE_KERNAL_REG_4));
    REG_DUMP(NET_ENGINE_KERNAL_REG_5, NET_ENGINE_mReadReg(instance->config.RegBase, NET_ENGINE_KERNAL_REG_5));
    REG_DUMP(NET_ENGINE_KERNAL_REG_6, NET_ENGINE_mReadReg(instance->config.RegBase, NET_ENGINE_KERNAL_REG_6));
    REG_DUMP(NET_ENGINE_KERNAL_REG_7, NET_ENGINE_mReadReg(instance->config.RegBase, NET_ENGINE_KERNAL_REG_7));
    REG_DUMP(NET_ENGINE_KERNAL_REG_8, NET_ENGINE_mReadReg(instance->config.RegBase, NET_ENGINE_KERNAL_REG_8));
    REG_DUMP(NET_ENGINE_KERNAL_REG_9, NET_ENGINE_mReadReg(instance->config.RegBase, NET_ENGINE_KERNAL_REG_9));
    REG_DUMP(NET_ENGINE_BIAS_REG,     NET_ENGINE_mReadReg(instance->config.RegBase, NET_ENGINE_BIAS_REG));

    // REG_DUMP(NET_ENGINE_STATUS_REG_1, instance->net_engine_regs->Status_1);
    // REG_DUMP(NET_ENGINE_STATUS_REG_2, instance->net_engine_regs->Status_2);
    // REG_DUMP(NET_ENGINE_STATUS_REG_3, instance->net_engine_regs->Status_3);
    // REG_DUMP(NET_ENGINE_STATUS_REG_4, instance->net_engine_regs->Status_4);
    // REG_DUMP(NET_ENGINE_STATUS_REG_5, instance->net_engine_regs->Status_5);
    // REG_DUMP(NET_ENGINE_STATUS_REG_6, instance->net_engine_regs->Status_6);
    // REG_DUMP(NET_ENGINE_CONFIG_REG_1, instance->net_engine_regs->Config_1);
    // REG_DUMP(NET_ENGINE_CONFIG_REG_2, instance->net_engine_regs->Config_2);
    // REG_DUMP(NET_ENGINE_CONFIG_REG_3, instance->net_engine_regs->Config_3);
    // REG_DUMP(NET_ENGINE_CONFIG_REG_4, instance->net_engine_regs->Config_4);
    // REG_DUMP(NET_ENGINE_KERNAL_REG_1, instance->net_engine_regs->Kernal_1);
    // REG_DUMP(NET_ENGINE_KERNAL_REG_2, instance->net_engine_regs->Kernal_2);
    // REG_DUMP(NET_ENGINE_KERNAL_REG_3, instance->net_engine_regs->Kernal_3);
    // REG_DUMP(NET_ENGINE_KERNAL_REG_4, instance->net_engine_regs->Kernal_4);
    // REG_DUMP(NET_ENGINE_KERNAL_REG_5, instance->net_engine_regs->Kernal_5);
    // REG_DUMP(NET_ENGINE_KERNAL_REG_6, instance->net_engine_regs->Kernal_6);
    // REG_DUMP(NET_ENGINE_KERNAL_REG_7, instance->net_engine_regs->Kernal_7);
    // REG_DUMP(NET_ENGINE_KERNAL_REG_8, instance->net_engine_regs->Kernal_8);
    // REG_DUMP(NET_ENGINE_KERNAL_REG_9, instance->net_engine_regs->Kernal_9);
    // REG_DUMP(NET_ENGINE_BIAS_REG,     instance->net_engine_regs->Bias);
}




NET_STATUS NET_ENGINE_init(Net_Engine_Inst *instance, UINTPTR baseaddr_p, UINTPTR dmaaddr_p){
    int ret = NET_ENGINE_FAIL;
    Net_Engine* net_reg = (Net_Engine*) baseaddr_p;

    instance->id               = 1;
    instance->config.RegBase   = baseaddr_p;
    instance->net_engine_regs  = net_reg;

    NET_ENGINE_mWriteReg(instance->config.RegBase, NET_ENGINE_S00_AXI_SLV_REG7_OFFSET, NET_ENGINE_INPUT_ROW_LENGTH);
    NET_ENGINE_mWriteReg(instance->config.RegBase, NET_ENGINE_S00_AXI_SLV_REG8_OFFSET, NET_ENGINE_ENABLE_VALUE);

    // NET_ENGINE_dump_regs(instance);

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

    // NET_ENGINE_dump_regs(instance);
    
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

void check_dma_status(XAxiDma *dma_inst) {
    u32 dma_status;

    dma_status = XAxiDma_ReadReg(dma_inst->RegBase + XAXIDMA_RX_OFFSET, XAXIDMA_SR_OFFSET);
    xil_printf("DMA Status: %08X\n", dma_status);

    if (dma_status & XAXIDMA_ERR_INTERNAL_MASK) xil_printf("DMA Internal Error\n");
    if (dma_status & XAXIDMA_ERR_SG_SLV_MASK) xil_printf("DMA Slave Error\n");
    if (dma_status & XAXIDMA_ERR_SG_DEC_MASK) xil_printf("DMA Decode Error\n");
    if (dma_status & XAXIDMA_ERR_SG_INT_MASK) xil_printf("DMA SG Internal Error\n");
    if (dma_status & XAXIDMA_HALTED_MASK) xil_printf("DMA Halted\n");
    if (dma_status & XAXIDMA_ERR_SLAVE_MASK) xil_printf("DMA Slave Mask Error\n");
    if (dma_status & XAXIDMA_ERR_DECODE_MASK) xil_printf("DMA Decode Mask Error\n");

    if (dma_status & XAXIDMA_ERR_ALL_MASK) {
        xil_printf("DMA Error\n");
        // if (dma_status & XAXIDMA_ERR_INTERNAL_MASK) xil_printf("DMA Internal Error\n");
        // if (dma_status & XAXIDMA_ERR_SG_SLV_FLT_MASK) xil_printf("DMA Slave Error\n");
        // if (dma_status & XAXIDMA_ERR_SG_DECERR_MASK) xil_printf("DMA Decode Error\n");
        // if (dma_status & XAXIDMA_ERR_SG_INTERR_MASK) xil_printf("DMA SG Internal Error\n");

        // Clear the error interrupt
        XAxiDma_IntrAckIrq(dma_inst, XAXIDMA_ERR_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
        NET_ENGINE_reset(dma_inst);
    }
}

void DumpDmaRegisters(XAxiDma *AxiDmaInst) {
    u32 regValue;
    u32 baseAddr = AxiDmaInst->RegBase;

    xil_printf("Dumping DMA registers:\n");

    regValue = XAxiDma_ReadReg(baseAddr, XAXIDMA_CR_OFFSET);
    xil_printf("Control Reg: 0x%08x\n", regValue);

    regValue = XAxiDma_ReadReg(baseAddr, XAXIDMA_SR_OFFSET);
    xil_printf("Status Reg: 0x%08x\n", regValue);

    regValue = XAxiDma_ReadReg(baseAddr, XAXIDMA_CDESC_OFFSET);
    xil_printf("Current Desc Reg: 0x%08x\n", regValue);

    regValue = XAxiDma_ReadReg(baseAddr, XAXIDMA_TDESC_OFFSET);
    xil_printf("Tail Desc Reg: 0x%08x\n", regValue);

    // regValue = XAxiDma_ReadReg(baseAddr, XAXIDMA_BTT_OFFSET);
    // xil_printf("Bytes To Transfer Reg: 0x%08x\n", regValue);

    regValue = XAxiDma_ReadReg(baseAddr, XAXIDMA_CR_OFFSET + XAXIDMA_TX_OFFSET);
    xil_printf("MM2S Control Reg: 0x%08x\n", regValue);

    regValue = XAxiDma_ReadReg(baseAddr, XAXIDMA_SR_OFFSET + XAXIDMA_TX_OFFSET);
    xil_printf("MM2S Status Reg: 0x%08x\n", regValue);

    regValue = XAxiDma_ReadReg(baseAddr, XAXIDMA_CDESC_OFFSET + XAXIDMA_TX_OFFSET);
    xil_printf("MM2S Current Desc Reg: 0x%08x\n", regValue);

    regValue = XAxiDma_ReadReg(baseAddr, XAXIDMA_TDESC_OFFSET + XAXIDMA_TX_OFFSET);
    xil_printf("MM2S Tail Desc Reg: 0x%08x\n", regValue);

    // regValue = XAxiDma_ReadReg(baseAddr, XAXIDMA_BTT_OFFSET + XAXIDMA_TX_OFFSET);
    // xil_printf("MM2S Bytes To Transfer Reg: 0x%08x\n", regValue);

    regValue = XAxiDma_ReadReg(baseAddr, XAXIDMA_CR_OFFSET + XAXIDMA_RX_OFFSET);
    xil_printf("S2MM Control Reg: 0x%08x\n", regValue);

    regValue = XAxiDma_ReadReg(baseAddr, XAXIDMA_SR_OFFSET + XAXIDMA_RX_OFFSET);
    xil_printf("S2MM Status Reg: 0x%08x\n", regValue);

    regValue = XAxiDma_ReadReg(baseAddr, XAXIDMA_CDESC_OFFSET + XAXIDMA_RX_OFFSET);
    xil_printf("S2MM Current Desc Reg: 0x%08x\n", regValue);

    regValue = XAxiDma_ReadReg(baseAddr, XAXIDMA_TDESC_OFFSET + XAXIDMA_RX_OFFSET);
    xil_printf("S2MM Tail Desc Reg: 0x%08x\n", regValue);

    // regValue = XAxiDma_ReadReg(baseAddr, XAXIDMA_BTT_OFFSET + XAXIDMA_RX_OFFSET);
    // xil_printf("S2MM Bytes To Transfer Reg: 0x%08x\n", regValue);
}

NET_STATUS NET_ENGINE_config_row_length(Net_Engine_Inst *instance, u32 row_length ){
    NET_ENGINE_mWriteReg(instance->config.RegBase, NET_ENGINE_S00_AXI_SLV_REG7_OFFSET, row_length);
    return NET_ENGINE_OK;
}



static NET_STATUS NET_ENGINE_process(Net_Engine_Inst *instance, u32 *input, u32 *output, u32 row_length){
    NET_STATUS ret = NET_ENGINE_OK;

    instance->cur_data.input  = NULL;
    instance->cur_data.output = NULL;

    instance->cur_data.input  = input;
    instance->cur_data.output = output;
    instance->cur_data.state  = NET_STATE_BUSY;
    instance->cur_data.received_row_count = 0;
    instance->cur_data.send_row_count     = 0;

    global_row_length = row_length;
    // dma_input_ptr     = input  + (NET_ENGINE_INPUT_ROW_LENGTH * 3);
    dma_input_ptr     = input  + (global_row_length * 3);
    
    // xil_printf("%d image pointer %p \r\n", count, input);
    count = 3;

    NET_ENGINE_mWriteReg(instance->config.RegBase, NET_ENGINE_S00_AXI_SLV_REG8_OFFSET, NET_ENGINE_ENABLE_VALUE);

    Xil_DCacheFlushRange((UINTPTR)input,  DCACHE_FLUSH_INPUT_LENGTH(global_row_length));
    Xil_DCacheFlushRange((UINTPTR)output, DCACHE_FLUSH_OUTPUT_LENGTH(global_row_length));

    // instance->cur_data.input = instance->cur_data.input + (NET_ENGINE_INPUT_ROW_LENGTH * 3);
    instance->cur_data.input = instance->cur_data.input + (global_row_length * 3);

    // printf("output %08x, input %08x \r\n", output, input);
    

    // NET_ENGINE_dump_regs(instance);
    img_received = 1;
	ret = XAxiDma_SimpleTransfer(&(instance->dma_inst), (u32)output, NET_ENGINE_TOTAL_DMA_RECEIVE_LENGTH(global_row_length), XAXIDMA_DEVICE_TO_DMA);
    // ret = XAxiDma_SimpleTransfer(&(instance->dma_inst), (u32)output, 97*97*4, XAXIDMA_DEVICE_TO_DMA);
	if(ret != XST_SUCCESS){
		xil_printf("DMA Receive Transfer failed %d\n", ret);
		return NET_ENGINE_FAIL;
	}

	ret = XAxiDma_SimpleTransfer(&(instance->dma_inst), (u32)input,  NET_ENGINE_INITIAL_SEND_LENGTH(global_row_length), XAXIDMA_DMA_TO_DEVICE);
    // ret = XAxiDma_SimpleTransfer(&(instance->dma_inst), (u32)input,  100 * 3 * 4, XAXIDMA_DMA_TO_DEVICE);
	if(ret != XST_SUCCESS){
		xil_printf("DMA Transmit Transfer failed %d\n", ret);
		return NET_ENGINE_FAIL;
	}

    int k = 0;
    
    // while(instance->cur_data.state != NET_STATE_COMPLETED){
    while (img_received){
        // check_dma_status(&(instance->dma_inst));
        k++;
        // if(k>10){
        //     // XAxiDma_Pause(&(instance->dma_inst));
        //     // XAxiDma_Resume(&(instance->dma_inst));
        //     DumpDmaRegisters(&(instance->dma_inst));
        //     xil_printf("Process Stopped\r\n");
        //     break;
        // }
    }

    // xil_printf("Completed \r\nOut : \n");
    Xil_DCacheInvalidateRange((UINTPTR)output, DCACHE_FLUSH_OUTPUT_LENGTH(global_row_length));

    NET_ENGINE_mWriteReg(instance->config.RegBase, NET_ENGINE_S00_AXI_SLV_REG8_OFFSET, NET_ENGINE_DISABLE_VALUE);
    // NET_ENGINE_dump_regs(instance);

    return NET_ENGINE_OK;
}

NET_STATUS NET_ENGINE_process_maxpooling(Net_Engine_Inst *instance, Net_Engine_Img *input, Net_Engine_Img *output){
    NET_STATUS ret = NET_ENGINE_OK;

    ret = NET_ENGINE_config(instance, NET_CONFIG_MAXPOOLING);
	if(ret != XST_SUCCESS){
		xil_printf("Net Engine Config failed\n");
		return NET_ENGINE_FAIL;
	}

    // ret = NET_ENGINE_process(instance, input, output);
    if(ret != XST_SUCCESS){
		xil_printf("Net Engine Process failed\n");
		return NET_ENGINE_FAIL;
	}

    return ret;
}


static NET_STATUS NET_ENGINE_set_cnn_values(Net_Engine_Inst *instance, CNN_Config_Data data){

    NET_ENGINE_mWriteReg(instance->config.RegBase, NET_ENGINE_KERNAL_REG_1, data.Kernal.Kernal_1);
    NET_ENGINE_mWriteReg(instance->config.RegBase, NET_ENGINE_KERNAL_REG_2, data.Kernal.Kernal_2);
    NET_ENGINE_mWriteReg(instance->config.RegBase, NET_ENGINE_KERNAL_REG_3, data.Kernal.Kernal_3);

    NET_ENGINE_mWriteReg(instance->config.RegBase, NET_ENGINE_KERNAL_REG_4, data.Kernal.Kernal_4);
    NET_ENGINE_mWriteReg(instance->config.RegBase, NET_ENGINE_KERNAL_REG_5, data.Kernal.Kernal_5);
    NET_ENGINE_mWriteReg(instance->config.RegBase, NET_ENGINE_KERNAL_REG_6, data.Kernal.Kernal_6);

    NET_ENGINE_mWriteReg(instance->config.RegBase, NET_ENGINE_KERNAL_REG_7, data.Kernal.Kernal_7);
    NET_ENGINE_mWriteReg(instance->config.RegBase, NET_ENGINE_KERNAL_REG_8, data.Kernal.Kernal_8);
    NET_ENGINE_mWriteReg(instance->config.RegBase, NET_ENGINE_KERNAL_REG_9, data.Kernal.Kernal_9);
    NET_ENGINE_mWriteReg(instance->config.RegBase, NET_ENGINE_BIAS_REG,     data.Bias);

    // xil_printf("Kernal Data K1(%08x) K2(%08x) K3(%08x) K4(%08x) K5(%08x) K6(%08x) K7(%08x) K8(%08x) K9(%08x) B(%08x)",
    //     data.Kernal.Kernal_1,
    //     data.Kernal.Kernal_2,
    //     data.Kernal.Kernal_3,
    //     data.Kernal.Kernal_4,
    //     data.Kernal.Kernal_5,
    //     data.Kernal.Kernal_6,
    //     data.Kernal.Kernal_7,
    //     data.Kernal.Kernal_8,
    //     data.Kernal.Kernal_9,
    //     data.Bias
    // );

    // NET_ENGINE_dump_regs(instance);

    return NET_ENGINE_OK;
}



NET_STATUS NET_ENGINE_process_cnn(Net_Engine_Inst *instance, u32 *input, u32 *output, CNN_Config_Data data, u32 row_length){
    NET_STATUS ret = NET_ENGINE_OK;

    ret = NET_ENGINE_config(instance, NET_CONFIG_CNN);
    // ret = NET_ENGINE_config(instance, NET_CONFIG_MAXPOOLING);
	if(ret != XST_SUCCESS){
		xil_printf("Net Engine Config failed\n");
		return NET_ENGINE_FAIL;
	}

    count = 0;

    // instance->net_engine_regs->Kernal_1 = data.Kernal.Kernal_1;
    // instance->net_engine_regs->Kernal_2 = data.Kernal.Kernal_2;
    // instance->net_engine_regs->Kernal_3 = data.Kernal.Kernal_3;
    // instance->net_engine_regs->Kernal_4 = data.Kernal.Kernal_4; 
    // instance->net_engine_regs->Kernal_5 = data.Kernal.Kernal_5;
    // instance->net_engine_regs->Kernal_6 = data.Kernal.Kernal_6; 
    // instance->net_engine_regs->Kernal_7 = data.Kernal.Kernal_7; 
    // instance->net_engine_regs->Kernal_8 = data.Kernal.Kernal_8;
    // instance->net_engine_regs->Kernal_9 = data.Kernal.Kernal_9;
    // instance->net_engine_regs->Bias     = data.Bias;

    ret = NET_ENGINE_set_cnn_values(instance, data);
	if(ret != XST_SUCCESS){
		xil_printf("Net Engine value setting failed\n");
		return NET_ENGINE_FAIL;
	}

    XAxiDma_IntrDisable(&(instance->dma_inst), XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrAckIrq(&(instance->dma_inst), XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);

    XAxiDma_IntrEnable(&(instance->dma_inst), XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);

    ret = NET_ENGINE_process(instance, input, output, row_length);
    
    if(ret != XST_SUCCESS){
		xil_printf("Net Engine Process failed\n");
		return NET_ENGINE_FAIL;
	}

    return ret;
}

