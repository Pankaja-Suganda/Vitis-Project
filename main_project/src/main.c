
#include "xparameters.h"
#include "sleep.h"
#include "xdebug.h"
#include "xil_util.h"
#include <xil_types.h>

#include "net_engine.h"
#include "xaxidma.h"
#include "xinterrupt_wrap.h"
#include "test_sample.h"

#ifdef XPAR_MEM0_BASEADDRESS
#define DDR_BASE_ADDR		XPAR_MEM0_BASEADDRESS
#endif

#ifndef DDR_BASE_ADDR
#warning CHECK FOR THE VALID DDR ADDRESS IN XPARAMETERS.H, \
DEFAULT SET TO 0x01000000
#define MEM_BASE_ADDR		0x01000000
#else
#define MEM_BASE_ADDR		(DDR_BASE_ADDR + 0x1000000)
#endif


#define TX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00100000)
#define RX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00300000)
#define RX_BUFFER_HIGH		(MEM_BASE_ADDR + 0x004FFFFF)

#define RESET_TIMEOUT_COUNTER	10000
#define MAX_PKT_LEN		        0x10000
#define POLL_TIMEOUT_COUNTER    10000000000U
#define NUMBER_OF_EVENTS     	1

static XAxiDma AxiDma;
static XAxiDma_Config *Config;

volatile u32 TxDone;
volatile u32 RxDone;
volatile u32 Error;

static void RxIntrHandler(void *Callback);
static void TxIntrHandler(void *Callback);
static int CheckData();

// #pragma section("image_data")
// static u32 receive_data[10];
// #pragma section(DEFAULT)

