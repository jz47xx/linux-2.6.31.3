/*
 *  arch/mips/include/asm/mach-jz4750/jz_platform_data.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Copyright (c) Ingenic Semiconductor Co., Ltd.
 */

#ifndef __JZ_PLATFORM_DATA_H__
#define __JZ_PLATFORM_DATA_H__

#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/mmc/sdio_func.h>

#define CARD_INSERTED 1
#define CARD_REMOVED 0

struct jz_mmc_platform_data {
	unsigned int ocr_mask;			/* available voltages */
	unsigned long detect_delay;		/* delay in jiffies before detecting cards after interrupt */
	unsigned char status_irq;
	unsigned char support_sdio;
	unsigned char bus_width;
	unsigned int max_bus_width;
	unsigned int detect_pin;

	unsigned char msc_irq;
	unsigned char dma_rxid;
	unsigned char dma_txid;

	void *driver_data;

	void (*init) (struct device *);
	void (*power_on) (struct device *);
	void (*power_off) (struct device *);
	void (*cpm_start) (struct device *);
	unsigned int (*status) (struct device *);
	unsigned int (*write_protect) (struct device *);
	void (*plug_change) (int);
	int (*register_status_notify)(void (*callback)(int card_present, void *dev_id), void *dev_id);


/*
	int (*init)(struct device *, irq_handler_t , void *);
	int (*get_ro)(struct device *);
	void (*setpower)(struct device *, unsigned int);
	void (*exit)(struct device *, void *);
*/


};

#endif
