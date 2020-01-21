/*
 * linux/arch/mips/jz4760/reset.c
 *
 * JZ4760 reset routines.
 *
 * Copyright (c) 2006-2007  Ingenic Semiconductor Inc.
 * Author: <yliu@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/sched.h>
#include <linux/mm.h>
#include <asm/io.h>
#include <asm/pgtable.h>
#include <asm/processor.h>
#include <asm/reboot.h>
#include <asm/system.h>
#include <asm/jzsoc.h>

void jz_restart(char *command)
{
	printk("Restarting after 4 ms\n");
	REG_WDT_WCSR = WCSR_PRESCALE4 | WCSR_CLKIN_EXT;
	REG_WDT_WCNT = 0;
	REG_WDT_WDR = JZ_EXTAL/1000;   /* reset after 4ms */
	REG_TCU_TSCR = TSCR_WDT; /* enable wdt clock */
	REG_WDT_WCER = WCER_TCEN;  /* wdt start */
	while (1);
}

void jz_halt(void)
{
	printk(KERN_NOTICE "\n** You can safely turn off the power\n");

	while (1)
		__asm__(".set\tmips3\n\t"
	                "wait\n\t"
			".set\tmips0");
}

void jz_power_off(void)
{
	jz_halt();
}
