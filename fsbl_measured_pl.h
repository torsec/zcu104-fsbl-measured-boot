/*
 * pl_measurements.h
 *
 *  Created on: Dec 11, 2018
 *      Author: jimwe
 */

#ifndef SRC_FSBL_MEASURED_PL_H_
#define SRC_FSBL_MEASURED_PL_H_

#include "fsbl_measured_boot.h"
#include "xfsbl_hw.h"
#include "xfsbl_authentication.h"
#include "xfsbl_image_header.h"
#include "xfsbl_bs.h"

extern XCsuDma CsuDma;  /* CSU DMA instance */
extern u32 XFsbl_PcapWaitForDone(void);

s32 Tpm_Measure_Pl(const XFsblPs *FsblInstancePtr, uint32_t PartitionNum, XSecure_Aes *InstancePtr, uint8_t *Destination, uint8_t *Source);
u32 Pl_Measure_Unencrypted(uint32_t SizeInWords, uint8_t *Source, uint8_t *PartitionDigest);
u32 Pl_Measure_Chunked(const XFsblPs *FsblInstancePtr, uint32_t PartitionNum, uint8_t *PartitionDigest);
u32 Pl_Measure_Encrypted(XSecure_Aes * AesPtr, uint8_t *Destination, uint8_t *Source, uint32_t SizeInWords, uint8_t *PartitionDigest);
u32 Pl_Measure_WriteToPcap_with_SHA3(u32 WrSize, u8 *WrAddr);

#endif /* SRC_FSBL_MEASURED_PL_H_ */
