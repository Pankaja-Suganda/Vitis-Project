

/***************************** Include Files *******************************/
#include "net_engine.h"
#include "xaxidma.h"

/************************** Function Definitions ***************************/
NET_STATUS NET_ENGINE_init(Net_Engine_Inst *instance, UINTPTR baseaddr_p, UINTPTR dmaaddr_p){
    NET_STATUS ret = NET_ENGINE_FAIL;

    // setup interrupts
    
}


NET_STATUS NET_ENGINE_reset(Net_Engine_Config config){
    (void)config;
}

NET_STATUS NET_ENGINE_register_intr(Net_Engine_Inst *config, Net_Engine_Intr id, Net_Engine_ISR_t isr){
    (void)config;
    (void)id;
    (void)isr;
}
