/*
 * linux/drivers/char/jzchar/poweroff.c
 *
 * Power off handling.
 *
 * Copyright (C) 2005-2007  Ingenic Semiconductor Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * Porting to Linux-2.6.31.3,
 * Use platform suspend/resume PM API.
 * - River <zwang@ingenic.cn>
 */

#include <linux/module.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/major.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/spinlock.h>
#include <linux/pm.h>
#include <linux/platform_device.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/jzsoc.h>

#include "jzchars.h"

MODULE_AUTHOR("Jianli Wei <jlwei@ingenic.cn>");
MODULE_DESCRIPTION("Poweroff handling");
MODULE_LICENSE("GPL");

#undef DEBUG
//#define DEBUG
#ifdef DEBUG
#define dprintk(x...)	printk(x)
#else
#define dprintk(x...)
#endif

//#define USE_SUSPEND_HOTPLUG

#ifdef CONFIG_SOC_JZ4730
#define GPIO_PW_I 97
#define GPIO_PW_O 66
#define POWEROFF_PIN_DOWN 1
#define SET_POWEROFF_PIN_AS_IRQ __gpio_as_irq_rise_edge(POWEROFF_PIN)
#define DO_SHUTDOWN_SYSTEM __gpio_clear_pin(GPIO_PW_O)
#define DO_SUSPEND jz_pm_suspend()

#define GPIO_DISP_OFF_N   93
#define __lcd_set_backlight_level(n) \
do { \
	REG_PWM_DUT(0) = n; \
	REG_PWM_PER(0) = 7; \
	REG_PWM_CTR(0) = 0x81; \
} while (0)
#define __lcd_close_backlight() \
do { \
	__lcd_set_backlight_level(0); \
} while (0)
#endif

#ifdef CONFIG_SOC_JZ4740
#define GPIO_PW_I 125
#define POWEROFF_PIN_DOWN 0
#define SET_POWEROFF_PIN_AS_IRQ __gpio_as_irq_fall_edge(POWEROFF_PIN)
#define DO_SHUTDOWN_SYSTEM jz_pm_hibernate()
#define DO_SUSPEND { \
	    jz_pm_sleep();\
	    suspend_flag = 0;\
	    SET_POWEROFF_PIN_AS_IRQ;\
	}

#define GPIO_DISP_OFF_N		118
#define GPIO_PWM    123
#define __lcd_close_backlight() \
do { \
__gpio_as_output(GPIO_PWM); \
__gpio_clear_pin(GPIO_PWM); \
} while (0)
#endif

#ifdef CONFIG_SOC_JZ4750
#define GPIO_PW_I               GPIO_WAKEUP
#define POWEROFF_PIN_DOWN       0
#define SET_POWEROFF_PIN_AS_IRQ __gpio_as_irq_fall_edge(POWEROFF_PIN)
#define DO_SHUTDOWN_SYSTEM      jz_pm_hibernate()
#define DO_SUSPEND { \
	    jz_pm_sleep();\
	    suspend_flag = 0;\
	    SET_POWEROFF_PIN_AS_IRQ;\
	}
#endif

#ifdef CONFIG_SOC_JZ4750D
#define GPIO_PW_I               GPIO_WAKEUP
#define POWEROFF_PIN_DOWN       0
#define SET_POWEROFF_PIN_AS_IRQ __gpio_as_irq_fall_edge(POWEROFF_PIN)
#define DO_SHUTDOWN_SYSTEM      jz_pm_hibernate()
#define DO_SUSPEND { \
	    jz_pm_sleep();\
	    suspend_flag = 0;\
	    SET_POWEROFF_PIN_AS_IRQ;\
	}
#endif


#define POWEROFF_PIN   GPIO_PW_I
#define POWEROFF_IRQ   (IRQ_GPIO_0 + POWEROFF_PIN)

#define POWEROFF_PERIOD         1000    /* unit: ms */
#define POWEROFF_DELAY          100     /* unit: ms */

static struct timer_list poweroff_timer;
static struct timer_list poweroff_delaytimer;
static struct work_struct suspend_work;

static int poweroff_flag = 0;
static int suspend_flag = 0;
static int num_seconds = 0;

#ifdef CONFIG_JZ_UDC_HOTPLUG
extern int jz_udc_active;
#endif

