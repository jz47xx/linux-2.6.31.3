/*
 *  linux/drivers/mmc/host/jz_mmc/msc/jz_mmc_msc.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Copyright (c) Ingenic Semiconductor Co., Ltd.
 */

#include <linux/dma-mapping.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sd.h>
#include <linux/mmc/sdio.h>
#include <linux/scatterlist.h>

#include <asm/jzsoc.h>
#include <asm/jzmmc/jz_mmc_msc.h>

#define MSC_STAT_ERR_BITS 0x3f

#if 0
#define TRACE_CMD_NUM MMC_SWITCH
#define TRACE_CMD_REQ()							\
	({								\
		if ( (TRACE_CMD_NUM == -1) || (TRACE_CMD_NUM == cmd->opcode) ) \
			printk("execute_cmd: opcode = %d cmdat = %#0x arg = %#0x data_flags = %#0x\n", \
			       cmd->opcode, cmdat, cmdarg,		\
			       host->curr.data ? host->curr.data->flags : 0); \
	})

#define TRACE_CMD_RES()							\
	({								\
		if ( (TRACE_CMD_NUM == -1) || (TRACE_CMD_NUM == cmd->opcode) ) \
			printk("cmd done: r_type = %d resp[0] = %#0x err = %d state = %#0x\n", \
			       host->curr.r_type, cmd->resp[0], cmd->error, \
			       REG_MSC_STAT(host->pdev_id));		\
	})
#else
#define TRACE_CMD_REQ() do {  } while(0)
#define TRACE_CMD_RES() do {  } while(0)
#endif

void jz_mmc_set_clock(struct jz_mmc_host *host, int rate);
static int jz_mmc_data_done(struct jz_mmc_host *host);

static void msc_irq_mask_all(int msc_id)
{
	REG_MSC_IMASK(msc_id) = 0xffff;
	REG_MSC_IREG(msc_id) = 0xffff;
}

static void jz_mmc_reset(struct jz_mmc_host *host)
{
	REG_MSC_STRPCL(host->pdev_id) = MSC_STRPCL_RESET;
	while (REG_MSC_STAT(host->pdev_id) & MSC_STAT_IS_RESETTING);
}

static inline int msc_calc_clkrt(int is_low, u32 rate)
{
	u32 clkrt;
	u32 clk_src = is_low ? 24000000 : 48000000;

	clkrt = 0;
	while (rate < clk_src) {
		clkrt++;
		clk_src >>= 1;
	}
	return clkrt;
}

void jz_mmc_set_clock(struct jz_mmc_host *host, int rate)
{
	int clkrt;

	/* __cpm_select_msc_clk_high will select 48M clock for MMC/SD card
	 * perhaps this will made some card with bad quality init fail,or
	 * bad stabilization.
	*/

	// Cause there is only ONE devider in CPM, the clock must only <= 24MHz
#ifndef CONFIG_SOC_JZ4750
#if 0
	if (rate > SD_CLOCK_FAST) {
		cpm_set_clock(CGU_MSCCLK, 48 * 1000 * 1000);
		clkrt = msc_calc_clkrt(0, rate);
	} else {
		cpm_set_clock(CGU_MSCCLK, 24 * 1000 * 1000);
		clkrt = msc_calc_clkrt(1, rate);
	}
#else
	if (rate > SD_CLOCK_FAST) {
		rate = SD_CLOCK_FAST;
		cpm_set_clock(CGU_MSCCLK, 24 * 1000 * 1000);
		clkrt = msc_calc_clkrt(1, rate);
	} else {
		cpm_set_clock(CGU_MSCCLK, 24 * 1000 * 1000);
		clkrt = msc_calc_clkrt(1, rate);
	}
#endif
	REG_MSC_CLKRT(host->pdev_id) = clkrt;
#else
		/* __cpm_select_msc_clk_high will select 48M clock for MMC/SD card
	 * perhaps this will made some card with bad quality init fail,or
	 * bad stabilization.
	*/
	if (rate > SD_CLOCK_FAST) {
		rate = SD_CLOCK_FAST;
		__cpm_select_msc_clk_high(host->pdev_id,1);	/* select clock source from CPM */

		//		__cpm_select_msc_clk(host->pdev_id,1);	/* select clock source from CPM */
		clkrt = msc_calc_clkrt(0, rate);
	} else {
		__cpm_select_msc_clk(host->pdev_id,1);	/* select clock source from CPM */
		clkrt = msc_calc_clkrt(1, rate);
	}

	// printk("clock rate = %d\n", __cpm_get_mscclk(0));
	REG_MSC_CLKRT(host->pdev_id) = clkrt;
#endif
}

