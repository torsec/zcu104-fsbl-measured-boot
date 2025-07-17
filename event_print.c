/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>
#include <errno.h>


// #include <common/debug.h>
#include "event_log.h"
#include "xfsbl_misc.h"

// #if LOG_LEVEL >= EVENT_LOG_LEVEL

/*
 * Print TCG_EfiSpecIDEventStruct
 *
 * @param[in/out] log_addr	Pointer to Event Log
 * @param[in/out] log_size	Pointer to Event Log size
 */
static void id_event_print(uint8_t **log_addr, size_t *log_size)
{
	unsigned int i;
	uint8_t info_size, *info_size_ptr;
	void *ptr = *log_addr;
	id_event_headers_t *event = (id_event_headers_t *)ptr;
	id_event_algorithm_size_t *alg_ptr;
	uint32_t event_size, number_of_algorithms;
	size_t digest_len;
// #if ENABLE_ASSERTIONS
// 	const uint8_t *end_ptr = (uint8_t *)((uintptr_t)*log_addr + *log_size);
// 	bool valid = true;
// #endif

	// assert(*log_size >= sizeof(id_event_headers_t));

	/* The fields of the event log header are defined to be PCRIndex of 0,
	 * EventType of EV_NO_ACTION, Digest of 20 bytes of 0, and
	 * Event content defined as TCG_EfiSpecIDEventStruct.
	 */
    XFsbl_Printf(DEBUG_INFO, "TCG_EfiSpecIDEvent:\r\n");
	// LOG_EVENT("TCG_EfiSpecIDEvent:\n");
    XFsbl_Printf(DEBUG_INFO, "  PCRIndex           : %u\r\n", event->header.pcr_index);
	// LOG_EVENT("  PCRIndex           : %u\n", event->header.pcr_index);
	// assert(event->header.pcr_index == (uint32_t)PCR_0);

    XFsbl_Printf(DEBUG_INFO, "  EventType          : %u\r\n", event->header.event_type);
	// LOG_EVENT("  EventType          : %u\n", event->header.event_type);
	// assert(event->header.event_type == EV_NO_ACTION);

    // XFsbl_PrintArray(DEBUG_INFO, event->header.digest, 48, "Digest:");
    XFsbl_Printf(DEBUG_INFO, "  Digest             :");
	// LOG_EVENT("  Digest             :");
	for (i = 0U; i < sizeof(event->header.digest); ++i) {
		uint8_t val = event->header.digest[i];

		XFsbl_Printf(DEBUG_INFO, " %02x", val);
		if ((i & U(0xF)) == 0U) {
			XFsbl_Printf(DEBUG_INFO, "\r\n");
			XFsbl_Printf(DEBUG_INFO, "\r\t\t      :");
		}
#if ENABLE_ASSERTIONS
		if (val != 0U) {
			valid = false;
		}
#endif
	}
	if ((i & U(0xF)) != 0U) {
		XFsbl_Printf(DEBUG_INFO, "\r\n");
	}

	// assert(valid);

	/* EventSize */
	event_size = event->header.event_size;
    XFsbl_Printf(DEBUG_INFO, " \r\n EventSize          : %u\r\n", event_size);
	// LOG_EVENT("  EventSize          : %u\n", event_size);
    XFsbl_Printf(DEBUG_INFO, "  Signature          : %s\r\n",
			event->struct_header.signature);
	// LOG_EVENT("  Signature          : %s\n",
			// event->struct_header.signature);
    XFsbl_Printf(DEBUG_INFO, "  PlatformClass      : %u\r\n",
			event->struct_header.platform_class);
	// LOG_EVENT("  PlatformClass      : %u\n",
			// event->struct_header.platform_class);
    XFsbl_Printf(DEBUG_INFO, "  SpecVersion        : %u.%u.%u\r\n",
			event->struct_header.spec_version_major,
			event->struct_header.spec_version_minor,
			event->struct_header.spec_errata);
	// LOG_EVENT("  SpecVersion        : %u.%u.%u\n",
			// event->struct_header.spec_version_major,
			// event->struct_header.spec_version_minor,
			// event->struct_header.spec_errata);
    XFsbl_Printf(DEBUG_INFO, "  UintnSize          : %u\r\n",
			event->struct_header.uintn_size);
	// LOG_EVENT("  UintnSize          : %u\n",
			// event->struct_header.uintn_size);

	/* NumberOfAlgorithms */
	number_of_algorithms = event->struct_header.number_of_algorithms;
    XFsbl_Printf(DEBUG_INFO, "  NumberOfAlgorithms : %u\r\n", number_of_algorithms);
	// LOG_EVENT("  NumberOfAlgorithms : %u\n", number_of_algorithms);

	/* Address of DigestSizes[] */
	alg_ptr = event->struct_header.digest_size;

	/* Size of DigestSizes[] */
	digest_len = number_of_algorithms * sizeof(id_event_algorithm_size_t);
	// assert(((uintptr_t)alg_ptr + digest_len) <= (uintptr_t)end_ptr);

    XFsbl_Printf(DEBUG_INFO, "  DigestSizes        :\r\n");
	// LOG_EVENT("  DigestSizes        :\n");
	for (i = 0U; i < number_of_algorithms; ++i) {
        XFsbl_Printf(DEBUG_INFO, "    #%u AlgorithmId   : SHA", i);
		// LOG_EVENT("    #%u AlgorithmId   : SHA", i);
		uint16_t algorithm_id = alg_ptr[i].algorithm_id;

		switch (algorithm_id) {
		case TPM_ALG_SHA256:
			XFsbl_Printf(DEBUG_INFO, "256\r\n");
			break;
		case TPM_ALG_SHA384:
			XFsbl_Printf(DEBUG_INFO, "384\r\n");
			break;
		case TPM_ALG_SHA512:
			XFsbl_Printf(DEBUG_INFO, "512\r\n");
			break;
		default:
			XFsbl_Printf(DEBUG_INFO, "?\n");
            XFsbl_Printf(DEBUG_INFO, "Algorithm 0x%x not found\n", algorithm_id);
			// ERROR("Algorithm 0x%x not found\n", algorithm_id);
			// assert(false);
		}

        XFsbl_Printf(DEBUG_INFO, "       DigestSize    : %u\r\n",
					alg_ptr[i].digest_size);
		// LOG_EVENT("       DigestSize    : %u\n",
		// 			alg_ptr[i].digest_size);
	}

	/* Address of VendorInfoSize */
	info_size_ptr = (uint8_t *)((uintptr_t)alg_ptr + digest_len);
	// assert((uintptr_t)info_size_ptr <= (uintptr_t)end_ptr);

	info_size = *info_size_ptr++;
    XFsbl_Printf(DEBUG_INFO,"  VendorInfoSize     : %u\r\n", info_size);
	// LOG_EVENT("  VendorInfoSize     : %u\n", info_size);

	/* Check VendorInfo end address */
	// assert(((uintptr_t)info_size_ptr + info_size) <= (uintptr_t)end_ptr);

	/* Check EventSize */
	// assert(event_size == (sizeof(id_event_struct_t) +
	// 			digest_len + info_size));
	if (info_size != 0U) {
        XFsbl_Printf(DEBUG_INFO, "  VendorInfo         :");
		// LOG_EVENT("  VendorInfo         :");
		for (i = 0U; i < info_size; ++i) {
            XFsbl_Printf(DEBUG_INFO, " %02x", *info_size_ptr++)
			// (void)printf(" %02x", *info_size_ptr++);
		}
		XFsbl_Printf(DEBUG_INFO, "\r\n");
	}

	*log_size -= (uintptr_t)info_size_ptr - (uintptr_t)*log_addr;
	*log_addr = info_size_ptr;
}

