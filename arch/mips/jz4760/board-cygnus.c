/*
 * linux/arch/mips/jz4760/board-cygnus.c
 *
 * JZ4760 Cygnus board setup routines.
 *
 * Copyright (c) 2006-2010  Ingenic Semiconductor Inc.
 * Author: <jlwei@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/sched.h>
#include <linux/ioport.h>
//#include <linux/mm.h>
#include <linux/console.h>
#include <linux/delay.h>

#include <asm/cpu.h>
#include <asm/bootinfo.h>
#include <asm/mipsregs.h>
#include <asm/reboot.h>

#include <asm/jzsoc.h>

extern void (*jz_timer_callback)(void);
//extern int __init jz_add_msc_devices(unsigned int controller, struct jz_mmc_platform_data *plat);

void __init board_msc_init(void);

#undef DEBUG
//#define DEBUG
#ifdef DEBUG
#define dprintk(x...)	printk(x)
#else
#define dprintk(x...)
#endif

/*
 * __gpio_as_sleep set all pins to pull-disable, and set all pins as input
 * except sdram and the pins which can be used as CS1_N to CS4_N for chip select.
 */
#define __gpio_as_sleep()	              \
do {	                                      \
	REG_GPIO_PXFUNC(1) = ~0x03ff7fff;     \
	REG_GPIO_PXSELC(1) = ~0x03ff7fff;     \
	REG_GPIO_PXDIRC(1) = ~0x03ff7fff;     \
	REG_GPIO_PXPES(1)  =  0xffffffff;     \
	REG_GPIO_PXFUNC(2) = ~0x01e00000;     \
	REG_GPIO_PXSELC(2) = ~0x01e00000;     \
	REG_GPIO_PXDIRC(2) = ~0x01e00000;     \
	REG_GPIO_PXPES(2)  =  0xffffffff;     \
	REG_GPIO_PXFUNC(3) =  0xffffffff;     \
	REG_GPIO_PXSELC(3) =  0xffffffff;     \
	REG_GPIO_PXDIRC(3) =  0xffffffff;     \
	REG_GPIO_PXPES(3)  =  0xffffffff;     \
	REG_GPIO_PXFUNC(4) =  0xffffffff;     \
	REG_GPIO_PXSELC(4) =  0xffffffff;     \
	REG_GPIO_PXDIRC(4) =  0xffffffff;     \
	REG_GPIO_PXPES(4)  =  0xffffffff;     \
	REG_GPIO_PXFUNC(5) =  0xffffffff;     \
	REG_GPIO_PXSELC(5) =  0xffffffff;     \
	REG_GPIO_PXDIRC(5) =  0xffffffff;     \
	REG_GPIO_PXPES(5)  =  0xffffffff;     \
} while (0)

struct wakeup_key_s {
	int gpio;       /* gpio pin number */
	int active_low; /* the key interrupt pin is low voltage
                           or fall edge acitve */
};

/* add wakeup keys here */
static struct wakeup_key_s wakeup_key[] = {
    {
		.gpio = KEY_C0,
		.active_low = 1,
	},
	{
		.gpio = KEY_C1,
		.active_low = 1,
	},
    {
		.gpio = KEY_C2,
		.active_low = 1,
	},
	{
		.gpio = KEY_C3,
		.active_low = 1,
	},
	{
		.gpio = KEY_C4,
		.active_low = 1,
	},
#if defined(CONFIG_GSM_IW368)
	{
		.gpio = GPIO_GSM_RI,
		.active_low = 1,
	},
#endif
};

static void wakeup_key_setup(void)
{
	int i;
	int num = sizeof(wakeup_key) / sizeof(wakeup_key[0]);

	for(i = 0; i < num; i++) {
#if 1
		if(wakeup_key[i].active_low)
			__gpio_as_irq_fall_edge(wakeup_key[i].gpio);
		else
			__gpio_as_irq_rise_edge(wakeup_key[i].gpio);
#endif
		__gpio_ack_irq(wakeup_key[i].gpio);
		__gpio_unmask_irq(wakeup_key[i].gpio);
		__intc_unmask_irq(IRQ_GPIO0 - (wakeup_key[i].gpio/32));  /* unmask IRQ_GPIOn */
	}
}


