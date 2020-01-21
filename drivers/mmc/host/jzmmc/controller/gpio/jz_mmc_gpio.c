/*
 *  linux/drivers/mmc/host/jz_mmc/gpio/jz_mmc_gpio.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Copyright (c) Ingenic Semiconductor Co., Ltd.
 */

#include <asm/jzmmc/jz_mmc_gpio.h>

#define	TRY_TIME	10
#define	RETRY_TIME	50
//#define CARD_REINSERT_DURATTION	(5 * HZ)
//static unsigned long msmsdcc_irqtime = 0;

#if 0
static irqreturn_t jz_mmc_detect_irq(int irq, void *devid)
{
	struct jz_mmc_host *host = (struct jz_mmc_host *) devid;

	printk("jz-mmc: card detect interrupt raised!!! check status......\n");

	if (!host->plat->status) {
		mmc_detect_change(host->mmc, 50);
		return IRQ_HANDLED;
	}

	if (host->plat->status(mmc_dev(host->mmc))) { /* card detected */
		host->eject = 0;
		host->plat->plug_change(CARD_INSERTED);
	} else {
		host->eject = 1;
		host->plat->plug_change(CARD_REMOVED);
	}

	if (host->eject ^ host->oldstat) {
		mmc_detect_change(host->mmc, 50);
	}

	host->oldstat = host->eject;

	return IRQ_HANDLED;
}
#else
static void jiq_de_quiver(struct work_struct *ptr){
	struct jz_mmc_host *host = container_of(ptr, struct jz_mmc_host, gpio_jiq_work);
	unsigned int time_to_try, i, tmp, counter = 0, result = 1;
	unsigned long duration;

	if (!host->plat->status) {
		mmc_detect_change(host->mmc, 0);
		return;
	}

	for (time_to_try = 0; time_to_try < RETRY_TIME; time_to_try++) {
		for (i = 0; i < TRY_TIME; i++) {
			tmp = (!host->plat->status(mmc_dev(host->mmc))); // tmp = 1 means slot is empty
			result &= tmp;
			if( !tmp )
				counter++;
			schedule_timeout((10*HZ)/1000);
		}

		if ( !result ) {
			// The card is there
			if (counter == TRY_TIME) {
				printk("Stable the card is there\n");
				host->eject = 0;
				/* wait for card removal */
				host->plat->plug_change(CARD_INSERTED);
				enable_irq(host->plat->status_irq);

				goto stable;
			}
			/* try again, goto for */
			counter = 0;
			result = 1;
		} else {
			printk("Stable the slot is empty\n");
			host->eject = 1;
			/* wait for card insertion */
			host->plat->plug_change(CARD_REMOVED);
			enable_irq(host->plat->status_irq);
			goto stable;
		}
	}

stable:
	if (host->eject ^ host->oldstat) {
		/* Delay the process for Card re-insert */
		//duration = jiffies - msmsdcc_irqtime;

		//if ((!(host->eject)) && (duration < CARD_REINSERT_DURATTION)) {
		//	duration = CARD_REINSERT_DURATTION - duration;
		//} else {
		//	duration = 0;
		//}

		mmc_detect_change(host->mmc, 50);
		//msmsdcc_irqtime = jiffies;
		if (host->eject)
			wake_up_interruptible(&host->msc_wait_queue);
	}

	host->oldstat = host->eject;
}

static irqreturn_t jz_mmc_detect_irq(int irq, void *devid)
{
	struct jz_mmc_host *host = (struct jz_mmc_host *) devid;

	printk("jz-msc%d: detect card......\n", host->pdev_id);
	disable_irq_nosync(host->plat->status_irq);

	schedule_work( &(((struct jz_mmc_host *) devid)->gpio_jiq_work) );

	return IRQ_HANDLED;
}
#endif

static int jz_mmc_gpio_init(struct jz_mmc_host *host, struct platform_device *pdev)
{
	int ret = 0;

	/*
	 * Setup card detect change
	 */
	if (host->plat->status_irq) {
		ret = request_irq(host->plat->status_irq,
				  jz_mmc_detect_irq,
				  0,
				  "jz-msc (gpio)",
				  host);
		if (ret) {
			printk(KERN_ERR "Unable to get slot IRQ %d (%d)\n",
			       host->plat->status_irq, ret);
			return ret;
		}

		device_init_wakeup(&pdev->dev, 1);

#if 1
		INIT_WORK(&(host->gpio_jiq_work), jiq_de_quiver);
#endif

		// Check if there were any card present
		if (host->plat->status) {
			host->eject = !(host->plat->status(mmc_dev(host->mmc)));
			host->oldstat = host->eject;

			if(host->eject) {
				host->plat->plug_change(CARD_REMOVED);
			} else {
				host->plat->plug_change(CARD_INSERTED);
			}
		}
	} else
		printk(KERN_ERR "%s: No card detect facilities available\n",
		       mmc_hostname(host->mmc));

	return 0;
}

static void jz_mmc_gpio_deinit(struct jz_mmc_host *host, struct platform_device *pdev)
{
	if(host->plat->status_irq) {
		free_irq(host->plat->status_irq, &host);
		device_init_wakeup(&pdev->dev, 0);
	}
}

int jz_mmc_gpio_register(struct jz_mmc_gpio *gpio)
{
	if(gpio == NULL)
		return -ENOMEM;

	gpio->init = jz_mmc_gpio_init;
	gpio->deinit = jz_mmc_gpio_deinit;

	return 0;
}
