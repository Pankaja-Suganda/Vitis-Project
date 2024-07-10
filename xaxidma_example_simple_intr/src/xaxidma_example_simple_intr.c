/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xaxidma_example_simple_poll.c
 *
 * This file demonstrates how to use the xaxidma driver on the Xilinx AXI
 * DMA core (AXIDMA) to transfer packets in polling mode when the AXI DMA core
 * is configured in simple mode.
 *
 * This code assumes a loopback hardware widget is connected to the AXI DMA
 * core for data packet loopback.
 *
 * To see the debug print, you need a Uart16550 or uartlite in your system,
 * and please set "-DDEBUG" in your compiler options. You need to rebuild your
 * software executable.
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 4.00a rkv  02/22/11 New example created for simple DMA, this example is for
 *       	       simple DMA
 * 5.00a srt  03/06/12 Added Flushing and Invalidation of Caches to fix CRs
 *		       648103, 648701.
 *		       Added V7 DDR Base Address to fix CR 649405.
 * 6.00a srt  03/27/12 Changed API calls to support MCDMA driver.
 * 7.00a srt  06/18/12 API calls are reverted back for backward compatibility.
 * 7.01a srt  11/02/12 Buffer sizes (Tx and Rx) are modified to meet maximum
 *		       DDR memory limit of the h/w system built with Area mode
 * 7.02a srt  03/01/13 Updated DDR base address for IPI designs (CR 703656).
 * 9.1   adk  01/07/16 Updated DDR base address for Ultrascale (CR 799532) and
 *		       removed the defines for S6/V6.
 * 9.3   ms   01/23/17 Modified xil_printf statement in main function to
 *                     ensure that "Successfully ran" and "Failed" strings are
 *                     available in all examples. This is a fix for CR-965028.
 *       ms   04/05/17 Modified Comment lines in functions to
 *                     recognize it as documentation block for doxygen
 *                     generation of examples.
 * 9.9   rsp  01/21/19 Fix use of #elif check in deriving DDR_BASE_ADDR.
 * 9.10  rsp  09/17/19 Fix cache maintenance ops for source and dest buffer.
 * 9.14  sk   03/08/22 Delete DDR memory limits comments as they are not
 * 		       relevant to this driver version.
 * 9.15  sa   08/12/22 Updated the example to use latest MIG cannoical define
 * 		       i.e XPAR_MIG_0_C0_DDR4_MEMORY_MAP_BASEADDR.
 * 9.16  sa   09/29/22 Fix infinite loops in the example.
 * </pre>
 *
 * ***************************************************************************

 */
/***************************** Include Files *********************************/
#include "xaxidma.h"
#include "xparameters.h"
#include "xdebug.h"
#include "xil_io.h"
#include "xscugic.h"
#include "sleep.h"
#include "test_sample.h"
#if defined(XPAR_UARTNS550_0_BASEADDR)
#include "xuartns550_l.h"       /* to use uartns550 */
#endif

/******************** Constant Definitions **********************************/

/*
 * Device hardware build related constants.
 */

#ifndef SDT
#define DMA_DEV_ID		XPAR_AXIDMA_0_DEVICE_ID

#ifdef XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR		XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#elif defined (XPAR_MIG7SERIES_0_BASEADDR)
#define DDR_BASE_ADDR	XPAR_MIG7SERIES_0_BASEADDR
#elif defined (XPAR_MIG_0_C0_DDR4_MEMORY_MAP_BASEADDR)
#define DDR_BASE_ADDR	XPAR_MIG_0_C0_DDR4_MEMORY_MAP_BASEADDR
#elif defined (XPAR_PSU_DDR_0_S_AXI_BASEADDR)
#define DDR_BASE_ADDR	XPAR_PSU_DDR_0_S_AXI_BASEADDR
#endif

#else

#ifdef XPAR_MEM0_BASEADDRESS
#define DDR_BASE_ADDR		XPAR_MEM0_BASEADDRESS
#endif
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

#define MAX_PKT_LEN		1000000

#define TEST_START_VALUE	0xC

#define NUMBER_OF_TRANSFERS	10
#define POLL_TIMEOUT_COUNTER    1000000U

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

#if (!defined(DEBUG))
extern void xil_printf(const char *format, ...);
#endif