static void jz_mmc_enable_irq(struct jz_mmc_host *host, unsigned int mask)
{
	unsigned long flags;

	spin_lock_irqsave(&host->lock, flags);
	host->imask &= ~mask;
	REG_MSC_IMASK(host->pdev_id) = host->imask;
	spin_unlock_irqrestore(&host->lock, flags);
}

static void jz_mmc_disable_irq(struct jz_mmc_host *host, unsigned int mask)
{
	unsigned long flags;

	spin_lock_irqsave(&host->lock, flags);
	host->imask |= mask;
	REG_MSC_IMASK(host->pdev_id) = host->imask;
	spin_unlock_irqrestore(&host->lock, flags);
}

static void jz_mmc_finish_request(struct jz_mmc_host *host, struct mmc_request *mrq)
{
	host->curr.mrq = NULL;
	host->curr.cmd = NULL;
	host->curr.data = NULL;
	mmc_request_done(host->mmc, mrq);
}

static int jz_mmc_cmd_done(struct jz_mmc_host *host, unsigned int stat)
{
	struct mmc_command *cmd = host->curr.cmd;
	int i, temp[16] = {0};
	unsigned char *buf;
	unsigned int data, v, w1, w2;

	if (!cmd)
		return 0;

	host->curr.cmd = NULL;
	buf = (u8 *) temp;
	switch (host->curr.r_type) {
	case 1:
	{
		/*
		 * Did I mention this is Sick.  We always need to
		 * discard the upper 8 bits of the first 16-bit word.
		 */

		data = REG_MSC_RES(host->pdev_id);
		buf[0] = (data >> 8) & 0xff;
		buf[1] = data & 0xff;

		data = REG_MSC_RES(host->pdev_id);
		buf[2] = (data >> 8) & 0xff;
		buf[3] = data & 0xff;

		data = REG_MSC_RES(host->pdev_id);
		buf[4] = data & 0xff;

		cmd->resp[0] =
			buf[1] << 24 | buf[2] << 16 | buf[3] << 8 |
			buf[4];

		// printk("opcode = %d, cmd->resp = 0x%08x\n", cmd->opcode, cmd->resp[0]);
		break;
	}
	case 2:
	{
		data = REG_MSC_RES(host->pdev_id);
		v = data & 0xffff;
		for (i = 0; i < 4; i++) {
			data = REG_MSC_RES(host->pdev_id);
			w1 = data & 0xffff;
			data = REG_MSC_RES(host->pdev_id);
			w2 = data & 0xffff;
			cmd->resp[i] = v << 24 | w1 << 8 | w2 >> 8;
			v = w2;
		}
		break;
	}
	case 0:
		break;
	}
	if (stat & MSC_STAT_TIME_OUT_RES) {
		/* :-( our customer do not want to see SO MANY timeouts :-(
		   so only CMD5 can return timeout error!!! */

		/*
		 * Note: we can not return timeout when CMD SD_SWITCH or MMC_SWITCH
		 * because we declared that out host->caps support MMC_CAP_4_BIT_DATA | MMC_CAP_8_BIT_DATA and MMC_CAP_MMC_HIGHSPEED
		 * if in the future some error occured because of this, we must add some code to remember
		 * which mode(SDIO/SD/MMC)  the MSC is in
		 */
		switch(cmd->opcode) {
		case SD_IO_SEND_OP_COND:
			//case SD_SWITCH:
			//case MMC_SWITCH:
		case SD_SEND_IF_COND:
		case MMC_APP_CMD:
			cmd->error = -ETIMEDOUT;
			break;
		default:
			printk("jz-msc%d: ignored MSC_STAT_TIME_OUT_RES, cmd=%d\n", host->pdev_id, cmd->opcode);
		}
	} else if (stat & MSC_STAT_CRC_RES_ERR && cmd->flags & MMC_RSP_CRC) {
		printk("jz-msc%d: MSC_STAT_CRC, cmd=%d\n", host->pdev_id, cmd->opcode);
		if (cmd->opcode == MMC_ALL_SEND_CID ||
		    cmd->opcode == MMC_SEND_CSD ||
		    cmd->opcode == MMC_SEND_CID) {
			/* a bogus CRC error can appear if the msb of
			   the 15 byte response is a one */
			if ((cmd->resp[0] & 0x80000000) == 0)
				cmd->error = -EILSEQ;
		}
	}

	TRACE_CMD_RES();

	if (host->curr.data && cmd->error == 0){
		jz_mmc_enable_irq(host, MSC_IMASK_DATA_TRAN_DONE);
	} else {
		jz_mmc_finish_request(host, host->curr.mrq);
	}

	return 1;
}

