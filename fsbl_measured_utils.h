/*
 * fsbl_measured_boot.h
 *
 *  Created on: Oct 24, 2018
 *      Author: jimwe
 */

#ifndef SRC_FSBL_MEASURE_UTILS_H_
#define SRC_FSBL_MEASURE_UTILS_H_

#include "xfsbl_authentication.h"
// #include "TPM.h"

#define CSU_ROM_DIGEST_ADDR (CSU_BASEADDR + 0x00000050U)
#define CSU_DIGEST_BYTE_COUNT 48U

#define TPM_MEASURE_ROM_BYTES_TX 64U
#define TPM_PCR0_HANDLE 0U

#define TPM_LOCALITY_ACCESS_FAILED 1u
#define TPM_COMMAND_TX_TIMEOUT 2u
#define TPM_RESPONSE_RX_TIMEOUT 4u

#define SHA256_NUM_BYTES 32u

/*TODO: Understand if locality is actually being used */
#define LOCALITY 0U

/* Function Prototypes */
// int Tpm_ReadPcr(uint32_t pcr_number, uint8_t *return_buffer);
int Tpm_Event (uint32_t pcr_number, uint16_t size, uint8_t *data);
// int Tpm_SendCommand(uint8_t *cmd, uint16_t length, uint16_t max_buffer_size, tpm_resp_code_t *resp);
void Tpm_Sha3WaitForDone(void);
void Tpm_Sha3Start(void);
void Tpm_Sha3Finish(uint32_t CurrentSize, u8 *Hash);
#endif /* SRC_FSBL_MEASURE_UTILS_H_ */
