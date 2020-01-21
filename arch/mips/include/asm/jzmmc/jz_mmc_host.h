/*
 *  linux/drivers/mmc/host/jz_mmc/jz_mmc_host.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Copyright (c) Ingenic Semiconductor Co., Ltd.
 */

#ifndef __JZ_MMC_HOST_H__
#define __JZ_MMC_HOST_H__

#include <asm/jzmmc/jz_mmc_platform_data.h>
#include <asm/jzsoc.h>

#define USE_DMA

#define MMC_CLOCK_SLOW    400000      /* 400 kHz for initial setup */
#define MMC_CLOCK_FAST  20000000      /* 20 MHz for maximum for normal operation */
#define SD_CLOCK_FAST   24000000      /* 24 MHz for SD Cards */
#define SD_CLOCK_HIGH   24000000      /* 24 MHz for SD Cards */
#define MMC_NO_ERROR  0

#define NR_SG	1

#define MSC_1BIT_BUS 0
#define MSC_4BIT_BUS 1
#define MSC_8BIT_BUS 2

#define SZ_4K                           0x00001000

typedef struct jzsoc_dma_desc {
	volatile u32 ddadr;	/* Points to the next descriptor + flags */
	volatile u32 dsadr;	/* DSADR value for the current transfer */
	volatile u32 dtadr;	/* DTADR value for the current transfer */
	volatile u32 dcmd;	/* DCMD value for the current transfer */
} jzsoc_dma_desc;

struct jz_mmc_curr_req {
	struct mmc_request	*mrq;
	struct mmc_command	*cmd;
	struct mmc_data		*data;
	int r_type;
};

struct jz_mmc_host {
	struct mmc_host *mmc;
	spinlock_t lock;
	struct {
		int len;
		int dir;
		int rxchannel;
		int txchannel;
	} dma;
	struct {
		int index;
		int offset;
		int len;
	} pio;
	unsigned int clkrt;
	unsigned int cmdat;
	unsigned int imask;
	unsigned int power_mode;
	unsigned int eject;
	unsigned int oldstat;
	unsigned int pdev_id;
	void __iomem *base;

	struct resource *irqres;
	struct resource *memres;
	struct resource *dmares;

	struct jz_mmc_platform_data *plat;
	struct jz_mmc_curr_req curr;
	dma_addr_t sg_dma;
	struct jzsoc_dma_desc *sg_cpu;
	unsigned int dma_len;
	unsigned int dma_dir;
	int dma_ts;
	int msc_ack;
	int flag_cp;
	unsigned int *dma_buf;
	jz_dma_desc *ua_desc;
	dma_addr_t dma_desc_phys_addr;
	struct work_struct gpio_jiq_work;
	struct work_struct msc_jiq_work;
	struct workqueue_struct *msc_work_queue;
	wait_queue_head_t msc_wait_queue;
};

#if 0
struct jz_mmc_functions {
	void (*deinit) (struct jz_mmc_host *, struct platform_device *);
	int (*transmit_data) (struct jz_mmc_host *);
	void (*execute_cmd) (struct jz_mmc_host *, struct mmc_command *, unsigned int);
	void (*set_clock) (struct jz_mmc_host *, int);
	void (*msc_deinit) (struct jz_mmc_host *);
	int (*gpio_deinit) (struct jz_mmc_host *, struct platform_device *);
	void (*dma_deinit) (struct jz_mmc_host *);
};
#endif

#endif /* __JZ_MMC_HOST_H__ */