#ifndef SDT
int XAxiDma_SimplePollExample(u16 DeviceId);
#else
int XAxiDma_SimplePollExample(UINTPTR BaseAddress);
#endif
static int CheckData(int index);

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XAxiDma AxiDma;


/*****************************************************************************/
/**
* The entry point for this example. It invokes the example function,
* and reports the execution status.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if example finishes successfully
*		- XST_FAILURE if example fails.
*
* @note		None.
*
******************************************************************************/
int main()
{
	int Status;

	xil_printf("\r\n--- Entering main() --- \r\n");

	/* Run the poll example for simple transfer */
#ifndef SDT
	Status = XAxiDma_SimplePollExample(DMA_DEV_ID);
#else
	Status = XAxiDma_SimplePollExample(XPAR_XAXIDMA_0_BASEADDR);
#endif

	if (Status != XST_SUCCESS) {
		xil_printf("XAxiDma_SimplePoll Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran XAxiDma_SimplePoll Example\r\n");

	xil_printf("--- Exiting main() --- \r\n");

	return XST_SUCCESS;

}

#if defined(XPAR_UARTNS550_0_BASEADDR)
/*****************************************************************************/
/*
*
* Uart16550 setup routine, need to set baudrate to 9600, and data bits to 8
*
* @param	None.
*
* @return	None
*
* @note		None.
*
******************************************************************************/
static void Uart550_Setup(void)
{

	/* Set the baudrate to be predictable
	 */
	XUartNs550_SetBaud(XPAR_UARTNS550_0_BASEADDR,
			   XPAR_XUARTNS550_CLOCK_HZ, 9600);

	XUartNs550_SetLineControlReg(XPAR_UARTNS550_0_BASEADDR,
				     XUN_LCR_8_DATA_BITS);

}
#endif

XScuGic_Config *IntcConfig;
XScuGic IntcInstance;
static void imageProcISR(void *CallBackRef);
static void dmaReceiveISR(void *CallBackRef);
static void dmaSendISR(void *CallBackRef);

/*****************************************************************************/
/**
* The example to do the simple transfer through polling. The constant
* NUMBER_OF_TRANSFERS defines how many times a simple transfer is repeated.
*
* @param	DeviceId is the Device Id of the XAxiDma instance
*
* @return
*		- XST_SUCCESS if example finishes successfully
*		- XST_FAILURE if error occurs
*
* @note		None
*
*
******************************************************************************/
#ifndef SDT
int XAxiDma_SimplePollExample(u16 DeviceId)
#else
int XAxiDma_SimplePollExample(UINTPTR BaseAddress)
#endif
{
	XAxiDma_Config *CfgPtr;
	int Status;
	int Tries = NUMBER_OF_TRANSFERS;
	int Index;
	u32 *TxBufferPtr;
	u32 *RxBufferPtr;
	u32 Value;
	int TimeOut = POLL_TIMEOUT_COUNTER;

    // static u32 input[1000]= {};
    // static u32 output[1000] = {};

    // #define BUFFER_ALIGNMENT 32  // Example alignment
    // static u8 input[1000] __attribute__((aligned(BUFFER_ALIGNMENT)));
    // static u8 output[1000] __attribute__((aligned(BUFFER_ALIGNMENT)));


	// TxBufferPtr = (u32 *)input ;
	// RxBufferPtr = (u32 *)output;
    TxBufferPtr = (u32*) TX_BUFFER_BASE;
    RxBufferPtr = (u32*) RX_BUFFER_BASE;

    
	/* Initialize the XAxiDma device.
	 */
#ifndef SDT
	CfgPtr = XAxiDma_LookupConfig(DeviceId);
	if (!CfgPtr) {
		xil_printf("No config found for %d\r\n", DeviceId);
		return XST_FAILURE;
	}
#else
	CfgPtr = XAxiDma_LookupConfig(BaseAddress);
	if (!CfgPtr) {
		xil_printf("No config found for %d\r\n", BaseAddress);
		return XST_FAILURE;
	}
#endif

	Status = XAxiDma_CfgInitialize(&AxiDma, CfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialization failed %d\r\n", Status);
		return XST_FAILURE;
	}

	if (XAxiDma_HasSg(&AxiDma)) {
		xil_printf("Device configured as SG mode \r\n");
		return XST_FAILURE;
	}

	/* Disable interrupts, we use polling mode
	 */
	XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK,
			    XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK,
			    XAXIDMA_DMA_TO_DEVICE);

    XAxiDma_IntrEnable(&AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
    XAxiDma_IntrEnable(&AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

	//Interrupt Controller Configuration
	XScuGic_Config *IntcConfig;
	IntcConfig = XScuGic_LookupConfig(XPAR_XSCUGIC_0_BASEADDR);
	Status =  XScuGic_CfgInitialize(&IntcInstance, IntcConfig, IntcConfig->CpuBaseAddress);

	if(Status != XST_SUCCESS){
		xil_printf("Interrupt controller initialization failed..");
		return -1;
	}

	XScuGic_SetPriorityTriggerType(&IntcInstance,XPS_FPGA2_INT_ID,0xA0,3);
	Status = XScuGic_Connect(&IntcInstance,XPS_FPGA2_INT_ID,(Xil_InterruptHandler)imageProcISR,(void *)&AxiDma);
	if(Status != XST_SUCCESS){
		xil_printf("Interrupt connection failed");
		return -1;
	}
	XScuGic_Enable(&IntcInstance,XPS_FPGA2_INT_ID);
    

	XScuGic_SetPriorityTriggerType(&IntcInstance,XPS_FPGA1_INT_ID,0xA1,3);
	Status = XScuGic_Connect(&IntcInstance,XPS_FPGA1_INT_ID,(Xil_InterruptHandler)dmaReceiveISR,(void *)&AxiDma);
	if(Status != XST_SUCCESS){
		xil_printf("Interrupt connection failed");
		return -1;
	}
	XScuGic_Enable(&IntcInstance,XPS_FPGA1_INT_ID);

	XScuGic_SetPriorityTriggerType(&IntcInstance,XPS_FPGA0_INT_ID,0xA1,3);
	Status = XScuGic_Connect(&IntcInstance,XPS_FPGA0_INT_ID,(Xil_InterruptHandler)dmaSendISR,(void *)&AxiDma);
	if(Status != XST_SUCCESS){
		xil_printf("Interrupt connection failed");
		return -1;
	}
	XScuGic_Enable(&IntcInstance,XPS_FPGA0_INT_ID);

	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,(Xil_ExceptionHandler)XScuGic_InterruptHandler,(void *)&IntcInstance);
	Xil_ExceptionEnable();

	Value = TEST_START_VALUE;

	for (Index = 0; Index < MAX_PKT_LEN; Index ++) {
		TxBufferPtr[Index] = Index;
        RxBufferPtr[Index] = 0;
	}
	/* Flush the buffers before the DMA transfer, in case the Data Cache
	 * is enabled
	 */

    // XAxiDma_Reset(&AxiDma);

    // TimeOut = POLL_TIMEOUT_COUNTER;

    // while (TimeOut) {
    //     xil_printf("Trying to reset \n");
    //     if (XAxiDma_ResetIsDone(&AxiDma)) {
    //         xil_printf("Reset completed \n");
    //         break;
    //     }
    //     TimeOut -= 1;
    // }

	Xil_DCacheFlushRange((UINTPTR)TxBufferPtr, MAX_PKT_LEN);
	Xil_DCacheFlushRange((UINTPTR)RxBufferPtr, MAX_PKT_LEN);
    int first = 0;

    Status = XAxiDma_SimpleTransfer(&AxiDma, (UINTPTR) RxBufferPtr,
						(97 * 30 * 4), XAXIDMA_DEVICE_TO_DMA);

	for (Index = 0; Index < 100; Index ++) {
        xil_printf("Try - %d\n", Index);

		
		// Status = XAxiDma_SimpleTransfer(&AxiDma, (u32) RxBufferPtr,
		// 				(97 *  4), XAXIDMA_DEVICE_TO_DMA);

		if (Status != XST_SUCCESS) {
			// return XST_FAILURE;
		}

        if(first){
            Status = XAxiDma_SimpleTransfer(&AxiDma, (u32) TxBufferPtr + (Index*100),
						(100 * 4), XAXIDMA_DMA_TO_DEVICE);
        }
        else {
            Status = XAxiDma_SimpleTransfer(&AxiDma, (u32) TxBufferPtr,
						(300 * 4), XAXIDMA_DMA_TO_DEVICE);
            first = 1;
        }

		if (Status != XST_SUCCESS) {
			// return XST_FAILURE;
		}

		/*Wait till tranfer is done or 1usec * 10^6 iterations of timeout occurs*/
		while (TimeOut) {
            // xil_printf("dma busy..\n");
			if (!(XAxiDma_Busy(&AxiDma, XAXIDMA_DEVICE_TO_DMA)) &&
			    !(XAxiDma_Busy(&AxiDma, XAXIDMA_DMA_TO_DEVICE))) {
				break;
			}
			TimeOut--;
			usleep(1U);
		}

        xil_printf("timeout %d - %d\n", POLL_TIMEOUT_COUNTER, TimeOut);
        // for (int j = 0 ; j < 20; j++){
            xil_printf("\tReg Status -  %04x\n", Xil_In32(XPAR_NET_ENGINE_0_BASEADDR + (2*4)));
        // }
        for (int j = 0 ; j < 20; j++){
            xil_printf("%x, ", TxBufferPtr[j  + (Index * 100)]);
        }
        xil_printf("\n");

        Xil_DCacheInvalidateRange((UINTPTR)RxBufferPtr, 100);
        for (int j = 0 ; j < 20; j++){
            xil_printf("%x, ", RxBufferPtr[j]);
        }
        xil_printf("\n");
	}

	/* Test finishes successfully
	 */
	return XST_SUCCESS;
}



/*****************************************************************************/
/*
*
* This function checks data buffer after the DMA transfer is finished.
*
* @param	None
*
* @return
*		- XST_SUCCESS if validation is successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
static int CheckData(int index)
{
	u32 *RxPacket;
    u32 *TxPacket;
	int Index = 0;
	u32 Value;

	RxPacket = (u32 *) RX_BUFFER_BASE;
    TxPacket = (u32 *) TX_BUFFER_BASE ;
	Value = TEST_START_VALUE;

	/* Invalidate the DestBuffer before receiving the data, in case the
	 * Data Cache is enabled
	 */
	Xil_DCacheInvalidateRange((UINTPTR)RxPacket, 100);

	for (Index = 0; Index < 96; Index++) {
		// if (RxPacket[Index] != Value) {
			xil_printf("Data error %d - %d: %x/%x\r\n",
				   index, Index, (unsigned int)RxPacket[Index], (unsigned int)TxPacket[(index*100)+Index]);

			// return XST_FAILURE;
		// }
		// Value = (Value + 1) & 0xFF;
	}

	return XST_SUCCESS;
}

