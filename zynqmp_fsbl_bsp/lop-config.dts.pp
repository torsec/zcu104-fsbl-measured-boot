# 1 "C:/Users/Utente/Vitis2025/prova2/zynqmp_fsbl/zynqmp_fsbl_bsp/lop-config.dts"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "C:/Users/Utente/Vitis2025/prova2/zynqmp_fsbl/zynqmp_fsbl_bsp/lop-config.dts"

/dts-v1/;
/ {
        compatible = "system-device-tree-v1,lop";
        lops {
                lop_0 {
                        compatible = "system-device-tree-v1,lop,load";
                        load = "assists/baremetal_validate_comp_xlnx.py";
                };

                lop_1 {
                    compatible = "system-device-tree-v1,lop,assist-v1";
                    node = "/";
                    outdir = "C:/Users/Utente/Vitis2025/prova2/zynqmp_fsbl/zynqmp_fsbl_bsp";
                    id = "module,baremetal_validate_comp_xlnx";
                    options = "psu_cortexa53_0 C:/Xilinx/2025.1/Vitis/data/embeddedsw/lib/sw_services/xilskey_v7_7/src C:/Users/Utente/Vitis2025/_ide/.wsdata/.repo.yaml";
                };

        };
    };