#ifdef USE_DMA
void jz_mmc_data_start(struct jz_mmc_host *host)
{
	struct mmc_data *data = host->curr.data;
	unsigned int nob = data->blocks;
	unsigned int block_size = data->blksz;
	//unsigned int *buf = 0;
	//int count;
	int unaligned_front, unaligned_behind, tail;
	int channel;
	int mode, i;

	if (data->flags & MMC_DATA_WRITE) {
		channel = host->dma.txchannel;
		mode = DMA_MODE_WRITE;
		host->dma.dir = DMA_TO_DEVICE;
	} else {
		channel = host->dma.rxchannel;
		mode = DMA_MODE_READ;
		host->dma.dir = DMA_FROM_DEVICE;
	}

	if (data->flags & MMC_DATA_STREAM)
		nob = 0xffff;

	REG_MSC_NOB(host->pdev_id) = nob;
	REG_MSC_BLKLEN(host->pdev_id) = block_size;

	host->dma.len =
	    dma_map_sg(mmc_dev(host->mmc), data->sg, data->sg_len,
		       host->dma.dir);

	for (i = 0; i < host->dma.len; i++) {
		host->sg_cpu[i].dtadr = sg_dma_address(&data->sg[i]);
		host->sg_cpu[i].dcmd = sg_dma_len(&data->sg[i]);

		unaligned_front = host->sg_cpu[i].dtadr & (DMA_TS - 1);                             // bytes
		unaligned_behind = (host->sg_cpu[i].dtadr + host->sg_cpu[i].dcmd) & (DMA_TS - 1);   // byte

		if((int)(host->sg_cpu[i].dcmd - unaligned_front) > DMA_TS) {


			if(unaligned_front == 0 && unaligned_behind == 0) {

				/*
				dma_cache_wback_inv((unsigned long)
						    CKSEG0ADDR(sg_dma_address(data->sg)) +
						    data->sg->offset,
						    host->sg_cpu[i].dcmd);
				*/
				dma_cache_wback_inv((unsigned long)CKSEG0ADDR(host->sg_cpu[i].dtadr),
						    host->sg_cpu[i].dcmd);

				jz_mmc_start_dma(channel, host->sg_cpu[i].dtadr,
						 host->sg_cpu[i].dcmd, mode, DMA_TS);

				host->flag_cp = 0;

			} else {
				/*
				printk("%s: addr unaligned\n", __FUNCTION__);
				printk("host->sg_cpu[i].dtadr = %x\n", host->sg_cpu[i].dtadr);
				printk("host->sg_cpu[i].dcmd = %d\n", host->sg_cpu[i].dcmd);
				printk("unaligned_front = %d\n", unaligned_front);
				printk("unaligned_behind = %d\n", unaligned_behind);
				*/

				memset(host->dma_buf, 0, 4096);
				tail = host->sg_cpu[i].dcmd % DMA_TS;

				if(tail) {
					if(mode == DMA_MODE_READ) {

						dma_cache_inv((unsigned long)host->dma_buf, host->sg_cpu[i].dcmd);

						jz_mmc_receive_dma(host, channel, CPHYSADDR(MSC_RXFIFO(host->pdev_id)),
								 CPHYSADDR(host->dma_buf), host->sg_cpu[i].dcmd / DMA_TS,
								   (tail + 3) / 4);

						host->flag_cp = 1;

/*
						dma_cache_inv((unsigned long)CKSEG0ADDR(host->sg_cpu[i].dtadr),
								    host->sg_cpu[i].dcmd);
						jz_mmc_start_dma(channel, host->sg_cpu[i].dtadr, host->sg_cpu[i].dcmd,
								 mode, 4);

						host->flag_cp = 0;
*/

					} else {

						memcpy(host->dma_buf, (void *)CKSEG0ADDR(host->sg_cpu[i].dtadr),
						     host->sg_cpu[i].dcmd);

						dma_cache_wback_inv((unsigned long)host->dma_buf, host->sg_cpu[i].dcmd);

						jz_mmc_send_dma(host, channel, CPHYSADDR(host->dma_buf),
								CPHYSADDR(MSC_TXFIFO(host->pdev_id)),
								host->sg_cpu[i].dcmd / DMA_TS, (tail + 3) / 4);
/*
						dma_cache_wback_inv((unsigned long)CKSEG0ADDR(host->sg_cpu[i].dtadr),
								    host->sg_cpu[i].dcmd);
						jz_mmc_start_dma(channel, host->sg_cpu[i].dtadr, host->sg_cpu[i].dcmd,
								 mode, 4);
*/
						host->flag_cp = 0;
					}
				} else {
					if(mode == DMA_MODE_READ) {
/*
						printk("host->sg_cpu[i].dtadr = %x\n", host->sg_cpu[i].dtadr);
						printk("unaligned_front = %d\n", unaligned_front);
						printk("unaligned_behind = %d\n", unaligned_behind);
						printk("host->sg_cpu[i].dcmd = %d\n", host->sg_cpu[i].dcmd);
*/
						dma_cache_inv((unsigned long)host->dma_buf, host->sg_cpu[i].dcmd);

						jz_mmc_start_dma(channel, CPHYSADDR(host->dma_buf), host->sg_cpu[i].dcmd,
								 mode, DMA_TS);

						host->flag_cp = 1;

					} else {
						memcpy(host->dma_buf, (void *)CKSEG1ADDR(host->sg_cpu[i].dtadr),
						       host->sg_cpu[i].dcmd);

						dma_cache_wback_inv((unsigned long)host->dma_buf, host->sg_cpu[i].dcmd);

						jz_mmc_start_dma(channel, CPHYSADDR(host->dma_buf), host->sg_cpu[i].dcmd,
								 mode, DMA_TS);

						host->flag_cp = 0;
					}
				}

			}
		} else {

			if(mode == DMA_MODE_READ) {
				dma_cache_inv((unsigned long)CKSEG0ADDR(host->sg_cpu[i].dtadr), host->sg_cpu[i].dcmd);
				jz_mmc_start_dma(channel, host->sg_cpu[i].dtadr, host->sg_cpu[i].dcmd, mode, 4);

				host->flag_cp = 0;
			} else {
				memset(host->dma_buf, 0, 4096);
				memcpy(host->dma_buf, (void *)CKSEG1ADDR(host->sg_cpu[i].dtadr),
				       host->sg_cpu[i].dcmd);

				dma_cache_wback_inv((unsigned long)host->dma_buf, host->sg_cpu[i].dcmd);

				jz_mmc_start_dma(channel, CPHYSADDR(host->dma_buf), host->sg_cpu[i].dcmd, mode, 4);

				host->flag_cp = 0;
			}
		}
	}
}
#else

