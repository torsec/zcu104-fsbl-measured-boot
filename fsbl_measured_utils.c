/*
 * fsbl_measured_boot.c
 *
 *  Created on: Oct 24, 2018
 *      Author: jimwe
 */

#include "fsbl_measured_boot.h"
#include "fsbl_measured_utils.h"
#include "xfsbl_authentication.h"

// int Tpm_ReadPcr(uint32_t pcr_number, uint8_t *return_buffer)
// {
//    int Status;
//    uint8_t cmd[512] = {0u};
//    tpm_resp_code_t response = (tpm_resp_code_t)0u;
//    uint8_t* cmd_ptr = cmd;
//    int i;

//    Tpm_InsertU16((uint16_t)TPM_ST_NO_SESSIONS, &cmd_ptr);
//    Tpm_InsertU32(20, &cmd_ptr);
//    Tpm_InsertU32(TPM_CC_PCR_READ, &cmd_ptr);

//    Tpm_InsertU32(0x000000001, &cmd_ptr);
//    Tpm_InsertU16(0x000B, &cmd_ptr);
//    *cmd_ptr = 0x03;   // 3 octets in select
//    cmd_ptr++;
//    // Define PCR Selection Mask
//    if (pcr_number < 8) {
// 	   *cmd_ptr = 0x1 << pcr_number;
// 	   cmd_ptr++;
// 	   *cmd_ptr = 0x0;
// 	   cmd_ptr++;
// 	   *cmd_ptr = 0x0;
// 	   cmd_ptr++;
//    } else {
// 	   *cmd_ptr = 0x0;
// 	   cmd_ptr++;
// 	   if (pcr_number < 16) {
// 		   *cmd_ptr = 0x1 << (pcr_number - 8);
// 		   cmd_ptr++;
// 		   *cmd_ptr = 0x0;
// 		   cmd_ptr++;
// 	   } else {
// 		   *cmd_ptr = 0x0;
// 		   cmd_ptr++;
// 		   if (pcr_number < 24) {
// 			   *cmd_ptr = 0x1 << (pcr_number - 16);
// 			   cmd_ptr++;
// 		   } else {
// 			   *cmd_ptr = 0x0;
// 			   cmd_ptr++;
// 		   }
// 	   }
//    }
//    Status = Tpm_SendCommand(cmd, 20, 512, &response);

//    // Copy PCR Value from Response
//    for(i=0;i<SHA256_NUM_BYTES;i++) {
// 	   return_buffer[i] = cmd[i+30];
//    }

//    XFsbl_Printf(DEBUG_DETAILED, "\r\n");
//    return Status;

// }/* end: void Tpm_ReadPcr */


int Tpm_Event (uint32_t pcr_number, uint16_t size, uint8_t *data)
{
   int Status = XFSBL_SUCCESS;
   uint8_t cmd[512] = {0u};
//    tpm_resp_code_t response = (tpm_resp_code_t)0u;
   uint8_t* cmd_ptr = cmd;
   int i;

   // If PCR >= 24 then just ignore the command
   if (pcr_number < 24) {
	   XFsbl_Printf(DEBUG_INFO, "SLB9670: Sending PCR_Event to PCR #%d\r\n", pcr_number);
// #ifdef FSBL_DEBUG_DETAILED
// 	   uint8_t PcrValue[32];
// 	   Tpm_ReadPcr(pcr_number, PcrValue);
// 	   XFsbl_PrintArray (DEBUG_DETAILED, PcrValue, SHA256_NUM_BYTES, "PCR VALUE BEFORE PCR_EVENT");
// 	   XFsbl_Printf(DEBUG_DETAILED, "\r\n");
// #endif

	//    // Command
	//    Tpm_InsertU16((uint16_t)TPM_ST_SESSIONS, &cmd_ptr);
	//    Tpm_InsertU32(29 + size, &cmd_ptr);
	//    Tpm_InsertU32(TPM_CC_PCR_EVENT, &cmd_ptr);
	//    Tpm_InsertU32(pcr_number, &cmd_ptr);

	//    // Authorization
	//    Tpm_InsertU32(9, &cmd_ptr);
	//    Tpm_InsertU32(TPM_RH_PW, &cmd_ptr);
	//    Tpm_InsertU16(0, &cmd_ptr);
	//    *cmd_ptr = 0x00;
	//    cmd_ptr++;
	//    Tpm_InsertU16(0, &cmd_ptr);

	//    // Add the digest to the data structure
	//    Tpm_InsertU16(size, &cmd_ptr);
	//    for(i=0;i<size;i++) {
	// 	   *cmd_ptr = data[i];
	// 	   cmd_ptr++;
	//    }

	//    // Send the command
	//    Status = Tpm_SendCommand(cmd, 29+size, 512, &response);

	   XFsbl_PrintArray (DEBUG_DETAILED, &cmd[42], XFSBL_HASH_TYPE_SHA3, "EVENT SHA256 DIGEST");

// #ifdef FSBL_DEBUG_DETAILED
// 	   Tpm_ReadPcr(pcr_number, PcrValue);
// 	   XFsbl_PrintArray (DEBUG_DETAILED, PcrValue, SHA256_NUM_BYTES, "PCR VALUE AFTER PCR_EVENT");
// 	   XFsbl_Printf(DEBUG_DETAILED, "\r\n");
// #endif
   }

   return Status;

}/* end: void Tpm_Event */