int setup_dma(XAxiDma *AxiDma){
    int Status;

    Config = XAxiDma_LookupConfig(XPAR_XAXIDMA_0_BASEADDR);
	if (!Config) {
		xil_printf("No config found for %d\r\n", XPAR_XAXIDMA_0_BASEADDR);

		return XST_FAILURE;
	}
    
	/* Initialize DMA engine */
	Status = XAxiDma_CfgInitialize(AxiDma, Config);

	if (Status != XST_SUCCESS) {
		xil_printf("Initialization failed %d\r\n", Status);
		return XST_FAILURE;
	}

	if (XAxiDma_HasSg(AxiDma)) {
		xil_printf("Device configured as SG mode \r\n");
		return XST_FAILURE;
	}

	Status = XSetupInterruptSystem(AxiDma, &TxIntrHandler,
				       Config->IntrId[0], Config->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XSetupInterruptSystem(AxiDma, &RxIntrHandler,
				       Config->IntrId[1], Config->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	} 

	// /* Disable all interrupts before setup */
	XAxiDma_IntrDisable(AxiDma, XAXIDMA_IRQ_ALL_MASK,
			    XAXIDMA_DMA_TO_DEVICE);

	XAxiDma_IntrDisable(AxiDma, XAXIDMA_IRQ_ALL_MASK,
			    XAXIDMA_DEVICE_TO_DMA);

	/* Enable all interrupts */
	XAxiDma_IntrEnable(AxiDma, XAXIDMA_IRQ_ALL_MASK,
			   XAXIDMA_DMA_TO_DEVICE);

	XAxiDma_IntrEnable(AxiDma, XAXIDMA_IRQ_ALL_MASK,
			   XAXIDMA_DEVICE_TO_DMA);

    return XST_SUCCESS;
}

// #define MATRIX_SIZE  100//6
// #define KERNEL_SIZE  3
// #define CELL_COUNT   2
// #define TOTAL_BYTES  (KERNEL_SIZE * KERNEL_SIZE) + (KERNEL_SIZE * (CELL_COUNT - 1)) + 1

// void prepare_region(int32_t *input, int32_t *output, int16_t x_pos, int16_t y_pos){
//     int32_t pos = x_pos + (MATRIX_SIZE * y_pos);
    
//     memcpy(output                                         , input + pos,                     TOTAL_BYTES);
//     memcpy(output + (KERNEL_SIZE  + (CELL_COUNT - 1))     , input + pos + MATRIX_SIZE,       TOTAL_BYTES);
//     memcpy(output + ((KERNEL_SIZE + (CELL_COUNT - 1)) * 2), input + pos + (MATRIX_SIZE * 2), TOTAL_BYTES);
// }

static u32 output[300] = {};
static u32 input[10000] = {};
int pointer = 0;

int process(XAxiDma *AxiDma, void* input, u16 in_len, int rcv_index){
    int Status;
    u32 *TxBufferPtr;
	u32 *RxBufferPtr;

    TxBufferPtr = (u32 *)input;
	RxBufferPtr = (u32 *)RX_BUFFER_BASE;

	/* Initialize flags before start transfer test  */
	TxDone = 0;
	RxDone = 0;
	Error  = 0;

	/* Flush the buffers before the DMA transfer, in case the Data Cache
	 * is enabled
	 */
	Xil_DCacheFlushRange((UINTPTR)TxBufferPtr, 1000 * 4);
	Xil_DCacheFlushRange((UINTPTR)RxBufferPtr, 800 * 4);

    // xil_printf("Testing %d\r\n", rcv_index);

	/* Send a packet */
    Status = XAxiDma_SimpleTransfer((XAxiDma *)AxiDma, (u32*) RxBufferPtr,
        (97 * 4 * 97), XAXIDMA_DEVICE_TO_DMA);

    Status = XAxiDma_SimpleTransfer(AxiDma, (u32*) TxBufferPtr,
                        (100 * 4 * 3), XAXIDMA_DMA_TO_DEVICE);

    // while(pointer!=4){

    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    /*
        * Wait for RX done or timeout
        */
    Status = Xil_WaitForEventSet(POLL_TIMEOUT_COUNTER, NUMBER_OF_EVENTS, &RxDone);
    if (Status != XST_SUCCESS) {
        xil_printf("Receive failed %d\r\n", Status);
        return XST_FAILURE;
    }
    // }

    return XST_SUCCESS;

}

//void image_convolution( )
    // ip_config
    // set_kernal
    // set_bias
    // feed data row by row
    // return output

//void image_maxpoolling( )
    // ip_config
    // feed data row by row
    // return output

XScuGic IntcInstance;



static void imageProcISR(void *CallBackRef){
	static int i=4;
	int status;
    u32 *RxBufferPtr;
    RxBufferPtr = (u32 *)RX_BUFFER_BASE;
	XScuGic_Disable(&IntcInstance,XPS_FPGA2_INT_ID);
	// status = checkIdle(XPAR_AXI_DMA_0_BASEADDR,0x4);
    // xil_printf("imageProcISR \n");
	// while(status == 0){
	// 	status = checkIdle(XPAR_AXI_DMA_0_BASEADDR,0x4);
    //     xil_printf("\timageProcISR status %d\n", status);
    // }
	// if(i<100){

	// Xil_DCacheFlushRange((UINTPTR)RxBufferPtr, 800 * 4);

    status = XAxiDma_SimpleTransfer((XAxiDma *)CallBackRef,(u32)&input[i*100],100 * 4,XAXIDMA_DMA_TO_DEVICE);
    // status = XAxiDma_SimpleTransfer((XAxiDma *)CallBackRef, (u32*) RxBufferPtr,
        // (97 * 4 * 1), XAXIDMA_DEVICE_TO_DMA);
    i++;
	// }
    // status = XAxiDma_SimpleTransfer((XAxiDma *)CallBackRef, (u32*) RxBufferPtr + (pointer *  (97 * 4)),
    //     (97 * 4), XAXIDMA_DEVICE_TO_DMA);
    xil_printf("imageProcISR \n");
	XScuGic_Enable(&IntcInstance,XPS_FPGA2_INT_ID);
}





int main(){
    int Status;
    int Index = 0;
    int Tries = 1;
    int out_len = 0;

    int width  = 6;
    int height = 6;
    int status = 0;

    // for(int i= 1; i <= 21; i++){
    //     Xil_Out32(XPAR_NET_ENGINE_0_BASEADDR + (i*4), 1);
    // }
    for(int i = 0; i < 10000; i++){
        input[i] = i;
    }

    // XTime start_time, end_time;

    setup_dma(&AxiDma);

    //Interrupt Controller Configuration
	XScuGic_Config *IntcConfig;
	IntcConfig = XScuGic_LookupConfig(XPAR_XSCUGIC_0_BASEADDR);
	status =  XScuGic_CfgInitialize(&IntcInstance, IntcConfig, IntcConfig->CpuBaseAddress);

	if(status != XST_SUCCESS){
		xil_printf("Interrupt controller initialization failed..");
		return -1;
	}

	XScuGic_SetPriorityTriggerType(&IntcInstance,XPS_FPGA2_INT_ID,0xA0,3);
	status = XScuGic_Connect(&IntcInstance,XPS_FPGA2_INT_ID,(Xil_InterruptHandler)imageProcISR,(void *)&AxiDma);
	if(status != XST_SUCCESS){
		xil_printf("Interrupt connection failed");
		return -1;
	}
    XScuGic_Enable(&IntcInstance,XPS_FPGA2_INT_ID);

    //s2mm
	XScuGic_SetPriorityTriggerType(&IntcInstance,XPS_FPGA0_INT_ID,0xA1,3);
	status = XScuGic_Connect(&IntcInstance,XPS_FPGA0_INT_ID,(Xil_InterruptHandler)RxIntrHandler,(void *)&AxiDma);
	if(status != XST_SUCCESS){
		xil_printf("Interrupt connection failed");
		return -1;
	}
    XScuGic_Enable(&IntcInstance,XPS_FPGA0_INT_ID);

    //mm2s
	XScuGic_SetPriorityTriggerType(&IntcInstance,XPS_FPGA1_INT_ID,0xA2,3);
	status = XScuGic_Connect(&IntcInstance,XPS_FPGA1_INT_ID,(Xil_InterruptHandler)TxIntrHandler,(void *)&AxiDma);
	if(status != XST_SUCCESS){
		xil_printf("Interrupt connection failed");
		return -1;
	}
    XScuGic_Enable(&IntcInstance,XPS_FPGA1_INT_ID);

	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,(Xil_ExceptionHandler)XScuGic_InterruptHandler,(void *)&IntcInstance);
	Xil_ExceptionEnable();

    // XTime_GetTime(&start_time);
    // xil_printf("Start %d \r\n", start_time);
    // for(int n = 0; n < 100;n++){
    //     // xil_printf("y:%d \r\n", n);
    //     for(int m = 0; m < 100; m=m+2){
    //         prepare_region(&imageData, &output, m, n);
    //         process(&AxiDma, &output, 12, m + ((MATRIX_SIZE-2) * n)); 
    //     }
    // }

    // XAxiDma_Reset(&AxiDma);

    // int  TimeOut = RESET_TIMEOUT_COUNTER;

    // while (TimeOut) {
    //     if (XAxiDma_ResetIsDone(&AxiDma)) {
    //         break;
    //     }

    //     TimeOut -= 1;
    // }

    process(&AxiDma, &input, 12, 1); 

    // convolute(&imageData, &output, 6, 6);
    // XTime_GetTime(&end_time);
    // xil_printf("End %d\r\n", end_time);

    CheckData();

	xil_printf("Successfully ran AXI DMA interrupt Example\r\n");

    // XDisconnectInterruptCntrl(Config->IntrId[0], Config->IntrParent);
	// XDisconnectInterruptCntrl(Config->IntrId[1], Config->IntrParent);

	xil_printf("--- Exiting main() --- \r\n");

	return XST_SUCCESS;
}




