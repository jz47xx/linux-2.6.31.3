/*
 *  linux/drivers/mmc/host/jz_mmc/jz_mmc_main.c - JZ SD/MMC driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Copyright (c) Ingenic Semiconductor Co., Ltd.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/mmc/host.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sd.h>
#include <linux/mmc/sdio.h>
#include <linux/mm.h>
#include <linux/signal.h>
#include <linux/pm.h>
#include <linux/scatterlist.h>
#include <asm/io.h>
#include <asm/scatterlist.h>
#include <asm/jzsoc.h>
#include <asm/jzmmc/jz_mmc_host.h>
#include <asm/jzmmc/jz_mmc_controller.h>

#define NUMBER_OF_CTRL 2

struct jz_mmc_controller controller[NUMBER_OF_CTRL];

static void jz_mmc_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct jz_mmc_host *host = mmc_priv(mmc);
	unsigned int cmdat;
	struct jz_mmc_functions *functions = host->plat->driver_data;

	cmdat = host->cmdat;
	host->cmdat &= ~MSC_CMDAT_INIT;
	host->cmdat &= ~MSC_CMDAT_STREAM_BLOCK;

	if (host->eject) {
		if (mrq->data && !(mrq->data->flags & MMC_DATA_READ)) {
			mrq->cmd->error = 0;
			mrq->data->bytes_xfered = mrq->data->blksz *
						  mrq->data->blocks;
		} else
			mrq->cmd->error = -ENOMEDIUM;

		mmc_request_done(mmc, mrq);
		return;
	}

	if (host->curr.mrq || host->curr.data || host->curr.cmd) {
		printk("warning, another request is processing!!!!\n");
	}
	host->curr.mrq = mrq;
	host->curr.data = mrq->data;
	host->curr.cmd = NULL;

	if(mrq->data) {
		cmdat &= ~MSC_CMDAT_BUSY;

		if ((mrq->cmd->opcode == 51) | (mrq->cmd->opcode == 8) | (mrq->cmd->opcode == 6)) {
			cmdat &= ~MSC_CMDAT_BUS_WIDTH_MASK;
			cmdat |= MSC_CMDAT_BUS_WIDTH_1BIT | MSC_CMDAT_DATA_EN;
		} else
			cmdat |= MSC_CMDAT_DATA_EN;
#ifdef USE_DMA
		cmdat |= MSC_CMDAT_DMA_EN;
#endif

		if (mrq->data->flags & MMC_DATA_WRITE)
			cmdat |= MSC_CMDAT_WRITE;

		if (mrq->data->flags & MMC_DATA_STREAM)
			cmdat |= MSC_CMDAT_STREAM_BLOCK;

		if (mrq->data->flags & MMC_DATA_READ)
			functions->transmit_data(host);
	}

	functions->execute_cmd(host, mrq->cmd, cmdat);
}

static int jz_mmc_get_ro(struct mmc_host *mmc)
{
	struct jz_mmc_host *host = mmc_priv(mmc);

	if(host->plat->write_protect != NULL)
		return host->plat->write_protect(mmc_dev(host->mmc));
	else
		return 0;
}

static int jz_mmc_get_cd(struct mmc_host *mmc)
{
	struct jz_mmc_host *host = mmc_priv(mmc);

	if(host->plat->status != NULL) {
		return host->plat->status(mmc_dev(host->mmc));
	}
	else
		return 1;
}

/* set clock and power */
static void jz_mmc_set_ios(struct mmc_host *mmc, struct mmc_ios *ios)
{
	struct jz_mmc_host *host = mmc_priv(mmc);
	struct jz_mmc_functions *functions = host->plat->driver_data;
	void *dev;

	if(!functions) {
//		printk("%s: functions is NULL!\n", __FUNCTION__);
		while(1);
	}

	if (ios->clock) {
		functions->set_clock(host, ios->clock);
	}

	switch(ios->power_mode) {
	case MMC_POWER_ON:
		host->plat->power_on((struct device *)dev);
		host->cmdat |= CMDAT_INIT;
		break;
	case MMC_POWER_OFF:
		host->plat->power_off((struct device *)dev);
		break;
	default:
		break;
	}

	if (ios->bus_width == MMC_BUS_WIDTH_4) {

		host->cmdat &= ~MSC_CMDAT_BUS_WIDTH_MASK;

		if(host->plat->bus_width == 4)
			host->cmdat |= MSC_CMDAT_BUS_WIDTH_4BIT;
		else
			host->cmdat |= host->plat->bus_width;
	} else if (ios->bus_width == MMC_BUS_WIDTH_8) {

		host->cmdat &= ~MSC_CMDAT_BUS_WIDTH_MASK;

		if(host->plat->bus_width == 8)
			host->cmdat |= MSC_CMDAT_BUS_WIDTH_8BIT;
//		else
//			host->cmdat |= host->plat->bus_width;
	} else {
		/* 1 bit bus*/
		host->cmdat &= ~MSC_CMDAT_BUS_WIDTH_8BIT;
	}
}