static void jz_mmc_receive_pio(struct jz_mmc_host *host)
{
	struct mmc_data *data = host->curr.data;
	int sg_len = 0, max = 0, count = 0;
	unsigned int *buf = 0;
	struct scatterlist *sg;
	unsigned int nob;

	nob = data->blocks;

	REG_MSC_NOB(host->pdev_id) = nob;
	REG_MSC_BLKLEN(host->pdev_id) = data->blksz;

	max = host->pio.len;
	if (host->pio.index < host->dma.len) {
		sg = &data->sg[host->pio.index];
		buf = sg_virt(sg) + host->pio.offset;

		/* This is the space left inside the buffer */
		sg_len = sg_dma_len(&data->sg[host->pio.index]) - host->pio.offset;
		/* Check to if we need less then the size of the sg_buffer */
		if (sg_len < max) max = sg_len;
	}

	max = max / 4;
	for(count = 0; count < max; count++) {
		while (REG_MSC_STAT(host->pdev_id) & MSC_STAT_DATA_FIFO_EMPTY)
			;
		*buf++ = REG_MSC_RXFIFO(host->pdev_id);
	}
	host->pio.len -= count;
	host->pio.offset += count;

	if (sg_len && count == sg_len) {
		host->pio.index++;
		host->pio.offset = 0;
	}
}

