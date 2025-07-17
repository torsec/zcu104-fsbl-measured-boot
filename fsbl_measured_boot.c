/*
 * fsbl_measure_utils.c
 *
 *  Created on: Dec 11, 2018
 *      Author: jimwe
 */

#include "fsbl_measured_boot.h"
#include "fsbl_measured_utils.h"
#include <stdint.h>
#include <stdlib.h> //per la malloc
#include <string.h>
#include "event_log.h"
#include "event_print.c"

const t_Partition_PCR_Map Partition_PCR_Map = {
		0,           // ROM should be extended into PCR0
		4,           // FSBL should be extended into PCR1
		{   6,   7, 255, 255,  // Extend to PCR #6 and #7
		  255, 255, 255, 255,  // All others aren't measured
		  255, 255, 255, 255,
		  255, 255, 255, 255,
		  255, 255, 255, 255,
		  255, 255, 255, 255,
		  255, 255, 255, 255,
		  255, 255, 255, 255,
		}
};

/*********************        EVENT LOG              ************************************/
#define EVENT_LOG_START ((uint8_t *)0x79640000) // Definizione dell'indirizzo iniziale
#define EVENT_LOG_END   ((uint8_t *)0x79640FFF) 

uint8_t *log_ptr2 = EVENT_LOG_START;
static uint8_t *log_ptr_end; 
int partition = 1;

/*
 * This function returns the PCR number assigned to a given
 * partition number based on the Partition_PCR_Map.
 */
uint32_t Tpm_GetPCRforPartition(uint32_t partition_num) {
	/* Subtract 1 because partition 0 is the FSBL */
	return Partition_PCR_Map.PARTITION_PCR[partition_num -1];
}

 /**
  * This function creates a PCR event for the ROM digest
  */
 int Tpm_Measure_Rom () {


   uint8_t data[48];
   uint32_t temp;
   uint8_t *temp_ptr = (uint8_t *) &temp;
   int i, j;
   int Status = XFSBL_FAILURE;
    // Re-order the data to match LSB first
   for(i=0;i<48;i+=4) {
 	  temp = Xil_In32((UINTPTR) (CSU_ROM_DIGEST_ADDR+44-i));
 	  for(j=0;j<4;j++) {
 		  data[i + j] = temp_ptr[j];
 	  }
   }

    XFsbl_PrintArray(DEBUG_INFO, data, XFSBL_HASH_TYPE_SHA3, "ROM Digest:");

    event_log_init(EVENT_LOG_START, EVENT_LOG_END);
    log_ptr2 = event_log_write_header(log_ptr2);
       
    event_log_metadata_t metadata;
    metadata.id = 0;                          // Assegna un ID all'evento
    metadata.name = "ROM";         // Assegna il nome dell'evento
    metadata.pcr = 0;    

    log_ptr2 = event_log_record(data, EV_POST_CODE, &metadata, log_ptr2); 

   return XFSBL_SUCCESS;
}

 /**
  * This function creates a PCR event for the FSBL digest
  * Note: This includes the image header which has already been loaded
  *       as well as other variables which have changed up to this
  *       point
  */
int Tpm_Measure_Fsbl (XFsblPs_ImageHeader * ImageHeader) {
	int Status;
   	u8 PartitionDigest[XFSBL_HASH_TYPE_SHA3]
					   __attribute__ ((aligned (4))) = {0};
   	XFsblPs_PartitionHeader * PartitionHeader =
   			&ImageHeader->PartitionHeader[0];
   	PTRSIZE LoadAddress =
   			(PTRSIZE)PartitionHeader->DestinationLoadAddress;
   	uint32_t SizeInBytes =
   			PartitionHeader->UnEncryptedDataWordLength << 2;

   	/* Measure the FSBL */
   	XFsbl_Printf(DEBUG_DETAILED, "FSBL LOAD ADDRESS = %08x\r\n", LoadAddress);
   	XFsbl_Printf(DEBUG_DETAILED, "FSBL Size = %d bytes\r\n", SizeInBytes);

   	XFsbl_ShaStart((void * )NULL, XFSBL_HASH_TYPE_SHA3);
   	XFsbl_ShaDigest((uint8_t *) LoadAddress, 250000, PartitionDigest,
   			XFSBL_HASH_TYPE_SHA3);
   	XFsbl_PrintArray (DEBUG_DETAILED, PartitionDigest, XFSBL_HASH_TYPE_SHA3,
   			"FSBL SHA3-384 DIGEST");

    event_log_metadata_t metadata;
    metadata.id = 1;                          // Assegna un ID all'evento
    metadata.name = "FSBL";         // Assegna il nome dell'evento
    metadata.pcr = 1;    
  
    log_ptr2 = event_log_record(PartitionDigest, EV_POST_CODE, &metadata, log_ptr2);

   	return XFSBL_SUCCESS;
}