/* NOTES:
 * 1: Pins that are floated (NC) should be set as input and pull-enable.
 * 2: Pins that are pull-up or pull-down by outside should be set as input
 *    and pull-disable.
 * 3: Pins that are connected to a chip except sdram and nand flash
 *    should be set as input and pull-disable, too.
 */
void jz_board_do_sleep(unsigned long *ptr)
{
	unsigned char i;

        /* Print messages of GPIO registers for debug */
	for(i=0;i<GPIO_PORT_NUM;i++) {
		dprintk("run dat:%x pin:%x fun:%x sel:%x dir:%x pull:%x msk:%x trg:%x\n",        \
			REG_GPIO_PXDAT(i),REG_GPIO_PXPIN(i),REG_GPIO_PXFUN(i),REG_GPIO_PXSEL(i), \
			REG_GPIO_PXDIR(i),REG_GPIO_PXPE(i),REG_GPIO_PXIM(i),REG_GPIO_PXTRG(i));
	}

        /* Save GPIO registers */
	for(i = 1; i < GPIO_PORT_NUM; i++) {
		*ptr++ = REG_GPIO_PXFUN(i);
		*ptr++ = REG_GPIO_PXSEL(i);
		*ptr++ = REG_GPIO_PXDIR(i);
		*ptr++ = REG_GPIO_PXPE(i);
		*ptr++ = REG_GPIO_PXIM(i);
		*ptr++ = REG_GPIO_PXDAT(i);
		*ptr++ = REG_GPIO_PXTRG(i);
	}

        /*
         * Set all pins to pull-disable, and set all pins as input except
         * sdram and the pins which can be used as CS1_N to CS4_N for chip select.
         */
	//       __gpio_as_sleep();

        /*
	 * Set proper status for GPC21 to GPC24 which can be used as CS1_N to CS4_N.
	 * Keep the pins' function used for chip select(CS) here according to your
         * system to avoid chip select crashing with sdram when resuming from sleep mode.
         */

#if defined(CONFIG_JZ4760_APUS)
        /* GPB25/CS1_N is used as chip select for nand flash, shouldn't be change. */

        /* GPB26/CS2_N is connected to nand flash, needn't be changed. */

        /* GPB28/CS3_N is used as cs8900's chip select, shouldn't be changed. */

	/* GPB27/CS4_N is used as NOR's chip select, shouldn't be changed. */
#endif

	/*
         * Enable pull for NC pins here according to your system
	 */

#if defined(CONFIG_JZ4760_APUS)
#endif

	/*
         * If you must set some GPIOs as output to high level or low level,
         * you can set them here, using:
         * __gpio_as_output(n);
         * __gpio_set_pin(n); or  __gpio_clear_pin(n);
	 */

#if defined(CONFIG_JZ4760_APUS)
	/* GPC7 which is used as AMPEN_N should be set to high to disable audio amplifier */
	__gpio_as_output(32*2+7);
	__gpio_set_pin(32*2+7);
#endif

#ifdef DEBUG
        /* Keep uart function for printing debug message */
	__gpio_as_uart0();
	__gpio_as_uart1();
	__gpio_as_uart2();
	__gpio_as_uart3();

        /* Print messages of GPIO registers for debug */
	for(i=0;i<GPIO_PORT_NUM;i++) {
		dprintk("sleep dat:%x pin:%x fun:%x sel:%x dir:%x pull:%x msk:%x trg:%x\n",      \
			REG_GPIO_PXDAT(i),REG_GPIO_PXPIN(i),REG_GPIO_PXFUN(i),REG_GPIO_PXSEL(i), \
			REG_GPIO_PXDIR(i),REG_GPIO_PXPE(i),REG_GPIO_PXIM(i),REG_GPIO_PXTRG(i));
	}
#endif
	wakeup_key_setup();
}