extern void jz_pm_suspend(void);
extern int jz_pm_hibernate(void);
extern int jz_pm_sleep(void);

static void poweroff_timer_routine(unsigned long dummy)
{
	if (__gpio_get_pin(POWEROFF_PIN) == POWEROFF_PIN_DOWN) {
		if (++num_seconds > 3)
		{
			printk("\nShutdown system now ..\n");

#ifndef USE_SUSPEND_HOTPLUG
                        /* Turn off LCD to inform user that the system is shutting down.
                         * But the information of shutting down system will be shown
                         * by userspace program if hotplug is used.
                         */
			__lcd_close_backlight();
#endif

                        /*
                         * Wait until the power key is up, or the system will reset with
                         * power key down after entering hibernate.
                         */
			while(__gpio_get_pin(POWEROFF_PIN)==POWEROFF_PIN_DOWN);

			poweroff_flag = 1;
			schedule_work(&suspend_work); /* inform user to poweroff */
		}
		else {
			del_timer(&poweroff_timer);
			init_timer(&poweroff_timer);
			poweroff_timer.expires = jiffies + POWEROFF_PERIOD/10;
			poweroff_timer.data = 0;
			poweroff_timer.function = poweroff_timer_routine;
			add_timer(&poweroff_timer);
		}
	}
	else
	{
		printk("\nSuspend system now ..\n");
		num_seconds = 0;
		suspend_flag = 1;
		poweroff_flag = 0;
		schedule_work(&suspend_work); /* we are entering suspend */
	}
}

static void poweroff_delaytimer_routine(unsigned long dummy)
{
	__gpio_as_input(POWEROFF_PIN);
	if (__gpio_get_pin(POWEROFF_PIN)==POWEROFF_PIN_DOWN) {
		if (suspend_flag) {
			suspend_flag = 0;
			del_timer(&poweroff_delaytimer);
			SET_POWEROFF_PIN_AS_IRQ;
			__gpio_unmask_irq(POWEROFF_PIN);
			return;
		}
		del_timer(&poweroff_delaytimer);
		del_timer(&poweroff_timer);
		init_timer(&poweroff_timer);
		poweroff_timer.expires = jiffies + POWEROFF_PERIOD/100;
		poweroff_timer.data = 0;
		poweroff_timer.function = poweroff_timer_routine;
		add_timer(&poweroff_timer);
	}
	else {
		del_timer(&poweroff_delaytimer);
		SET_POWEROFF_PIN_AS_IRQ;
		__gpio_unmask_irq(POWEROFF_PIN);

		printk("This is a dummy key\n");
	}
}

/*
 * Poweroff pin interrupt handler
 */
