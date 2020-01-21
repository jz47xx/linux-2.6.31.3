/*
 *  linux/drivers/mmc/host/jz_mmc/dma/jz_mmc_dma.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Copyright (c) Ingenic Semiconductor Co., Ltd.
 */


#include <asm/jzsoc.h>
#include <asm/jzmmc/jz_mmc_dma.h>

void jz_mmc_start_dma(int chan, unsigned long phyaddr, int count, int mode, int ts)
{
	unsigned long flags;

	flags = claim_dma_lock();
	disable_dma(chan);
	clear_dma_ff(chan);
	jz_set_dma_block_size(chan, ts);
	set_dma_mode(chan, mode);
	set_dma_addr(chan, phyaddr);
	set_dma_count(chan, count + (ts - 1));
	enable_dma(chan);
	release_dma_lock(flags);
}

void jz_mmc_send_dma(struct jz_mmc_host *host, int chan, unsigned long srcaddr,
		      unsigned long taraddr, int al_count, int unal_count)
{
	unsigned long flags;
	jz_dma_desc *desc;
	unsigned int next;

	flags = claim_dma_lock();

	memset(host->ua_desc, 0, 4096);

	next = (host->dma_desc_phys_addr + (sizeof(jz_dma_desc))) >> 4;
	desc = host->ua_desc;
	desc->dcmd = DMAC_DCMD_SAI | DMAC_DCMD_SWDH_32 | DMAC_DCMD_DWDH_32 | DMAC_DCMD_DS(DMA_TS) |
		DMAC_DCMD_LINK;
	desc->dsadr = srcaddr;      /* DMA source address */
	desc->dtadr = taraddr;      /* DMA target address */
	desc->ddadr = (next << 24) | al_count;
	desc++;
	desc->dcmd = DMAC_DCMD_SAI | DMAC_DCMD_SWDH_32 | DMAC_DCMD_DWDH_32 | DMAC_DCMD_DS_32BIT |
				DMAC_DCMD_TIE;
	desc->dsadr = srcaddr + (al_count * DMA_TS); /* DMA source address */
	desc->dtadr = taraddr;            /* DMA target address */
	desc->ddadr = unal_count;         /* counter */

	dma_cache_wback_inv((unsigned long)desc, 2 * (sizeof(jz_dma_desc)));

        /* Setup DMA descriptor address */
	REG_DMAC_DDA(chan) = host->dma_desc_phys_addr;

	/* Setup request source */
	if(host->pdev_id == 0)
		REG_DMAC_DRSR(chan) = DMAC_DRSR_RS_MSC0OUT;
	else if(host->pdev_id == 1)
		REG_DMAC_DRSR(chan) = DMAC_DRSR_RS_MSC1OUT;
	else
#ifdef  DMAC_DRSR_RS_MSC2OUT
		REG_DMAC_DRSR(chan) = DMAC_DRSR_RS_MSC2OUT;
#else
	;
#endif

	/* Setup DMA channel control/status register */
	REG_DMAC_DCCSR(chan) = DMAC_DCCSR_EN;	/* descriptor transfer, clear status, start channel */

	/* Enable DMA */
	REG_DMAC_DMACR(chan / HALF_DMA_NUM) = DMAC_DMACR_DMAE;

	/* DMA doorbell set -- start DMA now ... */
	REG_DMAC_DMADBSR(chan / HALF_DMA_NUM) = 1 << chan;

	release_dma_lock(flags);
}

void jz_mmc_receive_dma(struct jz_mmc_host *host, int chan, unsigned long srcaddr,
		      unsigned long taraddr, int al_count, int unal_count)
{
	unsigned long flags;
	jz_dma_desc *desc;
	unsigned int next;

	flags = claim_dma_lock();

	memset(host->ua_desc, 0, 4096);

	next = (host->dma_desc_phys_addr + (sizeof(jz_dma_desc))) >> 4;
	desc = host->ua_desc;
	desc->dcmd = DMAC_DCMD_DAI | DMAC_DCMD_SWDH_32 | DMAC_DCMD_DWDH_32 | DMAC_DCMD_DS(DMA_TS) |
		DMAC_DCMD_LINK;
	desc->dsadr = srcaddr;      /* DMA source address */
	desc->dtadr = taraddr;      /* DMA target address */
	desc->ddadr = (next << 24) | al_count;
	desc++;
	desc->dcmd = DMAC_DCMD_DAI | DMAC_DCMD_SWDH_32 | DMAC_DCMD_DWDH_32 | DMAC_DCMD_DS_32BIT |
				DMAC_DCMD_TIE;
	desc->dsadr = srcaddr;            /* DMA source address */
	desc->dtadr = taraddr + (al_count * DMA_TS); /* DMA target address */
	desc->ddadr = unal_count;        /* counter */

	dma_cache_wback((unsigned long)desc, 2 * (sizeof(jz_dma_desc)));

        /* Setup DMA descriptor address */
	REG_DMAC_DDA(chan) = host->dma_desc_phys_addr;

	/* Setup request source */
	if(host->pdev_id == 0)
		REG_DMAC_DRSR(chan) = DMAC_DRSR_RS_MSC0IN;
	else if(host->pdev_id == 1)
		REG_DMAC_DRSR(chan) = DMAC_DRSR_RS_MSC1IN;
	else
#ifdef  DMAC_DRSR_RS_MSC2IN
		REG_DMAC_DRSR(chan) = DMAC_DRSR_RS_MSC2IN;
#else
	;
#endif

	/* Setup DMA channel control/status register */
	REG_DMAC_DCCSR(chan) = DMAC_DCCSR_EN;	/* descriptor transfer, clear status, start channel */

	/* Enable DMA */
	REG_DMAC_DMACR(chan / HALF_DMA_NUM) = DMAC_DMACR_DMAE;

	/* DMA doorbell set -- start DMA now ... */
	REG_DMAC_DMADBSR(chan / HALF_DMA_NUM) = 1 << chan;

	release_dma_lock(flags);
}