/**
 * This blocking function sends a command to the TPM and retrieves a response.
 */
// int Tpm_SendCommand(uint8_t *cmd, uint16_t tx_length, uint16_t max_buffer_size, tpm_resp_code_t *response) {
//  /* This stores the result of requesting the locality. */
//  tpm_tis_access_r access = TPM_TIS_ACCESS_TIMEOUT;

//  /* This stores the result of sending and receiving the command response. */
//  tpm_tis_status_r status = (tpm_tis_status_r)0u;

//  /* Request a locality in order to access the TPM. */
//  access = TpmTis_RequestAccess(LOCALITY);

//  /* Status for caller */
//  int Status = TPM_LOCALITY_ACCESS_FAILED;

//  /* Check the access and inform the user. */
//  if (access & TPM_TIS_ACCESS_ACTIVE)
//  {
//    /* Store the locality. */
//    TpmTis_SetLocality(LOCALITY);

//    /* Start the command state machine. */
//    TpmTis_SendCommandX(true, cmd, tx_length, &status);

//    /* Wait until the command is sent. THIS IS POLLED AND NOT IDEAL FOR RELEASE
//     * CODE. */
//    while (!TpmTis_SendCommandX(false, cmd, tx_length, &status))
//    {
//        /* Wait a little bit. */
//        Tpm_MicrosecondDelay(TPM_CMD_RETRY_DELAY);
//    }

//    /* Make sure a timeout did not occur. */
//    if (!(status & TPM_TIS_STATUS_TIMEOUT))
//    {
//       /* Start the receive state machine. */
//       TpmTis_ReadResponseX(true, cmd, max_buffer_size, &status);

//       /* Wait until the response is received. This is polled and not ideal for
//        * release code. */
//       while (!TpmTis_ReadResponseX(false, cmd, max_buffer_size,
//              &status))
//       {
//           /* Wait a little bit. */
//           Tpm_MicrosecondDelay(TPM_CMD_RETRY_DELAY);
//       }

//       /* Make sure a timeout didn't occur. */
//       if (!(status & TPM_TIS_STATUS_TIMEOUT))
//       {
//          /* Parse the response. */
//          *response = Tpm_ParseRc(cmd);

//          /* Indicate if the response is valid. */
//          if (*response == TPM_RC_SUCCESS)
//          {
//              Status = XFSBL_SUCCESS;
//          } else {
//         	 Status = XFSBL_FAILURE;
//          }

//       }

//       /* Report the error for the response receipt timeout */
//       else
//       {
//    	    Status = TPM_RESPONSE_RX_TIMEOUT;
//  	  }
//    }/* end: if (status & TPM_TIS_STATUS_GO) */

//    /* Report the error for the command send timeout */
//    else
//    {
// 	  Status = TPM_COMMAND_TX_TIMEOUT;
//    }

//  }/* end: if (access & TPM_TIS_ACCESS_ACTIVE) */

//  /* Otherwise, a failure occurred. */
//  else
//  {
//     /* Print the failure from the access register. */
// 	XFsbl_Printf(DEBUG_DETAILED, "SLB9670: Locality request failed.\r\n");
// 	XFsbl_Printf(DEBUG_DETAILED, "SLB9670: ERROR: 0x%02X\r\n\r\n", access);
//  }

//  return Status;
// }/* end: void Tpm_SendCommand */
