/*
 * pl_measurements.c
 *
 *  Created on: Dec 11, 2018
 *      Author: jimwe
 */

#include "fsbl_measured_boot.h"
#include "fsbl_measured_pl.h"
#include "fsbl_measured_utils.h"
// #include "TPM.h"
#include "xsecure.h"
#include "xsecure_hw.h"
#include "event_log.h"
#include "event_print.c"

extern u8 ReadBuffer[READ_BUFFER_SIZE];
static XSecure_Sha3 SecureSha3;

/**
 * This function is a one stop shop for all the PL measurement cases -
 *   (1) Unauthenticated & unencrypted (DDR system)
 *   (2) Unauthenticated & unencrypted (DDR-less system)
 *   (2) Unauthenticated & encrypted
 *   (3) Authenticated (TO DO)
 */
s32 Tpm_Measure_Pl(const XFsblPs *FsblInstancePtr, uint32_t PartitionNum,
		XSecure_Aes *AesPtr, u8 * Destination, u8 *Source) {
	s32 Status = XST_SUCCESS;
	uint8_t PartitionDigest[XFSBL_HASH_TYPE_SHA3]
					   __attribute__ ((aligned (4))) = {0};
   	const XFsblPs_PartitionHeader *PartitionHeader =
   			&FsblInstancePtr->ImageHeader.PartitionHeader[PartitionNum];
   	uint32_t SizeInWords = PartitionHeader->UnEncryptedDataWordLength;
	uint32_t SizeInBytes = PartitionHeader->UnEncryptedDataWordLength << 2;

	/* Handle case where the bitstream is NOT encrypted */
	if (XFsbl_IsEncrypted(PartitionHeader) != XIH_PH_ATTRB_ENCRYPTION) {

#ifdef XFSBL_PS_DDR
			/* Use CSU DMA to load Bit stream to PL */
			Status = Pl_Measure_Unencrypted(SizeInWords, Source,
					PartitionDigest);
			if (Status != XFSBL_SUCCESS) {
				goto END;
			}

#else
			/* In case of DDR less system, do the chunked transfer */
			Status = Pl_Measure_Chunked(FsblInstancePtr, PartitionNum,
					PartitionDigest);
			if (Status != XFSBL_SUCCESS) {
				goto END;
			}

#endif
	/* Handle the encrypted case */
	} else {
		Status = Pl_Measure_Encrypted(AesPtr, Destination, Source,
				SizeInBytes, PartitionDigest);
		if (Status != XFSBL_SUCCESS) {
			goto END;
		}
	}

	// Send the event to the TPM
   	XFsbl_PrintArray (DEBUG_DETAILED, PartitionDigest, XFSBL_HASH_TYPE_SHA3,
   			"PL SHA3-384 DIGEST");
   	// Status = Tpm_Event(Tpm_GetPCRforPartition(PartitionNum),
   	// 		XFSBL_HASH_TYPE_SHA3, PartitionDigest);

    
    event_log_metadata_t metadata;
    metadata.id = 2;                          // Assegna un ID all'evento
    metadata.name = "BITSTREAM";         // Assegna il nome dell'evento
    metadata.pcr = 2;    
    extern uint8_t *log_ptr2;
    log_ptr2 = event_log_record(PartitionDigest, EV_POST_CODE, &metadata, log_ptr2);

END:
	return Status;
}

/**
 * This function measures the PL for unauthenticated & unencrypted (DDR system)
 * It is based on the implementation of XFsbl_WriteToPcap()
 */
u32 Pl_Measure_Unencrypted(uint32_t SizeInWords, uint8_t *Source,
	uint8_t *PartitionDigest) {
	uint32_t Status;

	/* Startup the SHA3 Engine */
	XSecure_Sha3Initialize(&SecureSha3, &CsuDma);
	XSecure_Sha3Start(&SecureSha3);

	/* Send data to the PCAP & SHA3 */
	Status = Pl_Measure_WriteToPcap_with_SHA3(SizeInWords, Source);

	/* Finish the SHA3 */
	XSecure_Sha3Finish(&SecureSha3, PartitionDigest);

	return Status;
}