static void jz_mmc_send_pio(struct jz_mmc_host *host)
{
	int sg_len, max, count = 0;
	unsigned int *wbuf = 0;
	unsigned int nob;
	struct mmc_data *data = host->curr.data;
	struct scatterlist *sg;

	nob = data->blocks;

	REG_MSC_NOB(host->pdev_id) = nob;
	REG_MSC_BLKLEN(host->pdev_id) = data->blksz;

	/* This is the pointer to the data buffer */
	sg = &data->sg[host->pio.index];
	wbuf = sg_virt(sg) + host->pio.offset;

	/* This is the space left inside the buffer */
	sg_len = data->sg[host->pio.index].length - host->pio.offset;

	/* Check to if we need less then the size of the sg_buffer */
	max = (sg_len > host->pio.len) ? host->pio.len : sg_len;
	max = max / 4;
	for(count = 0; count < max; count++) {
		while (REG_MSC_STAT(host->pdev_id) & MSC_STAT_DATA_FIFO_FULL)
				;
		REG_MSC_TXFIFO(host->pdev_id) = *wbuf++;
	}

	host->pio.len -= count;
	host->pio.offset += count;

	if (count == sg_len) {
		host->pio.index++;
		host->pio.offset = 0;
	}
}

#endif

static void jz_mmc_execute_cmd(struct jz_mmc_host *host, struct mmc_command *cmd, unsigned int cmdat)
{
	u32 timeout = 0x7fffff;
	unsigned int stat;
	int err;
	unsigned int stat_err_bits = 0;
	u32 cmdarg = 0;

	WARN_ON(host->curr.cmd != NULL);
	host->curr.cmd = cmd;

	/* mask interrupts */
	REG_MSC_IMASK(host->pdev_id) = 0xffff;

	/* clear status */
	REG_MSC_IREG(host->pdev_id) = 0xffff;

	if (cmd->flags & MMC_RSP_BUSY)
		cmdat |= MSC_CMDAT_BUSY;

	switch (RSP_TYPE(mmc_resp_type(cmd))) {
	case RSP_TYPE(MMC_RSP_R1):	// r1, r1b, r5, r6, r7
		cmdat |= MSC_CMDAT_RESPONSE_R1;
		host->curr.r_type = 1;
		break;
	case RSP_TYPE(MMC_RSP_R3):	// r3, r4
		cmdat |= MSC_CMDAT_RESPONSE_R3;
		host->curr.r_type = 1;
		break;
	case RSP_TYPE(MMC_RSP_R2):	// r2
		cmdat |= MSC_CMDAT_RESPONSE_R2;
		host->curr.r_type = 2;
		break;
	default:
		break;
	}

	REG_MSC_CMD(host->pdev_id) = cmd->opcode;

	cmdarg = cmd->arg;

	/* Set argument */
	if(host->plat->bus_width == 1) {
		if (cmd->opcode == 6) {
			/* set  1 bit sd card bus*/
			if (cmd->arg == 2) {
				cmdarg = 0;
				REG_MSC_ARG(host->pdev_id) = 0;
			}

			/* set  1 bit mmc card bus*/
			if (cmd->arg == 0x3b70101) {
				cmdarg = 0x3b70001;
				REG_MSC_ARG(host->pdev_id) = 0x3b70001;
			}
		} else
			REG_MSC_ARG(host->pdev_id) = cmd->arg;
	} else if(host->plat->bus_width == 8) {
		if (cmd->opcode == 6) {
			/* set  8 bit mmc card bus*/
			if (cmd->arg == 0x3b70101) {
				cmdarg = 0x3b70201;
				REG_MSC_ARG(host->pdev_id) = 0x3b70201;
			} else
				REG_MSC_ARG(host->pdev_id) = cmd->arg;
		} else
			REG_MSC_ARG(host->pdev_id) = cmd->arg;
	} else
		REG_MSC_ARG(host->pdev_id) = cmd->arg;

	/* Set command */
	REG_MSC_CMDAT(host->pdev_id) = cmdat;

	TRACE_CMD_REQ();

	/* Send command */
	REG_MSC_STRPCL(host->pdev_id) = MSC_STRPCL_START_OP;

	while (timeout-- && !((stat = REG_MSC_STAT(host->pdev_id)) & MSC_STAT_END_CMD_RES))
		;

	stat_err_bits = stat & MSC_STAT_ERR_BITS;

	if (timeout == 0)
		printk("wait END_CMD_RES failed!!!\n");


	REG_MSC_IREG(host->pdev_id) = MSC_IREG_END_CMD_RES;	/* clear irq flag */

	if (cmd->flags & MMC_RSP_BUSY) {
		timeout = 0x7fffff;
		while (timeout-- && !(REG_MSC_IREG(host->pdev_id) & MSC_IREG_PRG_DONE))
			;
		REG_MSC_IREG(host->pdev_id) = MSC_IREG_PRG_DONE;	/* clear status */
	} else {
		switch(cmd->opcode) { /* R1b cmds need wait PROG_DONE */
		case 7:
		case 12:
		case 28:
		case 29:
		case 38:
			timeout = 0x7fffff;
			while (timeout-- && !(REG_MSC_IREG(host->pdev_id) & MSC_IREG_PRG_DONE))
				;
			REG_MSC_IREG(host->pdev_id) = MSC_IREG_PRG_DONE;	/* clear status */
			break;
		default:
			/* do nothing */
			break;
		}
	}

	if (SD_IO_SEND_OP_COND == cmd->opcode) {
		if(host->plat->support_sdio == 0) {
			cmd->error = -ETIMEDOUT;
			jz_mmc_finish_request(host, host->curr.mrq);
			return;
		}
	}

	//timeout = 0x3fffff;
	//while ( timeout-- && ((stat = REG_MSC_STAT(host->pdev_id)) & MSC_STAT_CLK_EN))
		;
	//if (timeout == 0)
	//	printk("the clock never stopped!!!\n");
	//stat |= stat_err_bits;
	//printk("ireg = %#0x stat = %#0x timeout = %#0x\n", REG_MSC_IREG(host->pdev_id), REG_MSC_STAT(host->pdev_id), timeout);
	//printk("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
	//printk("ireg = %#0x stat = %#0x\n", REG_MSC_IREG(host->pdev_id), REG_MSC_STAT(host->pdev_id));
	//if (MMC_APP_CMD == cmd->opcode)
	//	udelay(100);
	jz_mmc_cmd_done(host, stat);

#ifdef USE_DMA
	if (host->curr.data) {
		if(host->curr.data->flags & MMC_DATA_WRITE)
			jz_mmc_data_start(host);

		/* in case that the controller never raise interrupt(May be there are some problem, isn't it?), we must finish the request here!!! */
		//err = wait_event_interruptible(host->msc_wait_queue, ((host->msc_ack) & (!host->eject)));
		err = wait_event_interruptible_timeout(host->msc_wait_queue, ((host->msc_ack) | host->eject), 2 * HZ);
		if(err == -ERESTARTSYS) {
			printk("err == -ERESTARTSYS\n");

			host->curr.mrq->cmd->error = -ENOMEDIUM;
			jz_mmc_finish_request(host, host->curr.mrq);
		}

	}
#else

	if (host->curr.data) {
		if (host->curr.data->flags & MMC_DATA_READ)
			jz_mmc_receive_pio(host);
		else
			jz_mmc_send_pio(host);
	}
#endif

}

