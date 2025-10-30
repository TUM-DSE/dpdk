/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2014 Intel Corporation
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/queue.h>

#include <rte_memory.h>
#include <rte_launch.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_debug.h>
#include <rte_malloc.h>
#include <rte_log.h>
#include <rte_memzone.h>
#include <rte_errno.h>

/* Test CVM shared memory allocation. 8< */
static void
test_cvm_shared_malloc(void)
{
	const size_t test_size = (1 << 20) * 4; // 4 MB
	const unsigned int test_align = 64;
	const uint8_t test_pattern = 0xAB;
	uint8_t *ptr;
	int i;
	int mismatch_count = 0;

	RTE_LOG(INFO, EAL, "Starting CVM shared memory allocation test\n");

	/* Allocate CVM shared memory */
	ptr = rte_cvm_shared_malloc("test_data", test_size, test_align);
	if (ptr == NULL) {
		RTE_LOG(ERR, EAL, "Failed to allocate CVM shared memory\n");
		return;
	}
	RTE_LOG(INFO, EAL, "Successfully allocated %zu bytes at address %p with alignment %u\n",
		test_size, (void *)ptr, test_align);

	/* Write test pattern to memory */
	for (i = 0; i < (int)test_size; i++)
		ptr[i] = test_pattern;
	RTE_LOG(INFO, EAL, "Wrote test pattern 0x%02X to all %zu bytes\n", test_pattern, test_size);

	/* Read back and verify */
	for (i = 0; i < (int)test_size; i++) {
		if (ptr[i] != test_pattern) {
			mismatch_count++;
		}
	}

	if (mismatch_count == 0) {
		RTE_LOG(INFO, EAL, "Verification successful: all %zu bytes match the written pattern\n",
			test_size);
	} else {
		RTE_LOG(ERR, EAL, "Verification failed: %d bytes do not match the pattern\n",
			mismatch_count);
	}

	/* Free the memory */
	rte_cvm_shared_free(ptr);
	RTE_LOG(INFO, EAL, "Successfully freed CVM shared memory\n");
}
/* >8 End of CVM shared memory allocation test. */

/* Test CVM shared memzone allocation. 8< */
static void
test_cvm_shared_memzone(void)
{
	const char *mz_name = "test_cvm_memzone";
	const size_t test_size = (1 << 20); // 1 MB
	const unsigned int test_align = 2 * 1024 * 1024; // 2MB alignment
	const uint32_t test_pattern = 0xDEADBEEF;
	const struct rte_memzone *mz;
	uint32_t *data;
	size_t i, num_words;
	int mismatch_count = 0;

	RTE_LOG(INFO, EAL, "Starting CVM shared memzone allocation test\n");

	/* Reserve CVM shared memzone */
	mz = rte_cvm_shared_memzone_reserve(mz_name, test_size, SOCKET_ID_ANY, 0);
	if (mz == NULL) {
		RTE_LOG(ERR, EAL, "Failed to reserve CVM shared memzone: %s\n",
			rte_strerror(rte_errno));
		return;
	}
	RTE_LOG(INFO, EAL, "Successfully reserved memzone '%s': addr=%p, len=%zu, hugepage_sz=%zu, socket_id=%d\n",
		mz->name, mz->addr, mz->len, mz->hugepage_sz, mz->socket_id);

	/* Get pointer to memzone data */
	data = (uint32_t *)mz->addr;
	num_words = test_size / sizeof(uint32_t);

	/* Write test pattern to memzone */
	RTE_LOG(INFO, EAL, "Writing test pattern 0x%08X to %zu words (%zu bytes)\n",
		test_pattern, num_words, test_size);
	for (i = 0; i < num_words; i++) {
		data[i] = test_pattern;
	}
	RTE_LOG(INFO, EAL, "Write complete\n");

	/* Read back and verify */
	RTE_LOG(INFO, EAL, "Verifying memzone contents\n");
	for (i = 0; i < num_words; i++) {
		if (data[i] != test_pattern) {
			if (mismatch_count < 10) {
				RTE_LOG(ERR, EAL, "Mismatch at word %zu: expected 0x%08X, got 0x%08X\n",
					i, test_pattern, data[i]);
			}
			mismatch_count++;
		}
	}

	if (mismatch_count == 0) {
		RTE_LOG(INFO, EAL, "Verification successful: all %zu words match the written pattern\n",
			num_words);
	} else {
		RTE_LOG(ERR, EAL, "Verification failed: %d words do not match the pattern\n",
			mismatch_count);
	}

	/* Free the memzone */
	RTE_LOG(INFO, EAL, "Freeing memzone '%s'\n", mz_name);
	if (rte_cvm_shared_memzone_free(mz) < 0) {
		RTE_LOG(ERR, EAL, "Failed to free memzone: %s\n",
			rte_strerror(rte_errno));
	} else {
		RTE_LOG(INFO, EAL, "Successfully freed CVM shared memzone\n");
	}
}
/* >8 End of CVM shared memzone allocation test. */