/*****************************************************************************/
/*
*
* This is the DMA TX Interrupt handler function.
*
* It gets the interrupt status from the hardware, acknowledges it, and if any
* error happens, it resets the hardware. Otherwise, if a completion interrupt
* is present, then sets the TxDone.flag
*
* @param	Callback is a pointer to TX channel of the DMA engine.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void TxIntrHandler(void *Callback)
{
    

	u32 IrqStatus;
	int TimeOut;
    u32 *RxBufferPtr;
    RxBufferPtr = (u32 *)RX_BUFFER_BASE;
	XAxiDma *AxiDmaInst = (XAxiDma *)Callback;

	/* Read pending interrupts */
	IrqStatus = XAxiDma_IntrGetIrq(AxiDmaInst, XAXIDMA_DMA_TO_DEVICE);

	/* Acknowledge pending interrupts */
    xil_printf("TxIntrHandler 1 %04X \r\n", IrqStatus);


	XAxiDma_IntrAckIrq(AxiDmaInst, IrqStatus, XAXIDMA_DMA_TO_DEVICE);
    xil_printf("TxIntrHandler 2 %04X \r\n", IrqStatus);
	/*
	 * If no interrupt is asserted, we do not do anything
	 */
	if (!(IrqStatus & XAXIDMA_IRQ_ALL_MASK)) {

		return;
	}

	/*
	 * If error interrupt is asserted, raise error flag, reset the
	 * hardware to recover from the error, and return with no further
	 * processing.
	 */
	if ((IrqStatus & XAXIDMA_IRQ_ERROR_MASK)) {

		Error = 1;

		/*
		 * Reset should never fail for transmit channel
		 */
		XAxiDma_Reset(AxiDmaInst);

		TimeOut = RESET_TIMEOUT_COUNTER;

		while (TimeOut) {
			if (XAxiDma_ResetIsDone(AxiDmaInst)) {
				break;
			}

			TimeOut -= 1;
		}

		return;
	}

	/*
	 * If Completion interrupt is asserted, then set the TxDone flag
	 */
     int Status;
	if ((IrqStatus & XAXIDMA_IRQ_IOC_MASK)) {

        XAxiDma_IntrDisable(AxiDmaInst, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DMA_TO_DEVICE);
        XAxiDma_IntrAckIrq(AxiDmaInst, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DMA_TO_DEVICE);
		RxDone  = 1;
        // pointer = pointer + 1;

        XAxiDma_IntrEnable(AxiDmaInst, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DMA_TO_DEVICE);
	}
}