static irqreturn_t jz_mmc_dma_rx_callback(int irq, void *devid)
{
	struct jz_mmc_host *host = devid;
	int chan = host->dma.rxchannel;

	disable_dma(chan);
	if (__dmac_channel_address_error_detected(chan)) {
		printk("%s: DMAC address error.\n",
		       __FUNCTION__);
		__dmac_channel_clear_address_error(chan);
	}
	if (__dmac_channel_transmit_end_detected(chan)) {
		__dmac_channel_clear_transmit_end(chan);
	}

	return IRQ_HANDLED;
}

static irqreturn_t jz_mmc_dma_tx_callback(int irq, void *devid)
{
	struct jz_mmc_host *host = devid;
	unsigned int chan = host->dma.txchannel;

	disable_dma(chan);
	if (__dmac_channel_address_error_detected(chan)) {
		printk("%s: DMAC address error.\n",
		       __FUNCTION__);
		__dmac_channel_clear_address_error(chan);
	}
	if (__dmac_channel_transmit_end_detected(chan)) {
		__dmac_channel_clear_transmit_end(chan);
	}

	return IRQ_HANDLED;
}

static int jz_mmc_init_dma(struct jz_mmc_host *host)
{
	host->dma.rxchannel = -1;
	host->dma.txchannel = -1;

	if (!host->dmares)
		return -ENODEV;

	host->dma.rxchannel = jz_request_dma(host->dmares->start, "dma-rx", jz_mmc_dma_rx_callback,
			       0, host);
	if (host->dma.rxchannel < 0) {
		printk(KERN_ERR "jz_request_dma failed for MMC Rx\n");
		goto err1;
	}

	if (host->dma.rxchannel < 6)
		REG_DMAC_DMACR(0) |= DMAC_DMACR_FMSC;
	else
		REG_DMAC_DMACR(1) |= DMAC_DMACR_FMSC;

	/* Request MMC Tx DMA channel */
	host->dma.txchannel = jz_request_dma(host->dmares->end, "dma-tx", jz_mmc_dma_tx_callback,
			       0, host);
	if (host->dma.txchannel < 0) {
		printk(KERN_ERR "jz_request_dma failed for MMC Tx\n");
		goto err2;
	}

	if (host->dma.txchannel < 6)
		REG_DMAC_DMACR(0) |= DMAC_DMACR_FMSC;
	else
		REG_DMAC_DMACR(1) |= DMAC_DMACR_FMSC;

	host->dma_buf = (unsigned int *)__get_free_page(GFP_KERNEL);

        /* setup descriptor */
	host->ua_desc = (jz_dma_desc *)__get_free_page(GFP_KERNEL);
	host->dma_desc_phys_addr = CPHYSADDR((unsigned long)host->ua_desc);

	memset(host->ua_desc, 0, 4096);
	dma_cache_wback_inv((unsigned long)host->ua_desc, 4096);

	return 0;

err2:
	jz_free_dma(host->dma.rxchannel);
err1:
	return -ENODEV;
}

static void jz_mmc_deinit_dma(struct jz_mmc_host *host)
{
	free_page((unsigned long)host->ua_desc);
	free_page((unsigned long)host->dma_buf);
	jz_free_dma(host->dma.rxchannel);
	jz_free_dma(host->dma.txchannel);
}

int jz_mmc_dma_register(struct jz_mmc_dma *dma)
{
	if(dma == NULL)
		return -ENOMEM;

	dma->init = jz_mmc_init_dma;
	dma->deinit = jz_mmc_deinit_dma;

	return 0;
}