static int jz_mmc_data_done(struct jz_mmc_host *host)
{
	struct mmc_data *data = host->curr.data;
	int stat = 0;
	unsigned int stat_err_bits = 0;
	u32 timeout = 0x7fffff;

	if (!data)
		return 0;

	stat = REG_MSC_STAT(host->pdev_id);
	stat_err_bits = stat & MSC_STAT_ERR_BITS;

	REG_MSC_IREG(host->pdev_id) = MSC_IREG_DATA_TRAN_DONE;	/* clear status */

	if (host->curr.mrq && (host->curr.mrq->data->flags & MMC_DATA_WRITE)) {

		while (timeout-- && !(REG_MSC_IREG(host->pdev_id) & MSC_IREG_PRG_DONE))
			;
		if (timeout == 0)
			printk("PRG_DONE not done!!!\n");
		REG_MSC_IREG(host->pdev_id) = MSC_IREG_PRG_DONE;	/* clear status */
	}

#if 1
	if(host->flag_cp) {

		//memcpy((unsigned long)CKSEG0ADDR(sg_dma_address(data->sg)) +
		//      data->sg->offset, host->dma_buf,
		//       host->sg_cpu[0].dcmd);

		memcpy((void *)CKSEG1ADDR(host->sg_cpu[0].dtadr), host->dma_buf, host->sg_cpu[0].dcmd);

	}
#endif

	dma_unmap_sg(mmc_dev(host->mmc), data->sg, host->dma.len,
		     host->dma.dir);

	stat = REG_MSC_STAT(host->pdev_id);
	stat |= stat_err_bits;

	if (stat & MSC_STAT_TIME_OUT_READ) {
		printk("MMC/SD/SDIO timeout, MMC_STAT 0x%x opcode = %d data flags = 0x%0x blocks = %d blksz = %d\n",
		       stat,
		       host->curr.mrq? host->curr.mrq->cmd->opcode : -1,
		       data->flags,
		       data->blocks,
		       data->blksz);
		data->error = -ETIMEDOUT;
	} else if (stat & (MSC_STAT_CRC_READ_ERROR | MSC_STAT_CRC_WRITE_ERROR)) {
		printk("jz-msc%d: MMC/SD/SDIO CRC error, MMC_STAT 0x%x, cmd=%d\n",
		       host->pdev_id, stat,
		       host->curr.mrq? host->curr.mrq->cmd->opcode : -1);
		data->error = -EILSEQ;
	}
	/*
	 * There appears to be a hardware design bug here.  There seems to
	 * be no way to find out how much data was transferred to the card.
	 * This means that if there was an error on any block, we mark all
	 * data blocks as being in error.
	 */
	if (data->error == 0)
		data->bytes_xfered = data->blocks * data->blksz;
	else
		data->bytes_xfered = 0;

	jz_mmc_disable_irq(host, MSC_IMASK_DATA_TRAN_DONE);

	host->curr.data = NULL;
	if (host->curr.mrq->stop) {
		jz_mmc_execute_cmd(host, host->curr.mrq->stop, 0);
	} else {
		jz_mmc_finish_request(host, host->curr.mrq);
	}

	return 1;
}