/*****************************************************************************/
/*
*
* This is the DMA RX interrupt handler function
*
* It gets the interrupt status from the hardware, acknowledges it, and if any
* error happens, it resets the hardware. Otherwise, if a completion interrupt
* is present, then it sets the RxDone flag.
*
* @param	Callback is a pointer to RX channel of the DMA engine.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void RxIntrHandler(void *Callback)
{
	u32 IrqStatus;
    int Status;
	int TimeOut;
    u32 *RxBufferPtr;
    RxBufferPtr = (u32 *)RX_BUFFER_BASE;

	XAxiDma *AxiDmaInst = (XAxiDma *)Callback;


	/* Read pending interrupts */
	IrqStatus = XAxiDma_IntrGetIrq(AxiDmaInst, XAXIDMA_DEVICE_TO_DMA);

    xil_printf("RxIntrHandle 1 %04X \r\n", IrqStatus);

	/* Acknowledge pending interrupts */
	XAxiDma_IntrAckIrq(AxiDmaInst, IrqStatus, XAXIDMA_DEVICE_TO_DMA);
    xil_printf("RxIntrHandler 2 %04X \r\n", IrqStatus);

	/*
	 * If no interrupt is asserted, we do not do anything
	 */
	if (!(IrqStatus & XAXIDMA_IRQ_ALL_MASK)) {
		return;
	}

	/*
	 * If error interrupt is asserted, raise error flag, reset the
	 * hardware to recover from the error, and return with no further
	 * processing.
	 */
	if ((IrqStatus & XAXIDMA_IRQ_ERROR_MASK)) {

		Error = 1;

		/* Reset could fail and hang
		 * NEED a way to handle this or do not call it??
		 */
		XAxiDma_Reset(AxiDmaInst);

		TimeOut = RESET_TIMEOUT_COUNTER;

		while (TimeOut) {
			if (XAxiDma_ResetIsDone(AxiDmaInst)) {
				break;
			}

			TimeOut -= 1;
		}

		return;
	}

	/*
	 * If completion interrupt is asserted, then set RxDone flag
	 */
    RxDone = 1;
	if ((IrqStatus & XAXIDMA_IRQ_IOC_MASK)) {

		RxDone = 1;
        XAxiDma_IntrDisable(AxiDmaInst, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);
        XAxiDma_IntrAckIrq(AxiDmaInst, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);
		// TxDone  = 1;
        // pointer = pointer + 1;

        XAxiDma_IntrEnable(AxiDmaInst, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);

        // Status = XAxiDma_SimpleTransfer(AxiDmaInst, (u32*) RxBufferPtr + (pointer *  (97 * 4)),
        //         (97 * 4), XAXIDMA_DEVICE_TO_DMA);
        // Status = XAxiDma_SimpleTransfer(AxiDmaInst, (u32*) input + (pointer *  (100 * 4)), 
        //             (100 * 4), XAXIDMA_DMA_TO_DEVICE);
	}
}


static int CheckData()
{
	u32 *RxPacket;
    u32 *TxPacket;
	int Index = 0;
	u8 Value;

    float d = 3.144;


	RxPacket = (u32 *) RX_BUFFER_BASE;
    TxPacket = (u32 *) TX_BUFFER_BASE;

	/* Invalidate the DestBuffer before receiving the data, in case the
	 * Data Cache is enabled
	 */
	Xil_DCacheInvalidateRange((UINTPTR)RxPacket, 150 * 4);
    xil_printf("CNN Data : \r\n");
    xil_printf("Float Size %d : \r\n", sizeof(d));
	for (Index = 0; Index < 150; Index++) {
		// if (RxPacket[Index] != Value) {
		xil_printf("%d - %d \n", Index, RxPacket[Index]);
	}
    xil_printf("\nCNN Data End : \r\n");

	return XST_SUCCESS;
}