/**
 * This function measures the PL for unauthenticated & unencrypted
 * (DDR-less system)
 * It is based on the implementation of XFsbl_ChunkedBSTxfer()
 */
#ifndef XFSBL_PS_DDR
u32 Pl_Measure_Chunked(const XFsblPs *FsblInstancePtr, uint32_t PartitionNum,
		uint8_t *PartitionDigest) {

	uint32_t Status = XFSBL_SUCCESS;
	const XFsblPs_PartitionHeader *PartitionHeader;
	uint32_t ChunkSize = 0U;
	uint32_t RemainingBytes = 0U;
	uint32_t BitStreamSizeWord = 0U;
	uint32_t BitStreamSizeByte = 0U;
	uint32_t ImageOffset = 0U;
	uint32_t StartAddrByte = 0U;

	XFsbl_Printf(DEBUG_GENERAL,
		"Nonsecure Bitstream transfer in chunks to begin now\r\n");

	/**
	 * Assign the partition header to local variable
	 */
	PartitionHeader =
		&FsblInstancePtr->ImageHeader.PartitionHeader[PartitionNum];

	BitStreamSizeWord = PartitionHeader->UnEncryptedDataWordLength;
	ImageOffset = FsblInstancePtr->ImageOffsetAddress;

	StartAddrByte = ImageOffset + 4*(PartitionHeader->DataWordOffset);

	XFsbl_Printf(DEBUG_INFO,
			"Nonsecure Bitstream to be copied from %0x \r\n",
			StartAddrByte);

	/* Converting size in words to bytes */
	BitStreamSizeByte = BitStreamSizeWord*4;
	RemainingBytes = BitStreamSizeByte;

	/* Startup the SHA3 Engine */
	XSecure_Sha3Initialize(&SecureSha3, &CsuDma);
	XSecure_Sha3Start(&SecureSha3);

	while (RemainingBytes > 0) {
		/* Transfer the chunk from the device to OCM */
		ChunkSize = (RemainingBytes >= READ_BUFFER_SIZE) ?
				READ_BUFFER_SIZE : RemainingBytes;
		Status = FsblInstancePtr->DeviceOps.DeviceCopy(StartAddrByte,
				(PTRSIZE)ReadBuffer, ChunkSize);
		if (XFSBL_SUCCESS != Status)
		{
			XFsbl_Printf(DEBUG_GENERAL,
				"Copy of chunk from flash to OCM failed \r\n");
			goto END;
		}

		/* Send data to the PCAP & SHA3 */
		Status = Pl_Measure_WriteToPcap_with_SHA3(ChunkSize/4, &ReadBuffer[0]);
		if (XFSBL_SUCCESS != Status)
		{
			goto END;
		}

		/* Update pointers and byte remaining */
		StartAddrByte += ChunkSize;
		RemainingBytes -= ChunkSize;
	}

END:
	/* Finish the SHA3 */
	XFsbl_Printf(DEBUG_INFO, "FINISH SHA3 SIZE %d\r\n", SecureSha3.Sha3Len);
	XSecure_Sha3Finish(&SecureSha3, PartitionDigest);
	return Status;
}
#endif

/**
 * This function measures the PL for unauthenticated & encrypted (DDR system)
 */