void jz_board_do_resume(unsigned long *ptr)
{
	unsigned char i;

	/* Restore GPIO registers */
	for(i = 1; i < GPIO_PORT_NUM; i++) {
		 REG_GPIO_PXFUNS(i) = *ptr;
		 REG_GPIO_PXFUNC(i) = ~(*ptr++);

		 REG_GPIO_PXSELS(i) = *ptr;
		 REG_GPIO_PXSELC(i) = ~(*ptr++);

		 REG_GPIO_PXDIRS(i) = *ptr;
		 REG_GPIO_PXDIRC(i) = ~(*ptr++);

		 REG_GPIO_PXPES(i) = *ptr;
		 REG_GPIO_PXPEC(i) = ~(*ptr++);

		 REG_GPIO_PXIMS(i)=*ptr;
		 REG_GPIO_PXIMC(i)=~(*ptr++);

		 REG_GPIO_PXDATS(i)=*ptr;
		 REG_GPIO_PXDATC(i)=~(*ptr++);

		 REG_GPIO_PXTRGS(i)=*ptr;
		 REG_GPIO_PXTRGC(i)=~(*ptr++);
	}

        /* Print messages of GPIO registers for debug */
	for(i=0;i<GPIO_PORT_NUM;i++) {
		dprintk("resume dat:%x pin:%x fun:%x sel:%x dir:%x pull:%x msk:%x trg:%x\n",     \
			REG_GPIO_PXDAT(i),REG_GPIO_PXPIN(i),REG_GPIO_PXFUN(i),REG_GPIO_PXSEL(i), \
			REG_GPIO_PXDIR(i),REG_GPIO_PXPE(i),REG_GPIO_PXIM(i),REG_GPIO_PXTRG(i));
	}
}

#if 0
static void dancing(void)
{
	static unsigned char slash[] = "\\|/-";
//	static volatile unsigned char *p = (unsigned char *)0xb6000058;
	static volatile unsigned char *p = (unsigned char *)0xb6000016;
	static unsigned int count = 0;
	*p = slash[count++];
	count &= 3;
}
#endif

/* MSC SETUP */
static void cygnus_sd_8bit_gpio_init(struct device *dev)
{
	__gpio_as_msc0_8bit();
	__gpio_as_output(GPIO_SD0_VCC_EN_N);
	__gpio_as_input(GPIO_SD0_CD_N);
}

static void cygnus_sd_8bit_power_on(struct device *dev)
{
	__gpio_clear_pin(GPIO_SD0_VCC_EN_N);
}

static void cygnus_sd_8bit_power_off(struct device *dev)
{
	__gpio_set_pin(GPIO_SD0_VCC_EN_N);
}

/*
static unsigned int cygnus_sd_8bit_status(struct device *dev)
{
	unsigned int status;

	status = (unsigned int) __gpio_get_pin(GPIO_SD0_CD_N);
	return (!status);
}
*/

/*
static void cygnus_sd_8bit_plug_change(int state)
{
	if(state == CARD_INSERTED)
		__gpio_as_irq_high_level(MSC0_HOTPLUG_PIN);
	else
		__gpio_as_irq_low_level(MSC0_HOTPLUG_PIN);
}
*/

static unsigned int cygnus_sd_8bit_get_wp(struct device *dev)
{
	unsigned int status;

	status = (unsigned int) __gpio_get_pin(GPIO_SD0_WP_N);
	return (status);
}

/*
struct jz_mmc_platform_data cygnus_sd_8bit_data = {
#ifndef CONFIG_JZ_MSC0_SDIO_SUPPORT
	.support_sdio   = 0,
#else
	.support_sdio   = 1,
#endif
	.ocr_mask	= MMC_VDD_32_33 | MMC_VDD_33_34,
	.status_irq	= MSC0_HOTPLUG_IRQ,
	.detect_pin     = GPIO_SD0_CD_N,
	.init           = cygnus_sd_8bit_gpio_init,
	.power_on       = cygnus_sd_8bit_power_on,
	.power_off      = cygnus_sd_8bit_power_off,
	.status		= cygnus_sd_8bit_status,
	.plug_change	= cygnus_sd_8bit_plug_change,
	.write_protect  = cygnus_sd_8bit_get_wp,
	.max_bus_width  = MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED | MMC_CAP_8_BIT_DATA,
#ifdef CONFIG_JZ_MSC0_BUS_1
	.bus_width      = 1,
#elif defined  CONFIG_JZ_MSC0_BUS_4
	.bus_width      = 4,
#else
	.bus_width      = 8,
#endif
};
*/