static void jiq_msc_irq(struct work_struct *ptr)
{
	struct jz_mmc_host *host = container_of(ptr, struct jz_mmc_host, msc_jiq_work);

	jz_mmc_data_done(host);

	host->msc_ack = 1;
	wake_up_interruptible(&host->msc_wait_queue);
}


static irqreturn_t jz_mmc_irq(int irq, void *devid)
{
	struct jz_mmc_host *host = devid;
	unsigned int ireg = 0;
	int handled = 0;

#if 1
	ireg = REG_MSC_IREG(host->pdev_id);

	if (ireg) {

		//if(host->curr.data)
		//	handled = 1;

		if (ireg & MSC_IREG_DATA_TRAN_DONE) {
			//REG_MSC_IREG(host->pdev_id) = MSC_IREG_DATA_TRAN_DONE;
			jz_mmc_disable_irq(host, MSC_IMASK_DATA_TRAN_DONE);
			//schedule_work( &(((struct jz_mmc_host *) devid)->msc_jiq_work) );
			queue_work(host->msc_work_queue, &(((struct jz_mmc_host *) devid)->msc_jiq_work));
			//host->msc_ack = 1;
			//wake_up_interruptible(&host->msc_wait_queue);
			handled = 1;
		}
	}

	return IRQ_RETVAL(handled);
#endif

#if 0
	ireg = REG_MSC_IREG(host->pdev_id);

	if (ireg) {

		if (ireg & MSC_IREG_DATA_TRAN_DONE) {
			REG_MSC_IREG(host->pdev_id) = MSC_IREG_DATA_TRAN_DONE;
			//while (!(REG_MSC_STAT(host->pdev_id) & MSC_STAT_PRG_DONE)) ;
			//	REG_MSC_IREG(host->pdev_id) = MSC_STAT_PRG_DONE;
			jz_mmc_disable_irq(host, MSC_IMASK_DATA_TRAN_DONE);

			handled |= jz_mmc_data_done(host);

			host->msc_ack = 1;
			wake_up_interruptible(&host->msc_wait_queue);
		}
	}

	return IRQ_RETVAL(handled);
#endif
}