static irqreturn_t poweroff_irq(int irq, void *dev_id)
{
	__gpio_ack_irq(POWEROFF_PIN);
	__gpio_mask_irq(POWEROFF_PIN);
	__gpio_as_input(POWEROFF_PIN);
#ifdef CONFIG_JZ_UDC_HOTPLUG
	if (__gpio_get_pin(POWEROFF_PIN)==POWEROFF_PIN_DOWN && jz_udc_active == 0){
#else
	if (__gpio_get_pin(POWEROFF_PIN)==POWEROFF_PIN_DOWN){
#endif
		del_timer(&poweroff_delaytimer);
		init_timer(&poweroff_delaytimer);
		poweroff_delaytimer.expires = jiffies + POWEROFF_DELAY/10;
		poweroff_delaytimer.data = 0;
		poweroff_delaytimer.function = poweroff_delaytimer_routine;
		add_timer(&poweroff_delaytimer);
	}
	else {

/*
 * If it reaches here without jz_udc_active == 0, then it indicates POWEROFF_PIN was
 * changed to WAKEUP key in pm.c for hand is not able to rise up so quickly, so the
 * irq handler entered because of WAKEUP key not POWEROFF_PIN.
 */

#ifdef CONFIG_JZ_UDC_HOTPLUG
		if (jz_udc_active == 1)
		printk("\nUSB is working; Operation is denied\n");
#endif
		SET_POWEROFF_PIN_AS_IRQ;
		__gpio_unmask_irq(POWEROFF_PIN);
	}

	return IRQ_HANDLED;
}

#ifdef USE_SUSPEND_HOTPLUG
static void run_sbin_hotplug(int state)
{
        int i;
        char *argv[3], *envp[8];
        char media[64], slotnum[16];
        if (!uevent_helper[0])
                return;

        i = 0;
	argv[i++] = uevent_helper;
	//argv[i++] = "home/lhhuang/hotplug";

	if ( poweroff_flag ==  1 )
		argv[i++] = "poweroff";
	else
		argv[i++] = "suspend";

        argv[i] = 0;

        /* minimal command environment */
        i = 0;
        envp[i++] = "HOME=/";
        envp[i++] = "PATH=/sbin:/bin:/usr/sbin:/usr/bin";

        /* other stuff we want to pass to /sbin/hotplug */
        sprintf(slotnum, "SLOT=0");

	if ( poweroff_flag ==  1 )
		sprintf(media, "MEDIA=poweroff");
	else
		sprintf(media, "MEDIA=suspend");

        envp[i++] = slotnum;
        envp[i++] = media;

	if (state)
		envp[i++] = "ACTION=enter";
	else
		envp[i++] = "ACTION=exit";

        envp[i] = 0;

        dprintk("SUSPEND: hotplug path=%s state=%d\n", argv[0], state);

	SET_POWEROFF_PIN_AS_IRQ;
	__gpio_unmask_irq(POWEROFF_PIN); /* set it because call hotplug with call_usermodehelper() \
                                           might failed, especially when using nfsroot */

	call_usermodehelper (argv [0], argv, envp, -1);
}
#endif

static void suspend_handler(struct work_struct *work)
{
#ifdef USE_SUSPEND_HOTPLUG
        int state = 1;
        run_sbin_hotplug(state);
#else
	if (poweroff_flag) {
		dprintk("DO_SHUTDOWN_SYSTEM\n");
		DO_SHUTDOWN_SYSTEM;
	} else {
		dprintk("DO_SUSPEND\n");
		DO_SUSPEND;
	}
#endif
}

#ifdef CONFIG_PM
static int poweroff_suspend(struct platform_device *pdev, pm_message_t state)
{
	printk("%s(): Called.\n", __func__);

	suspend_flag = 1;
	poweroff_flag = 0;

	return 0;
}

static int poweroff_resume(struct platform_device *pdev)
{
	printk("%s(): Called.\n", __func__);

	suspend_flag = 0;
	SET_POWEROFF_PIN_AS_IRQ;
	__gpio_unmask_irq(POWEROFF_PIN);

	return 0;
}
#endif /* CONFIG_PM */

static int __devinit poweroff_probe(struct platform_device *pdev)
{
	int retval;

	retval = request_irq(POWEROFF_IRQ, poweroff_irq,
			     IRQF_DISABLED, "poweroff", NULL);

	SET_POWEROFF_PIN_AS_IRQ;

	if (retval) {
		printk("Could not get poweroff irq %d\n", POWEROFF_IRQ);
		return retval;
	}

	INIT_WORK(&suspend_work, suspend_handler);

	printk(KERN_INFO JZ_SOC_NAME": Power GPIO Button driver registered.\n");
	return 0;
}

static int __devexit poweroff_remove(struct platform_device *pdev)
{
	free_irq(POWEROFF_IRQ, NULL);

	return 0;
}

static void jz_poweroff_release(struct device *dev)
{
	return;
}

static struct platform_device jz_poweroff_device = {
	.name		= "jz-poweroff",
	.id		= -1,
	.dev		= {
		.release	= jz_poweroff_release,
	},
};

static struct platform_driver jz_poweroff_driver = {
	.probe		= poweroff_probe,
	.remove		= __devexit_p(poweroff_remove),
	.suspend	= poweroff_suspend,
	.resume		= poweroff_resume,
	.driver		= {
		.name	= "jz-poweroff",
		.owner	= THIS_MODULE,
	},
};

static int __init jz_poweroff_init(void)
{
	int rv;
	rv = platform_driver_register(&jz_poweroff_driver);
	if (rv)
		return rv;

	return platform_device_register(&jz_poweroff_device);
}

static void __exit jz_poweroff_cleanup(void)
{
	platform_driver_unregister(&jz_poweroff_driver);
}

module_init(jz_poweroff_init);
module_exit(jz_poweroff_cleanup);
