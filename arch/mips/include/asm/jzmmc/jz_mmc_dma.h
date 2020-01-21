/*
 *  linux/drivers/mmc/host/jz_mmc/dma/jz_mmc_dma.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Copyright (c) Ingenic Semiconductor Co., Ltd.
 */

#ifndef __JZ_MMC_DMA_H__
#define __JZ_MMC_DMA_H__

#include <asm/jzmmc/jz_mmc_host.h>

#define DMA_TS 32 // ONLY BYTE

// No 64 byte in jz4760dmac.h
#define DMAC_DCMD_DS_64BYTE	(5 << DMAC_DCMD_DS_BIT)

#define DMAC_DCMD_DS(x) (x == 32? DMAC_DCMD_DS_32BYTE:DMAC_DCMD_DS_64BYTE)

struct jz_mmc_dma {

	int (*init) (struct jz_mmc_host *);
	void (*deinit) (struct jz_mmc_host *);
};

int jz_mmc_dma_register(struct jz_mmc_dma *dma);

void jz_mmc_start_dma(int chan, unsigned long phyaddr, int count, int mode, int ts);

void jz_mmc_send_dma(struct jz_mmc_host *host, int chan, unsigned long srcaddr,
			unsigned long taraddr, int al_count, int unal_count);

void jz_mmc_receive_dma(struct jz_mmc_host *host, int chan, unsigned long srcaddr,
			unsigned long taraddr, int al_count, int unal_count);

#endif /* __JZ_MMC_DMA_H__ */