/*
 * Print TCG_PCR_EVENT2
 *
 * @param[in/out] log_addr	Pointer to Event Log
 * @param[in/out] log_size	Pointer to Event Log size
 */
static void event2_print(uint8_t **log_addr, size_t *log_size)
{
	uint32_t event_size, count;
	size_t sha_size, digests_size = 0U;
	void *ptr = *log_addr;
#if ENABLE_ASSERTIONS
	const uint8_t *end_ptr = (uint8_t *)((uintptr_t)*log_addr + *log_size);
#endif

	// assert(*log_size >= sizeof(event2_header_t));

	XFsbl_Printf(DEBUG_INFO, "PCR_Event2:\r\n");
	XFsbl_Printf(DEBUG_INFO, "  PCRIndex           : %u\r\n",
			((event2_header_t *)ptr)->pcr_index);
	XFsbl_Printf(DEBUG_INFO, "  EventType          : %u\r\n",
			((event2_header_t *)ptr)->event_type);

	count = ((event2_header_t *)ptr)->digests.count;
	XFsbl_Printf(DEBUG_INFO, "  Digests Count      : %u\r\n", count);

	/* Address of TCG_PCR_EVENT2.Digests[] */
	ptr = (uint8_t *)ptr + sizeof(event2_header_t);
	// assert(((uintptr_t)ptr <= (uintptr_t)end_ptr) && (count != 0U));

	for (unsigned int i = 0U; i < count; ++i) {
		/* Check AlgorithmId address */
		// assert(((uintptr_t)ptr +
		// 	offsetof(tpmt_ha, digest)) <= (uintptr_t)end_ptr);

		XFsbl_Printf(DEBUG_INFO, "    #%u AlgorithmId   : SHA", i);
		switch (((tpmt_ha *)ptr)->algorithm_id) {
		case TPM_ALG_SHA256:
			sha_size = SHA256_DIGEST_SIZE;
			XFsbl_Printf(DEBUG_INFO, "256\r\n");
			break;
		case TPM_ALG_SHA384:
			sha_size = SHA384_DIGEST_SIZE;
			XFsbl_Printf(DEBUG_INFO, "384\r\n");
			break;
		case TPM_ALG_SHA512:
			sha_size = SHA512_DIGEST_SIZE;
			XFsbl_Printf(DEBUG_INFO, "512\r\n");
			break;
		default:
			XFsbl_Printf(DEBUG_INFO, "?\n");
			XFsbl_Printf(DEBUG_INFO, "Algorithm 0x%x not found\r\n",
				((tpmt_ha *)ptr)->algorithm_id);
			// panic();
		}

		/* End of Digest[] */
		ptr = (uint8_t *)((uintptr_t)ptr + offsetof(tpmt_ha, digest));
		// assert(((uintptr_t)ptr + sha_size) <= (uintptr_t)end_ptr);

		/* Total size of all digests */
		digests_size += sha_size;

		XFsbl_Printf(DEBUG_INFO, "       Digest        :");
		for (unsigned int j = 0U; j < sha_size; ++j) {
			XFsbl_Printf(DEBUG_INFO, " %02x", *(uint8_t *)ptr++);
			if ((j & U(0xF)) == U(0xF)) {
				XFsbl_Printf(DEBUG_INFO, "\r\n");
				if (j < (sha_size - 1U)) {
					XFsbl_Printf(DEBUG_INFO, "\r\t\t      :");
				}
			}
		}
	}

	/* TCG_PCR_EVENT2.EventSize */
	// assert(((uintptr_t)ptr + offsetof(event2_data_t, event)) <= (uintptr_t)end_ptr);

	event_size = ((event2_data_t *)ptr)->event_size;
	XFsbl_Printf(DEBUG_INFO, "  EventSize          : %u\r\n", event_size);

	/* Address of TCG_PCR_EVENT2.Event[EventSize] */
	ptr = (uint8_t *)((uintptr_t)ptr + offsetof(event2_data_t, event));

	/* End of TCG_PCR_EVENT2.Event[EventSize] */
	// assert(((uintptr_t)ptr + event_size) <= (uintptr_t)end_ptr);

	if ((event_size == sizeof(startup_locality_event_t)) &&
	     (strcmp((const char *)ptr, TCG_STARTUP_LOCALITY_SIGNATURE) == 0)) {
		XFsbl_Printf(DEBUG_INFO, "  Signature          : %s\r\n",
			((startup_locality_event_t *)ptr)->signature);
		XFsbl_Printf(DEBUG_INFO, "  StartupLocality    : %u\r\n",
			((startup_locality_event_t *)ptr)->startup_locality);
	} else {
		XFsbl_Printf(DEBUG_INFO, "  Event              : %s\r\n", (uint8_t *)ptr);
	}

	*log_size -= (uintptr_t)ptr + event_size - (uintptr_t)*log_addr;
	*log_addr = (uint8_t *)ptr + event_size;
}
// #endif	/* LOG_LEVEL >= EVENT_LOG_LEVEL */

/*
 * Print Event Log
 *
 * @param[in]	log_addr	Pointer to Event Log
 * @param[in]	log_size	Event Log size
 */
// void dump_event_log(uint8_t *log_addr, size_t log_size)
// {
// #if LOG_LEVEL >= EVENT_LOG_LEVEL
	// assert(log_addr != NULL);

	/* Print TCG_EfiSpecIDEvent */
	// id_event_print(&log_addr, &log_size);

	// while (log_size != 0U) {
	// 	event2_print(&log_addr, &log_size);
	// }
// #endif
// }