static const struct mmc_host_ops jz_mmc_ops = {
	.request = jz_mmc_request,
	.get_ro = jz_mmc_get_ro,
	.set_ios = jz_mmc_set_ios,
	.get_cd = jz_mmc_get_cd,
};

static int jz_mmc_probe(struct platform_device *pdev)
{
	struct jz_mmc_platform_data *plat = pdev->dev.platform_data;
	struct mmc_host *mmc;
	struct jz_mmc_host *host = NULL;
//	struct jz_mmc_controller controller;
	struct jz_mmc_functions *functions;

	struct resource *irqres = NULL;
	struct resource *memres = NULL;
	struct resource *dmares = NULL;
	int i;
	int ret = 0;

	if (pdev == NULL) {
		printk(KERN_ERR "%s: pdev is NULL\n", __func__);
		return -EINVAL;
	}
	if (!plat) {
		printk(KERN_ERR "%s: Platform data not available\n", __func__);
		return -EINVAL;
	}



	if (pdev->id < 0 || pdev->id > 1)
		return -EINVAL;

	plat->cpm_start(&pdev->dev);

	// IORESOURCE_DMA is NOT required
	if (pdev->resource == NULL || pdev->num_resources < 2) {
		printk(KERN_ERR "%s: Invalid resource\n", __func__);
		return -ENXIO;
	}
	for (i = 0; i < pdev->num_resources; i++) {
		if (pdev->resource[i].flags & IORESOURCE_MEM)
			memres = &pdev->resource[i];
		if (pdev->resource[i].flags & IORESOURCE_IRQ)
			irqres = &pdev->resource[i];
		if (pdev->resource[i].flags & IORESOURCE_DMA)
			dmares = &pdev->resource[i];
	}
	if (!irqres || !memres) {
		printk(KERN_ERR "%s: Invalid resource\n", __func__);
		return -ENXIO;
	}
	/*
	 * Setup our host structure
	 */
	mmc = mmc_alloc_host(sizeof(struct jz_mmc_host), &pdev->dev);
	if (!mmc) {
		return -ENOMEM;
	}
	host = mmc_priv(mmc);
	host->pdev_id = pdev->id;
	host->plat = plat;
	host->mmc = mmc;
	// base address of MSC controller
	host->base = ioremap(memres->start, PAGE_SIZE);
	if (!host->base) {
		return -ENOMEM;
	}
	// back up these info. for future using
	host->irqres = irqres;
	host->memres = memres;
	host->dmares = dmares;
	host->imask = 0xffff;
	host->sg_cpu =
		dma_alloc_coherent(&pdev->dev, PAGE_SIZE, &host->sg_dma,
				   GFP_KERNEL);
	if (!host->sg_cpu) {
		ret = -ENOMEM;
		goto out;
	}
	spin_lock_init(&host->lock);

	/*
	 * Setup MMC host structure
	 */
	mmc->ops = &jz_mmc_ops;
	mmc->f_min = MMC_CLOCK_SLOW;
	mmc->f_max = SD_CLOCK_HIGH;
	mmc->ocr_avail = plat->ocr_mask;
	mmc->caps |= host->plat->max_bus_width;
	mmc->max_phys_segs = NR_SG;
	mmc->max_blk_size = 4095;
	mmc->max_blk_count = 65535;

	mmc->max_req_size = PAGE_SIZE * 16;
	mmc->max_seg_size = mmc->max_req_size;
	plat->init(&pdev->dev);
	plat->power_on(&pdev->dev);
	/*
	 * Initialize controller and register some functions
	 * From here, we can do everything!
	 */
	controller_register(&controller[host->pdev_id], host);
	functions = host->plat->driver_data;

	if(controller[host->pdev_id].init(&controller[host->pdev_id], host, pdev))
		goto out;

//	printk("%s: functions->set_clock = %x  jz_mmc_set_clock = %x\n", __FUNCTION__, functions->set_clock, jz_mmc_set_clock);
	mmc_set_drvdata(pdev, mmc);
	mmc_add_host(mmc);

	printk("JZ %s driver registered\n", pdev->name);

	return 0;

out:
	if (host->sg_cpu)
		dma_free_coherent(&pdev->dev, PAGE_SIZE,
				  host->sg_cpu, host->sg_dma);
	return -1;
}

