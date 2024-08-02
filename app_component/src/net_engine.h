
#ifndef NET_ENGINE_H
#define NET_ENGINE_H


/****************** Include Files ********************/
#include "xil_types.h"
#include "xstatus.h"
#include "net_engine_hw.h"
#include "net_engine_type.h"

/**************************** Type Definitions *****************************/
/**
 *
 * Write a value to a NET_ENGINE register. A 32 bit write is performed.
 * If the component is implemented in a smaller width, only the least
 * significant data is written.
 *
 * @param   BaseAddress is the base address of the NET_ENGINEdevice.
 * @param   RegOffset is the register offset from the base to write to.
 * @param   Data is the data written to the register.
 *
 * @return  None.
 *
 * @note
 * C-style signature:
 * 	void NET_ENGINE_mWriteReg(u32 BaseAddress, unsigned RegOffset, u32 Data)
 *
 */
#define NET_ENGINE_mWriteReg(BaseAddress, RegOffset, Data) \
  	Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))

/**
 *
 * Read a value from a NET_ENGINE register. A 32 bit read is performed.
 * If the component is implemented in a smaller width, only the least
 * significant data is read from the register. The most significant data
 * will be read as 0.
 *
 * @param   BaseAddress is the base address of the NET_ENGINE device.
 * @param   RegOffset is the register offset from the base to write to.
 *
 * @return  Data is the data from the register.
 *
 * @note
 * C-style signature:
 * 	u32 NET_ENGINE_mReadReg(u32 BaseAddress, unsigned RegOffset)
 *
 */
#define NET_ENGINE_mReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))

/************************** Function Prototypes ****************************/
/**
 *
 * Run a self-test on the driver/device. Note this may be a destructive test if
 * resets of the device are performed.
 *
 * If the hardware system is not built correctly, this function may never
 * return to the caller.
 *
 * @param   baseaddr_p is the base address of the NET_ENGINE instance to be worked on.
 *
 * @return
 *
 *    - XST_SUCCESS   if all self-test code passed
 *    - XST_FAILURE   if any self-test code failed
 *
 * @note    Caching must be turned off for this function to work.
 * @note    Self test may fail if data memory and device are not on the same bus.
 *
 */
NET_STATUS NET_ENGINE_Reg_SelfTest(void * baseaddr_p);

NET_STATUS NET_ENGINE_init(Net_Engine_Inst *instance, UINTPTR baseaddr_p, UINTPTR dmaaddr_p);

NET_STATUS NET_ENGINE_reset(Net_Engine_Inst *instance);

NET_STATUS NET_ENGINE_register_intr(Net_Engine_Inst *config, Net_Engine_Intr intr_type, int32_t intr_pin);

NET_STATUS NET_ENGINE_intr_setup(Net_Engine_Inst *instance, UINTPTR intraddr_p);

NET_STATUS NET_ENGINE_config(Net_Engine_Inst *instance, NET_CONFIG config);

NET_STATUS NET_ENGINE_process_cnn(Net_Engine_Inst *instance, u32 *input, u32 *output, CNN_Config_Data data, u32 row_length);

NET_STATUS NET_ENGINE_process_maxpooling(Net_Engine_Inst *instance, Net_Engine_Img *input, Net_Engine_Img *output);

NET_STATUS NET_ENGINE_config_row_length(Net_Engine_Inst *instance, u32 row_length );

#endif // NET_ENGINE_H
