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
// uint8_t *log_ptr_end_header; 
static uint8_t *log_ptr_end; 

// struct event_log {
//     int pcr_index;        // PCR register index (e.g., PCRIndex: 0)
//     // uint32_t event_type;       // Type of event (e.g., EventType: 1)
//     int digests_count;    // Number of digests (e.g., Digests Count: 1)
    
//     struct {
//         int algorithm_id; // Algorithm used for the digest (e.g., AlgorithmId: SHA256)
//         uint8_t digest[48];    // The SHA384 digest (48 bytes)
//     } digest_info;
    
//     uint32_t event_size;       // Size of the event data (e.g., EventSize: 14Bytes)
//     char event_data[40];    // Actual event data (description string)
// };


// #define RESERVED_MEMORY_ADDRESS ((struct event_log *)0x70000000) // Event log at 0x70000000
// #define MAX_EVENTS 10
// struct event_log event_array[MAX_EVENTS];
// int num_events = 0;
int partition = 1;

// // Function to add an event to the event log
// void add_event_to_log(struct event_log event) {
//     if (num_events < MAX_EVENTS) {
//         memcpy(&RESERVED_MEMORY_ADDRESS[num_events], &event, sizeof(struct event_log));
//         // RESERVED_MEMORY_ADDRESS[num_events] = event;  // Store event in reserved memory
//         num_events++;  // Update the local count
//     }
// }

// struct event_log create_event(int pcr_index, char *data, uint8_t *digest, uint32_t Size) {
//     size_t data_length = strlen(data);
//     struct event_log event;
    
    
//         event.pcr_index = pcr_index;
//         event.event_type = 3;
//         event.event_size = Size;
//         event.digests_count = 1;
//         event.digest_info.algorithm_id = 1; //sha384

//         for(int i=0;i<48;i++) 
//  		    event.digest_info.digest[i] = digest[i];
 	  
//         int i=0;
//         for(i=0; i<data_length; i++)
//             event.event_data[i]=data[i];
//         event.event_data[i]='\0';
    
//     add_event_to_log(event);
//     return event;
// }

// void print_log(){
//     XFsbl_Printf(DEBUG_INFO, "=====EVENT LOG=====\r\n");
//     for(int i=0; i<num_events; i++){
//         XFsbl_Printf(DEBUG_INFO, "Event:\t%s", event_array[i].event_data);
//         XFsbl_Printf(DEBUG_INFO, "\r\nEvent Size:\t%d bytes\r\n", event_array[i].event_size);
//         XFsbl_Printf(DEBUG_INFO, "Pcr index:\t%d\r\n", event_array[i].pcr_index);
//         XFsbl_Printf(DEBUG_INFO, "Digest count:\t%d\r\n", event_array[i].digests_count);
//         XFsbl_Printf(DEBUG_INFO, "Algorithm id:\t%d  (SHA-3/384)\r\n", event_array[i].digest_info.algorithm_id);
//         XFsbl_PrintArray(DEBUG_INFO, event_array[i].digest_info.digest, XFSBL_HASH_TYPE_SHA3, "Digest:");
//     }
//     XFsbl_Printf(DEBUG_INFO, "====================\r\n");
//     // num_events=0;
// }

// Function to print the event log
// void print_log() {
//     XFsbl_Printf(DEBUG_INFO, "=====EVENT LOG=====\r\n");
//     for(int i=0; i<num_events; i++){
//         XFsbl_Printf(DEBUG_INFO, "Event:\t%s", RESERVED_MEMORY_ADDRESS[i].event_data);
//         XFsbl_Printf(DEBUG_INFO, "\r\nEvent Size:\t%d bytes\r\n", RESERVED_MEMORY_ADDRESS[i].event_size);
//         XFsbl_Printf(DEBUG_INFO, "Pcr index:\t%d\r\n", RESERVED_MEMORY_ADDRESS[i].pcr_index);
//         XFsbl_Printf(DEBUG_INFO, "Digest count:\t%d\r\n", RESERVED_MEMORY_ADDRESS[i].digests_count);
//         XFsbl_Printf(DEBUG_INFO, "Algorithm id:\t%d  (SHA-3/384)\r\n", RESERVED_MEMORY_ADDRESS[i].digest_info.algorithm_id);
//         XFsbl_PrintArray(DEBUG_INFO, RESERVED_MEMORY_ADDRESS[i].digest_info.digest, XFSBL_HASH_TYPE_SHA3, "Digest:");
//     }
//     XFsbl_Printf(DEBUG_INFO, "====================\r\n");
// }

/*********************        FINE EVENT LOG              ************************************/

/*
 * This function returns the PCR number assigned to a given
 * partition number based on the Partition_PCR_Map.
 */
uint32_t Tpm_GetPCRforPartition(uint32_t partition_num) {
	/* Subtract 1 because partition 0 is the FSBL */
	return Partition_PCR_Map.PARTITION_PCR[partition_num -1];
}

/**
 * This function sends the startup command to the TPM for initialization
 */
// int Tpm_Startup() {
//    int Status;
//    uint8_t cmd[TPM_CMD_STARTUP_BYTES_TX] = {0u};
//    tpm_resp_code_t response = (tpm_resp_code_t)0u;
//    uint8_t* cmd_ptr = cmd;