static int jz_mmc_msc_init(struct jz_mmc_host *host)
{
	int ret = 0;

	jz_mmc_reset(host);
	// __msc_start_clk(host->pdev_id);
	REG_MSC_LPM(host->pdev_id) = 0x1;	// Low power mode
	REG_MSC_RDTO(host->pdev_id) = 0xffff;

	host->msc_ack = 0;
	init_waitqueue_head(&host->msc_wait_queue);

	msc_irq_mask_all(host->pdev_id);

	ret = request_irq(host->irqres->start, jz_mmc_irq, 0, "jz-msc (msc)", host);
	if (ret) {
		printk(KERN_ERR "MMC/SD: can't request MMC/SD IRQ\n");
		return ret;
	}

	host->msc_work_queue = create_rt_workqueue("mscworkqueue");

	INIT_WORK(&host->msc_jiq_work, jiq_msc_irq);

	return 0;
}

static void jz_mmc_msc_deinit(struct jz_mmc_host *host)
{
	free_irq(host->irqres->start, &host);
	destroy_workqueue(host->msc_work_queue);
}

int jz_mmc_msc_register(struct jz_mmc_msc *msc)
{
	if(msc == NULL)
		return -ENOMEM;

	msc->init = jz_mmc_msc_init;
	msc->deinit = jz_mmc_msc_deinit;
	msc->set_clock = jz_mmc_set_clock;
	msc->execute_cmd = jz_mmc_execute_cmd;

	return 0;
}