static void imageProcISR(void *CallBackRef){
    XScuGic_Disable(&IntcInstance,XPS_FPGA2_INT_ID);
    xil_printf("imageProcISR\r\n");
    XScuGic_Enable(&IntcInstance,XPS_FPGA2_INT_ID);
}

static void dmaReceiveISR(void *CallBackRef){
	int TimeOut = POLL_TIMEOUT_COUNTER;
    u32* RxBufferPtr;
    RxBufferPtr = (u32*) RX_BUFFER_BASE;
    int Status;
	XAxiDma_IntrDisable((XAxiDma *)CallBackRef, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrAckIrq((XAxiDma *)CallBackRef, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);
    xil_printf("dmaReceiveISR \n");

    /*Wait till tranfer is done or 1usec * 10^6 iterations of timeout occurs*/
    while (TimeOut) {
        // xil_printf("dma busy..\n");
        if (!(XAxiDma_Busy(&AxiDma, XAXIDMA_DEVICE_TO_DMA)) &&
            !(XAxiDma_Busy(&AxiDma, XAXIDMA_DMA_TO_DEVICE))) {
            break;
        }
        TimeOut--;
        usleep(1U);
    }

    Status = XAxiDma_SimpleTransfer(&AxiDma, (UINTPTR) RxBufferPtr,
						(97 * 30 * 4), XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrEnable((XAxiDma *)CallBackRef, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DEVICE_TO_DMA);
}

static void dmaSendISR(void *CallBackRef){
    XAxiDma_IntrDisable((XAxiDma *)CallBackRef, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DMA_TO_DEVICE);
	XAxiDma_IntrAckIrq((XAxiDma *)CallBackRef, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DMA_TO_DEVICE);
    xil_printf("dmaSendISR \n");
    XAxiDma_IntrEnable((XAxiDma *)CallBackRef, XAXIDMA_IRQ_IOC_MASK, XAXIDMA_DMA_TO_DEVICE);
}