/* Test normal malloc allocation. 8< */
static void
test_normal_malloc(void)
{
	const size_t test_size = (1 << 16) * 4; // 256 KB
	const unsigned int test_align = 64;
	const uint8_t test_pattern = 0xCD;
	uint8_t *ptr;
	int i;
	int mismatch_count = 0;

	RTE_LOG(INFO, EAL, "Starting normal malloc allocation test\n");

	/* Allocate normal memory via rte_malloc */
	ptr = rte_malloc("test_data_normal", test_size, test_align);
	if (ptr == NULL) {
		RTE_LOG(ERR, EAL, "Failed to allocate normal memory\n");
		return;
	}
	RTE_LOG(INFO, EAL, "Successfully allocated %zu bytes at address %p with alignment %u\n",
		test_size, (void *)ptr, test_align);

	/* Write test pattern to memory */
	for (i = 0; i < (int)test_size; i++)
		ptr[i] = test_pattern;
	RTE_LOG(INFO, EAL, "Wrote test pattern 0x%02X to all %zu bytes\n", test_pattern, test_size);

	/* Read back and verify */
	for (i = 0; i < (int)test_size; i++) {
		if (ptr[i] != test_pattern) {
			mismatch_count++;
		}
	}

	if (mismatch_count == 0) {
		RTE_LOG(INFO, EAL, "Verification successful: all %zu bytes match the written pattern\n",
			test_size);
	} else {
		RTE_LOG(ERR, EAL, "Verification failed: %d bytes do not match the pattern\n",
			mismatch_count);
	}

	/* Free the memory */
	rte_free(ptr);
	RTE_LOG(INFO, EAL, "Successfully freed normal malloc memory\n");
}
/* >8 End of normal malloc allocation test. */

/* Test normal memzone allocation. 8< */
static void
test_normal_memzone(void)
{
	const char *mz_name = "test_normal_memzone";
	const size_t test_size = (1 << 20); // 1 MB
	const unsigned int test_align = 2 * 1024 * 1024; // 2MB alignment
	const uint32_t test_pattern = 0xCAFEBABE;
	const struct rte_memzone *mz;
	uint32_t *data;
	size_t i, num_words;
	int mismatch_count = 0;

	RTE_LOG(INFO, EAL, "Starting normal memzone allocation test\n");

	/* Reserve normal memzone */
	mz = rte_memzone_reserve(mz_name, test_size, SOCKET_ID_ANY, 0);
	if (mz == NULL) {
		RTE_LOG(ERR, EAL, "Failed to reserve normal memzone: %s\n",
			rte_strerror(rte_errno));
		return;
	}
	RTE_LOG(INFO, EAL, "Successfully reserved memzone '%s': addr=%p, len=%zu, hugepage_sz=%zu, socket_id=%d\n",
		mz->name, mz->addr, mz->len, mz->hugepage_sz, mz->socket_id);

	/* Get pointer to memzone data */
	data = (uint32_t *)mz->addr;
	num_words = test_size / sizeof(uint32_t);

	/* Write test pattern to memzone */
	RTE_LOG(INFO, EAL, "Writing test pattern 0x%08X to %zu words (%zu bytes)\n",
		test_pattern, num_words, test_size);
	for (i = 0; i < num_words; i++) {
		data[i] = test_pattern;
	}
	RTE_LOG(INFO, EAL, "Write complete\n");

	/* Read back and verify */
	RTE_LOG(INFO, EAL, "Verifying memzone contents\n");
	for (i = 0; i < num_words; i++) {
		if (data[i] != test_pattern) {
			if (mismatch_count < 10) {
				RTE_LOG(ERR, EAL, "Mismatch at word %zu: expected 0x%08X, got 0x%08X\n",
					i, test_pattern, data[i]);
			}
			mismatch_count++;
		}
	}

	if (mismatch_count == 0) {
		RTE_LOG(INFO, EAL, "Verification successful: all %zu words match the written pattern\n",
			num_words);
	} else {
		RTE_LOG(ERR, EAL, "Verification failed: %d words do not match the pattern\n",
			mismatch_count);
	}

	/* Free the memzone */
	RTE_LOG(INFO, EAL, "Freeing memzone '%s'\n", mz_name);
	if (rte_memzone_free(mz) < 0) {
		RTE_LOG(ERR, EAL, "Failed to free memzone: %s\n",
			rte_strerror(rte_errno));
	} else {
		RTE_LOG(INFO, EAL, "Successfully freed normal memzone\n");
	}
}
/* >8 End of normal memzone allocation test. */

/* Initialization of Environment Abstraction Layer (EAL). 8< */
int
main(int argc, char **argv)
{
	int ret;
	unsigned lcore_id;

	/* Set EAL log level to DEBUG for detailed logging */
	rte_log_set_level_pattern("lib.eal*", RTE_LOG_DEBUG);
	RTE_LOG(INFO, EAL, "Set lib.eal log level to DEBUG\n");


	ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_panic("Cannot init EAL\n");
	/* >8 End of initialization of Environment Abstraction Layer */


	/* Test normal malloc allocation. 8< */
	test_normal_malloc();
	/* >8 End of normal malloc allocation test. */

	/* Test normal memzone allocation. 8< */
	test_normal_memzone();
	/* >8 End of normal memzone allocation test. */

	/* Test CVM shared memory allocation. 8< */
	test_cvm_shared_malloc();
	/* >8 End of CVM shared memory allocation test. */

	/* Test CVM shared memzone allocation. 8< */
	test_cvm_shared_memzone();
	/* >8 End of CVM shared memzone allocation test. */

	RTE_LOG(INFO, EAL, "Cleanup\n");
	/* clean up the EAL */
	rte_eal_cleanup();

	RTE_LOG(INFO, EAL, "Cleanup complete\n");

	return 0;
}