u32 Pl_Measure_Encrypted(XSecure_Aes * AesPtr, uint8_t *Destination,
		uint8_t *Source, uint32_t Size, uint8_t *PartitionDigest) {

	uint32_t SssCfg = 0x0U;
	volatile s32 Status = XST_SUCCESS;
	uint32_t CurrentImgLen = 0x0U;
	uint32_t NextBlkLen = 0x0U;
	uint32_t PrevBlkLen = 0x0U;
	uint8_t *DestAddr= 0x0U;
	uint8_t *SrcAddr = 0x0U;
	uint8_t *GcmTagAddr = 0x0U;
	uint32_t BlockCnt = 0x0U;
	uint32_t ImageLen = 0x0U;
	uint32_t SssPcap = 0x0U;
	uint32_t SssDma = 0x0U;
	uint32_t SssAes = 0x0U;
	uint32_t SssSha = 0x0U;
	XCsuDma_Configure ConfigurValues = {0};
	uint32_t KeyClearStatus;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(AesPtr != NULL);
	/* Chunking is only for bitstream partitions */
	Xil_AssertNonvoid(((Destination == (u8*)XSECURE_DESTINATION_PCAP_ADDR)
				|| (AesPtr->IsChunkingEnabled
					== XSECURE_CSU_AES_CHUNKING_DISABLED)));

	/* Configure the SSS for AES. */
	SssAes = XSecure_SssInputAes(XSECURE_CSU_SSS_SRC_SRC_DMA);
	SssSha  = XSecure_SssInputSha3(XFSBL_CSU_SSS_SRC_SRC_DMA);

	if (Destination) {
		SssPcap = XSecure_SssInputPcap(XSECURE_CSU_SSS_SRC_AES);
		SssCfg =  SssPcap | SssAes | SssSha;
	} else {
		SssDma = XSecure_SssInputDstDma(XSECURE_CSU_SSS_SRC_AES);
		SssCfg = SssDma | SssAes | SssSha;
	}

	XSecure_SssSetup(SssCfg);

	/* Startup the SHA3 Engine */
	XSecure_Sha3Initialize(&SecureSha3, &CsuDma);
	XSecure_Sha3Start(&SecureSha3);

	/* Configure AES for Decryption */
	XSecure_WriteReg(AesPtr->BaseAddress, XSECURE_CSU_AES_CFG_OFFSET,
					 XSECURE_CSU_AES_CFG_DEC);

	DestAddr = Destination;
	ImageLen = Size;

	SrcAddr = (u8 *)Source ;
	GcmTagAddr = SrcAddr + XSECURE_SECURE_HDR_SIZE;

	/* Clear AES contents by reseting it. */
	XSecure_AesReset(AesPtr);

	/* Clear AES_KEY_CLEAR bits to avoid clearing of key */
	XSecure_WriteReg(AesPtr->BaseAddress,
			XSECURE_CSU_AES_KEY_CLR_OFFSET, (u32)0x0U);

	if(AesPtr->KeySel == XSECURE_CSU_AES_KEY_SRC_DEV)	{
		XSecure_AesKeySelNLoad(AesPtr);
	} else {
		u32 Count=0U, Value=0U;
		u32 Addr=0U;
		for(Count = 0U; Count < 8U; Count++) {
			/* Helion AES block expects the key in big-endian. */
			Value = Xil_Htonl(AesPtr->Key[Count]);

			Addr = AesPtr->BaseAddress +
				XSECURE_CSU_AES_KUP_0_OFFSET + (Count * 4);
			XSecure_Out32(Addr, Value);
		}
		XSecure_AesKeySelNLoad(AesPtr);
	}

	do	{
		PrevBlkLen = NextBlkLen;
		if (BlockCnt == 0) {
			/* Enable CSU DMA Src channel for byte swapping.*/
			XCsuDma_GetConfig(AesPtr->CsuDmaPtr,
				XCSUDMA_SRC_CHANNEL, &ConfigurValues);
			ConfigurValues.EndianType = 1U;
			XCsuDma_SetConfig(AesPtr->CsuDmaPtr,
				XCSUDMA_SRC_CHANNEL, &ConfigurValues);
		}

		/* Start decryption of Secure-Header/Block/Footer. */
		Status = XSecure_AesDecryptBlk(AesPtr, DestAddr,
						(const u8 *)SrcAddr,
						((const u8 *)GcmTagAddr),
						NextBlkLen, BlockCnt);
		/* If decryption failed then return error. */
		if(Status != XST_SUCCESS) {
			goto ENDF;
		}

		/*
		 * Find the size of next block to be decrypted.
		 * Size is in 32-bit words so mul it with 4
		 */
		NextBlkLen = Xil_Htonl(XSecure_ReadReg(AesPtr->BaseAddress,
					XSECURE_CSU_AES_IV_3_OFFSET)) * 4;

		/* Update the current image size. */
		CurrentImgLen += NextBlkLen;

		if(0U == NextBlkLen) {
			if(CurrentImgLen != Size) {
				/*
				 * If this is the last block then check
				 * if the current image != size in the header
				 * then return error.
				 */
				Status = XSECURE_CSU_AES_IMAGE_LEN_MISMATCH;
				goto ENDF;
			} else {
				goto ENDF;
			}
		} else {
			/*
			 * If this is not the last block then check
			 * if the current image > size in the header
			 * then return error.
			 */
			if(CurrentImgLen > ImageLen) {
				Status = XSECURE_CSU_AES_IMAGE_LEN_MISMATCH;
				goto ENDF;
			}
		}

		/*
		 * Update DestAddr and SrcAddr for next Block decryption.
		 */
		if (Destination != (u8*)XSECURE_DESTINATION_PCAP_ADDR) {
			DestAddr += PrevBlkLen;
		}
		SrcAddr = (GcmTagAddr + XSECURE_SECURE_GCM_TAG_SIZE);

		/*
		 * We are done with Secure header to decrypt the Block 0
		 * we can change the AES key source to KUP.
		 */
		AesPtr->KeySel = XSECURE_CSU_AES_KEY_SRC_KUP;
		XSecure_AesKeySelNLoad(AesPtr);
		/* Point IV to the CSU IV register. */
		AesPtr->Iv = (u32 *)(AesPtr->BaseAddress +
					(UINTPTR)XSECURE_CSU_AES_IV_0_OFFSET);

		/* Update the GcmTagAddr to get GCM-TAG for next block. */
		GcmTagAddr = SrcAddr + NextBlkLen + XSECURE_SECURE_HDR_SIZE;

		/* Update block count. */
		BlockCnt++;

	} while(1);

ENDF:
	/* Finish the SHA3 */
	XFsbl_Printf(DEBUG_INFO, "FINISH SHA3 SIZE %d\r\n", SecureSha3.Sha3Len);
	XSecure_Sha3Finish(&SecureSha3, PartitionDigest);

	/* Clear the AES keys */
	XSecure_AesReset(AesPtr);
	KeyClearStatus = XSecure_AesKeyZero(AesPtr);
	if (KeyClearStatus != XST_SUCCESS) {
		return KeyClearStatus;
	}

	return Status;
}


