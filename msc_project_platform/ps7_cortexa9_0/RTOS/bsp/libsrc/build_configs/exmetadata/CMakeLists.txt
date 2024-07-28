
cmake_minimum_required(VERSION 3.15)
project(bsp)
        
set(DRIVER_TARGETS axidma;canps;coresightps_dcc;devcfg;dmaps;emacps;gpiops;iicps;scugic;qspips;scutimer;scuwdt;sdps;ttcps;uartps;usbps;xadcps)
set(DRIVER_LOCATIONS D:/Vivado/Software/Vitis/2023.2/data/embeddedsw/XilinxProcessorIPLib/drivers/axidma_v9_17;D:/Vivado/Software/Vitis/2023.2/data/embeddedsw/XilinxProcessorIPLib/drivers/canps_v3_7;D:/Vivado/Software/Vitis/2023.2/data/embeddedsw/XilinxProcessorIPLib/drivers/coresightps_dcc_v1_9;D:/Vivado/Software/Vitis/2023.2/data/embeddedsw/XilinxProcessorIPLib/drivers/devcfg_v3_8;D:/Vivado/Software/Vitis/2023.2/data/embeddedsw/XilinxProcessorIPLib/drivers/dmaps_v2_9;D:/Vivado/Software/Vitis/2023.2/data/embeddedsw/XilinxProcessorIPLib/drivers/emacps_v3_19;D:/Vivado/Software/Vitis/2023.2/data/embeddedsw/XilinxProcessorIPLib/drivers/gpiops_v3_12;D:/Vivado/Software/Vitis/2023.2/data/embeddedsw/XilinxProcessorIPLib/drivers/iicps_v3_18;D:/Vivado/Software/Vitis/2023.2/data/embeddedsw/XilinxProcessorIPLib/drivers/scugic_v5_2;D:/Vivado/Software/Vitis/2023.2/data/embeddedsw/XilinxProcessorIPLib/drivers/qspips_v3_11;D:/Vivado/Software/Vitis/2023.2/data/embeddedsw/XilinxProcessorIPLib/drivers/scutimer_v2_5;D:/Vivado/Software/Vitis/2023.2/data/embeddedsw/XilinxProcessorIPLib/drivers/scuwdt_v2_5;D:/Vivado/Software/Vitis/2023.2/data/embeddedsw/XilinxProcessorIPLib/drivers/sdps_v4_2;D:/Vivado/Software/Vitis/2023.2/data/embeddedsw/XilinxProcessorIPLib/drivers/ttcps_v3_18;D:/Vivado/Software/Vitis/2023.2/data/embeddedsw/XilinxProcessorIPLib/drivers/uartps_v3_13;D:/Vivado/Software/Vitis/2023.2/data/embeddedsw/XilinxProcessorIPLib/drivers/usbps_v2_8;D:/Vivado/Software/Vitis/2023.2/data/embeddedsw/XilinxProcessorIPLib/drivers/xadcps_v2_7)

list(LENGTH DRIVER_TARGETS no_of_drivers)
set(index 0)

while(${index} LESS ${no_of_drivers})
    list(GET DRIVER_TARGETS ${index} drv)
    list(GET DRIVER_LOCATIONS ${index} drv_dir)
    set(exp_dir "${drv_dir}/examples")
    add_custom_target(
        ${drv}_example ALL
        COMMAND lopper -f -O D:/Vivado/Learning/Vitis/Learning_1/msc_project_platform/ps7_cortexa9_0/RTOS/bsp/libsrc/${drv} D:/Vivado/Learning/Vitis/Learning_1/msc_project_platform/ps7_cortexa9_0/RTOS/bsp/hw_artifacts/ps7_cortexa9_0_baremetal.dts -- bmcmake_metadata_xlnx ps7_cortexa9_0 ${exp_dir} drvcmake_metadata
        BYPRODUCTS x${drv}_exlist.yaml)
    MATH(EXPR index "${index}+1")
endwhile()
