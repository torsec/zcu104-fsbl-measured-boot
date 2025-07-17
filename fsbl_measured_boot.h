/*
 * fsbl_measured_boot.h
 *
 *  Created on: Dec 11, 2018
 *      Author: jimwe
 */

#ifndef SRC_FSBL_MEASURED_BOOT_H_
#define SRC_FSBL_MEASURED_BOOT_H_

#define XFSBL_SLB9670_ERROR (0x72U)

#include "xfsbl_main.h"
#include "xfsbl_image_header.h"
// #include "TPM.h"

/* Type Declarations */
typedef struct {
	uint32_t ROM_PCR;
	uint32_t FSBL_PCR;
	uint32_t PARTITION_PCR[32];
} t_Partition_PCR_Map;

/* Function Prototypes */
// int Tpm_Startup();
// int Tpm_SelfTest();
int Tpm_Measure_Rom ();
int Tpm_Measure_Fsbl (XFsblPs_ImageHeader * ImageHeader);
int Tpm_Measure_Partition(XFsblPs * FsblInstancePtr,  uint32_t PartitionNum, PTRSIZE LoadAddress);
uint32_t Tpm_GetPCRforPartition(uint32_t partition_num);

#endif /* SRC_FSBL_MEASURED_BOOT_H_ */