static int jz_mmc_remove(struct platform_device *pdev)
{
	struct mmc_host *mmc = platform_get_drvdata(pdev);
	struct jz_mmc_platform_data *plat = pdev->dev.platform_data;

	platform_set_drvdata(pdev, NULL);

	if (mmc) {
		struct jz_mmc_host *host = mmc_priv(mmc);
		struct jz_mmc_functions *functions = host->plat->driver_data;

		plat->power_off(&pdev->dev);

		functions->deinit(host, pdev);

		mmc_remove_host(mmc);
		mmc_free_host(mmc);
	}
	return 0;
}

static int jz_mmc_suspend(struct platform_device *dev, pm_message_t state)
{
	struct mmc_host *mmc = platform_get_drvdata(dev);
	struct jz_mmc_host *host = mmc_priv(mmc);
	int ret = 0;

	printk("enter jz_mmc_suspend......\n");
	if (mmc) {

		if (host->plat->detect_pin)
			enable_irq_wake(host->plat->detect_pin);

		if (mmc->card && mmc->card->type != MMC_TYPE_SDIO) {
			ret = mmc_suspend_host(mmc, state);
		}

	}
	return ret;
}

static int jz_mmc_resume(struct platform_device *dev)
{
	struct mmc_host *mmc = platform_get_drvdata(dev);
//	struct jz_mmc_host *host = mmc_priv(mmc);
	int ret = 0;

	printk("enter jz_mmc_resume......\n");
	if (mmc) {
		if (mmc->card && mmc->card->type != MMC_TYPE_SDIO) {
			ret = mmc_resume_host(mmc);
			//disable_irq_wake(host->plat->detect_pin);
		}

	}

	return ret;
}

static struct platform_driver jz_mmc0_driver = {
	.probe = jz_mmc_probe,
	.remove = jz_mmc_remove,
	.suspend = jz_mmc_suspend,
	.resume = jz_mmc_resume,
	.driver = {
		   .name = "jz-msc0",
		   },
};

static struct platform_driver jz_mmc1_driver = {
	.probe = jz_mmc_probe,
	.remove = jz_mmc_remove,
	.suspend = jz_mmc_suspend,
	.resume = jz_mmc_resume,
	.driver = {
		   .name = "jz-msc1",
		   },
};

static int __init jz_mmc_init(void)
{
	int ret = 0;
	printk("here!!!!!jz_mmc_init......\n");
	ret = platform_driver_register(&jz_mmc0_driver);
	ret = platform_driver_register(&jz_mmc1_driver);

	return ret;
}

static void __exit jz_mmc_exit(void)
{
	platform_driver_unregister(&jz_mmc1_driver);
	platform_driver_unregister(&jz_mmc0_driver);
}

module_init(jz_mmc_init);
module_exit(jz_mmc_exit);

MODULE_DESCRIPTION("JZ47XX SD/Multimedia Card Interface Driver");
MODULE_LICENSE("GPL");