// 	XFsbl_Printf(DEBUG_INFO,"SLB9670: Sending startup command\r\n");
// //   Tpm_FormStartup(TPM_SU_CLEAR, cmd);
//    Tpm_InsertU16((uint16_t)TPM_ST_NO_SESSIONS, &cmd_ptr);
//    Tpm_InsertU32(TPM_CMD_STARTUP_BYTES_TX, &cmd_ptr);
//    Tpm_InsertU32((uint32_t)TPM_CC_STARTUP, &cmd_ptr);
//    Tpm_InsertU16((uint16_t)TPM_SU_CLEAR, &cmd_ptr);
//    Status = Tpm_SendCommand(cmd, TPM_CMD_STARTUP_BYTES_TX,
// 		   TPM_CMD_STARTUP_BYTES_TX, &response);

//    return Status;

// }

/**
 * This function performs the TPM Selftest
 */
// int Tpm_SelfTest() {
//    int Status;
//    uint8_t cmd[TPM_CMD_STARTUP_BYTES_TX] = {0u};
//    tpm_resp_code_t response = (tpm_resp_code_t)0u;
//    uint8_t* cmd_ptr = cmd;

//    XFsbl_Printf(DEBUG_INFO,"SLB9670: Sending selftest command\r\n");
//    Tpm_InsertU16((uint16_t)TPM_ST_NO_SESSIONS, &cmd_ptr);
//    Tpm_InsertU32(11, &cmd_ptr);
//    Tpm_InsertU32(TPM_CC_SELFTEST, &cmd_ptr);
//    cmd_ptr[0] = (uint8_t) 1U;
//    cmd_ptr = cmd_ptr +1;
//    Status = Tpm_SendCommand(cmd, 11, 11, &response);
//    if (Status != XFSBL_SUCCESS) {
// 	   XFsbl_Printf(DEBUG_INFO, "SLB9670: Selftest failed\r\n");
//    } else {
// 	   XFsbl_Printf(DEBUG_INFO, "SLB670: Selftest passed\r\n");

//    }
//    return Status;
// }


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
    
    ///////////////////////////////

    // event_array[num_events] = create_event(0, "ROM", data, 48);
    // num_events+=1;
    // print_log();

    // Send the event to the TPM
//    Status = Tpm_Event(Partition_PCR_Map.ROM_PCR, XFSBL_HASH_TYPE_SHA3,
// 		   	   	   	  data);
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

       
    // if (LoadAddress == NULL) {
    // XFsbl_Printf(DEBUG_INFO, "LoadAddress is NULL\r\n");
    //     return XFSBL_FAILURE;
    // }else
    //     XFsbl_PrintArray(DEBUG_INFO, (uint8_t *) LoadAddress, 200, "FSBL Memory:");

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
     
    /////////////////////////////

    // event_array[num_events] = create_event(0, "FSBL", PartitionDigest, SizeInBytes);
    // num_events+=1;
    // print_log();

   	// // Send the event to the TPM
   	// Status = Tpm_Event(Partition_PCR_Map.FSBL_PCR, XFSBL_HASH_TYPE_SHA3,
   	// 		PartitionDigest);

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
        
        // event_array[num_events] = create_event(0, "Partition 1 - bl31", PartitionDigest, SizeInBytes);
        // num_events+=1;
        partition+=1;
    }else if(partition==2){
        event_log_metadata_t metadata;
        metadata.id = 4;                          // Assegna un ID all'evento
        metadata.name = "DEVICE TREE BLOB";         // Assegna il nome dell'evento
        metadata.pcr = 5;    

        log_ptr2 = event_log_record(PartitionDigest, EV_POST_CODE, &metadata, log_ptr2);
       
        // event_array[num_events] = create_event(0, "Partition 2 - U-BOOT", PartitionDigest, SizeInBytes);
        // num_events+=1;
        partition+=1;
    }else if(partition==3){
        event_log_metadata_t metadata;
        metadata.id = 5;                          // Assegna un ID all'evento
        metadata.name = "U-BOOT";         // Assegna il nome dell'evento
        metadata.pcr = 4;    

        log_ptr2 = event_log_record(PartitionDigest, EV_POST_CODE, &metadata, log_ptr2);
       
        // event_array[num_events] = create_event(0, "Partition 2 - U-BOOT", PartitionDigest, SizeInBytes);
        // num_events+=1;
        partition+=1;
    }else if(partition==4){
        event_log_metadata_t metadata;
        metadata.id = 6;                          // Assegna un ID all'evento
        metadata.name = "OP-TEE";         // Assegna il nome dell'evento
        metadata.pcr = 3;    

        log_ptr2 = event_log_record(PartitionDigest, EV_POST_CODE, &metadata, log_ptr2);
         
        // event_array[num_events] = create_event(0, "Partition 3 - OP-TEE", PartitionDigest, SizeInBytes);
        // num_events+=1;
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

        // pq_sign();
        ///////////////////////////////////////////////
        
        // uint8_t *start2 = log_ptr_end_header;
        // log_ptr_end = log_ptr2;
        // size_t size2 = log_ptr_end - start2;
        // event2_print(&start2, &size2);
        // print_log();
    }

	// // Send the event to the TPM
	// Status = Tpm_Event(Tpm_GetPCRforPartition(PartitionNum),
	// 		XFSBL_HASH_TYPE_SHA3, PartitionDigest);

	return XFSBL_SUCCESS;
}