/**
 * This function sends data to the PCAP and SHA3 simultaneously.
 * It is based on the implementation of XFsbl_WriteToPcap()
 */
u32 Pl_Measure_WriteToPcap_with_SHA3(u32 WrSize, u8 *WrAddr) {
	uint32_t SssCfg;
	uint32_t Status;

	/*  Setup the  SSS, setup the PCAP to receive from DMA source */
	SssCfg = XSecure_SssInputPcap(XFSBL_CSU_SSS_SRC_SRC_DMA);
	SssCfg |= XSecure_SssInputSha3(XFSBL_CSU_SSS_SRC_SRC_DMA);
	XSecure_SssSetup(SssCfg);

	/* Setup the source DMA channel */
	XCsuDma_Transfer(&CsuDma, XCSUDMA_SRC_CHANNEL, (PTRSIZE) WrAddr, WrSize, 0);
	SecureSha3.Sha3Len += WrSize << 2;

	/* wait for the SRC_DMA to complete and the pcap to be IDLE */
	XCsuDma_WaitForDone(&CsuDma, XCSUDMA_SRC_CHANNEL){}

	/* Acknowledge the transfer has completed */
	XCsuDma_IntrClear(&CsuDma, XCSUDMA_SRC_CHANNEL, XCSUDMA_IXR_DONE_MASK);

	XFsbl_Printf(DEBUG_INFO, "DMA transfer done \r\n");
	Status = XFsbl_PcapWaitForDone();

	return Status;
}