/**
 * This function creates a PCR event for a partition digest
 */
int Tpm_Measure_Partition(XFsblPs * FsblInstancePtr,  uint32_t PartitionNum,
		PTRSIZE LoadAddress) {
	int Status = XFSBL_SUCCESS;
	u8 PartitionDigest[XFSBL_HASH_TYPE_SHA3]
					   __attribute__ ((aligned (4))) = {0};
   	XFsblPs_PartitionHeader *PartitionHeader =
   			&FsblInstancePtr->ImageHeader.PartitionHeader[PartitionNum];
	uint32_t SizeInBytes =
			PartitionHeader->UnEncryptedDataWordLength << 2;

	XFsbl_Printf(DEBUG_DETAILED, "PARTITION %d LOAD ADDRESS = %08x\r\n",
			PartitionNum, LoadAddress);
	XFsbl_Printf(DEBUG_DETAILED, "PARTITION %d Size = %d bytes\r\n",
			PartitionNum, SizeInBytes);

	XFsbl_ShaStart((void * )NULL, XFSBL_HASH_TYPE_SHA3);
	XFsbl_ShaDigest((uint8_t *) LoadAddress, SizeInBytes, PartitionDigest,
			XFSBL_HASH_TYPE_SHA3);

	XFsbl_PrintArray (DEBUG_DETAILED, PartitionDigest, XFSBL_HASH_TYPE_SHA3,
			"PARTITION SHA3-384 DIGEST");

    if(partition==1){
        event_log_metadata_t metadata;
        metadata.id = 3;                          // Assegna un ID all'evento
        metadata.name = "BL31";         // Assegna il nome dell'evento
        metadata.pcr = 3;    

        log_ptr2 = event_log_record(PartitionDigest, EV_POST_CODE, &metadata, log_ptr2);
        partition+=1;
    }else if(partition==2){
        event_log_metadata_t metadata;
        metadata.id = 4;                          // Assegna un ID all'evento
        metadata.name = "DEVICE TREE BLOB";         // Assegna il nome dell'evento
        metadata.pcr = 5;    

        log_ptr2 = event_log_record(PartitionDigest, EV_POST_CODE, &metadata, log_ptr2);
        partition+=1;
    }else if(partition==3){
        event_log_metadata_t metadata;
        metadata.id = 5;                          // Assegna un ID all'evento
        metadata.name = "U-BOOT";         // Assegna il nome dell'evento
        metadata.pcr = 4;    

        log_ptr2 = event_log_record(PartitionDigest, EV_POST_CODE, &metadata, log_ptr2);
        partition+=1;
    }else if(partition==4){
        event_log_metadata_t metadata;
        metadata.id = 6;                          // Assegna un ID all'evento
        metadata.name = "OP-TEE";         // Assegna il nome dell'evento
        metadata.pcr = 3;    

        log_ptr2 = event_log_record(PartitionDigest, EV_POST_CODE, &metadata, log_ptr2);
        partition+=1;

        //////////////// PRINT EVENT LOG ///////////
        uint8_t *start = EVENT_LOG_START;
        log_ptr_end = log_ptr2;
        size_t size = log_ptr_end - start;

        XFsbl_Printf(DEBUG_INFO, "Event Log Size: %lu bytes\r\n", size);
        XFsbl_Printf(DEBUG_INFO, "Printing Event Log... starting at address %p\r\n", start);
        id_event_print(&start, &size);

        while (size != 0U) {
		    event2_print(&start, &size);
	    }

        XFsbl_Printf(DEBUG_INFO, "Flushing event log...\r\n");
        Xil_DCacheFlushRange(0x79640000, size);
        XFsbl_Printf(DEBUG_INFO, "Flush of event log completed...\r\n");
    }

	return XFSBL_SUCCESS;
}