/*
static void cygnus_sd_4bit_gpio_init(struct device *dev)
{
	__gpio_as_msc1_4bit();
	__gpio_as_output(GPIO_SD1_VCC_EN_N);
	__gpio_as_input(GPIO_SD1_CD_N);
}

static void cygnus_sd_4bit_power_on(struct device *dev)
{
	__msc1_enable_power();
}

static void cygnus_sd_4bit_power_off(struct device *dev)
{
	__msc1_disable_power();
}

static unsigned int cygnus_sd_4bit_status(struct device *dev)
{
	unsigned int status;

	status = (unsigned int) __gpio_get_pin(GPIO_SD1_CD_N);
	return (status);
}

static void cygnus_sd_4bit_plug_change(int state)
{
	if(state == CARD_INSERTED)
		__gpio_as_irq_low_level(MSC1_HOTPLUG_PIN);
	else
		__gpio_as_irq_high_level(MSC1_HOTPLUG_PIN);
}

static unsigned int cygnus_sd_4bit_get_wp(struct device *dev)
{
	unsigned int status;

	status = (unsigned int) __gpio_get_pin(GPIO_SD1_WP_N);
	return (status);
}

struct jz_mmc_platform_data cygnus_sd_4bit_data = {
#ifndef CONFIG_JZ_MSC1_SDIO_SUPPORT
	.support_sdio   = 0,
#else
	.support_sdio   = 1,
#endif
	.ocr_mask	= MMC_VDD_32_33 | MMC_VDD_33_34,
	.status_irq	= MSC1_HOTPLUG_IRQ,
	.detect_pin     = GPIO_SD1_CD_N,
	.init           = cygnus_sd_4bit_gpio_init,
	.power_on       = cygnus_sd_4bit_power_on,
	.power_off      = cygnus_sd_4bit_power_off,
	.status		= cygnus_sd_4bit_status,
	.plug_change	= cygnus_sd_4bit_plug_change,
	.write_protect  = cygnus_sd_4bit_get_wp,
	.max_bus_width  = MMC_CAP_SD_HIGHSPEED | MMC_CAP_4_BIT_DATA,
#ifdef CONFIG_JZ_MSC1_BUS_1
	.bus_width      = 1,
#else
	.bus_width      = 4,
#endif
};

void __init board_msc_init(void)
{
#ifdef CONFIG_JZ_MSC0
	jz_add_msc_devices(0, &cygnus_sd_8bit_data);
#endif

#ifdef CONFIG_JZ_MSC1
	jz_add_msc_devices(1, &cygnus_sd_4bit_data);
#endif
}
*/

static void f4760_timer_callback(void)
{
	static unsigned long count = 0;

	if ((++count) % 50 == 0) {
//		dancing();
		count = 0;
	}
}

static void __init board_cpm_setup(void)
{
	/* Stop unused module clocks here.
	 * We have started all module clocks at arch/mips/jz4760/setup.c.
	 */
}

static void __init board_gpio_setup(void)
{
	/*
	 * Initialize SDRAM pins
	 */
}

void __init jz_board_setup(void)
{
	printk("JZ4760 Cygnus board setup\n");
//	jz_restart(NULL);
	board_cpm_setup();
	board_gpio_setup();

	jz_timer_callback = f4760_timer_callback;
}

/**
 * Called by arch/mips/kernel/proc.c when 'cat /proc/cpuinfo'.
 * Android requires the 'Hardware:' field in cpuinfo to setup the init.%hardware%.rc.
 */
const char *get_board_type(void)
{
	return "Cygnus";
}
