/*
 * linux/arch/mips/jz4760/pm.c
 *
 * JZ4760 Power Management Routines
 *
 * Copyright (C) 2006 - 2010 Ingenic Semiconductor Inc.
 *
 *  This program is free software; you can distribute it and/or modify it
 *  under the terms of the GNU General Public License (Version 2) as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 */

#include <linux/init.h>
#include <linux/pm.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/suspend.h>
#include <linux/proc_fs.h>
#include <linux/sysctl.h>

#include <asm/cacheops.h>
#include <asm/jzsoc.h>

#undef DEBUG
//#define DEBUG
#ifdef DEBUG
#define dprintk(x...)	printk(x)
#else
#define dprintk(x...)
#endif

extern void jz_board_do_sleep(unsigned long *ptr);
extern void jz_board_do_resume(unsigned long *ptr);
#if defined(CONFIG_PM_POWERDOWN_P0)
extern void jz_cpu_sleep(void);
extern void jz_cpu_resume(void);
#endif
#if defined(CONFIG_INPUT_WM831X_ON)
extern void wm8310_power_off(void);
#endif
int jz_pm_do_hibernate(void)
{
#if defined(CONFIG_INPUT_WM831X_ON)
	printk("The power will be off.\n");
	wm8310_power_off();
	while(1);
#else

	printk("Put CPU into hibernate mode.\n");

	/* Mask all interrupts */
	OUTREG32(INTC_ICMSR(0), 0xffffffff);
	OUTREG32(INTC_ICMSR(1), 0x7ff);

	/*
	 * RTC Wakeup or 1Hz interrupt can be enabled or disabled
	 * through  RTC driver's ioctl (linux/driver/char/rtc_jz.c).
	 */

	/* Set minimum wakeup_n pin low-level assertion time for wakeup: 100ms */
	rtc_write_reg(RTC_HWFCR, HWFCR_WAIT_TIME(100));

	/* Set reset pin low-level assertion time after wakeup: must  > 60ms */
	rtc_write_reg(RTC_HRCR, HRCR_WAIT_TIME(60));

	/* Scratch pad register to be reserved */
	rtc_write_reg(RTC_HSPR, HSPR_RTCV);

	/* clear wakeup status register */
	rtc_write_reg(RTC_HWRSR, 0x0);

	/* Put CPU to hibernate mode */
	rtc_write_reg(RTC_HCR, HCR_PD);

	while (1) {
		printk("We should NOT come here, please check the jz4760rtc.h!!!\n");
	};
#endif

	/* We can't get here */
	return 0;
}

static int jz_pm_do_sleep(void)
{
	unsigned long delta;
	unsigned long nfcsr = REG_NEMC_NFCSR;
	unsigned long opcr = INREG32(CPM_OPCR);
	unsigned long icmr0 = INREG32(INTC_ICMR(0));
	unsigned long icmr1 = INREG32(INTC_ICMR(1));
	unsigned long sadc = INREG8(SADC_ADENA);
	unsigned long pmembs0 = REG_EMC_PMEMBS0;
	unsigned long sleep_gpio_save[7*(GPIO_PORT_NUM-1)];
	unsigned long cpuflags;

	/* set SLEEP mode */
	CMSREG32(CPM_LCR, LCR_LPM_SLEEP, LCR_LPM_MASK);

	/* Preserve current time */
	delta = xtime.tv_sec - rtc_read_reg(RTC_RTCSR);

	/* Save CPU irqs */
	local_irq_save(cpuflags);

        /* Disable nand flash */
	REG_NEMC_NFCSR = ~0xff;

	/*pull up enable pin of DQS */
	REG_EMC_PMEMBS0 |= (0xff << 8);

        /* stop sadc */
	SETREG8(SADC_ADENA,ADENA_POWER);
	while ((INREG8(SADC_ADENA) & ADENA_POWER) != ADENA_POWER) {
		dprintk("INREG8(SADC_ADENA) = 0x%x\n",INREG8(SADC_ADENA));
		udelay(100);
	}

        /* stop uhc */
	SETREG32(CPM_OPCR, OPCR_UHCPHY_DISABLE);

	/* stop otg and gps */
	CLRREG32(CPM_OPCR, OPCR_OTGPHY_ENABLE | OPCR_GPSEN);

	/*power down gps and ahb1*/
	SETREG32(CPM_LCR, LCR_PDAHB1 | LCR_PDGPS);

	/* Mask all interrupts except rtc*/
	OUTREG32(INTC_ICMSR(0), 0xffffffff);
	OUTREG32(INTC_ICMSR(1), 0x7fe);

#if defined(CONFIG_RTC_DRV_JZ4760)
	/* unmask rtc interrupts */
	OUTREG32(INTC_ICMCR(1), 0x1);
#else
	/* mask rtc interrupts */
	OUTREG32(INTC_ICMSR(1), 0x1);
#endif

	/* Sleep on-board modules */
	jz_board_do_sleep(sleep_gpio_save);

#if 0
	/* WAKEUP key */
	__gpio_as_irq_fall_edge(GPIO_POWER_ON);
	__gpio_unmask_irq(GPIO_POWER_ON);
	__intc_unmask_irq(IRQ_GPIO0 - (GPIO_POWER_ON/32));  /* unmask IRQ_GPIOn depends on GPIO_WAKEUP */
#endif

	/* disable externel clock Oscillator in sleep mode */
	CLRREG32(CPM_OPCR, OPCR_O1SE);

	/* select 32K crystal as RTC clock in sleep mode */
	SETREG32(CPM_OPCR, OPCR_ERCS);

#if 0 /*for cpu 336M 1:2:4*/
	OUTREG32(CPM_PSWC0ST, 0);
	OUTREG32(CPM_PSWC1ST, 6);
	OUTREG32(CPM_PSWC2ST, 8);
	OUTREG32(CPM_PSWC3ST, 0);
#endif

#if 1 /*for cpu 533M 1:2:4*/
	OUTREG32(CPM_PSWC0ST, 0);
	OUTREG32(CPM_PSWC1ST, 8);
	OUTREG32(CPM_PSWC2ST, 11);
	OUTREG32(CPM_PSWC3ST, 0);
#endif

#if defined(CONFIG_PM_POWERDOWN_P0)
	/* power down the p0 */
	SETREG32(CPM_OPCR, OPCR_PD);

	/* Clear previous reset status */
	CLRREG32(CPM_RSR, RSR_PR | RSR_WR | RSR_P0R);

	/* Set resume return address */
	OUTREG32(CPM_CPSPPR, 0x00005a5a);
	udelay(1);
	OUTREG32(CPM_CPSPR, virt_to_phys(jz_cpu_resume));

	/* *** go zzz *** */
	jz_cpu_sleep();
#else
	__asm__(".set\tmips3\n\t"
		"sync\n\t"
		"wait\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		".set\tmips0");
#endif


	/*if power down p0 ,return from sleep.S*/

	/* Restore to IDLE mode */
	CMSREG32(CPM_LCR, LCR_LPM_IDLE, LCR_LPM_MASK);

	/* Restore nand flash control register, it must be restored,
	   because it will be clear to 0 in bootrom. */
	REG_NEMC_NFCSR = nfcsr;

	/*Restore pmembs0*/
	REG_EMC_PMEMBS0 = pmembs0;

	/* Restore interrupts */
	OUTREG32(INTC_ICMR(0), icmr0);
	OUTREG32(INTC_ICMR(1), icmr1);

	/* Restore sadc */
	OUTREG8(SADC_ADENA, sadc);

	/* Resume on-board modules */
	jz_board_do_resume(sleep_gpio_save);

	/* Restore Oscillator and Power Control Register */
	OUTREG32(CPM_OPCR, opcr);

	/* Restore CPU interrupt flags */
	local_irq_restore(cpuflags);

	/* Restore current time */
	xtime.tv_sec = rtc_read_reg(RTC_RTCSR) + delta;

	return 0;
}

#define K0BASE  KSEG0
void jz_flush_cache_all(void)
{
	unsigned long addr;

	/* Clear CP0 TagLo */
	asm volatile ("mtc0 $0, $28\n\t"::);

	for (addr = K0BASE; addr < (K0BASE + 0x4000); addr += 32) {
		asm volatile (
			".set mips3\n\t"
			" cache %0, 0(%1)\n\t"
			".set mips2\n\t"
			:
			: "I" (Index_Writeback_Inv_D), "r"(addr));

		asm volatile (
			".set mips3\n\t"
			" cache %0, 0(%1)\n\t"
			".set mips2\n\t"
			:
			: "I" (Index_Store_Tag_I), "r"(addr));
	}

	asm volatile ("sync\n\t"::);

	/* invalidate BTB */
	asm volatile (
		".set mips32\n\t"
		" mfc0 %0, $16, 7\n\t"
		" nop\n\t"
		" ori $0, 2\n\t"
		" mtc0 %0, $16, 7\n\t"
		" nop\n\t"
		".set mips2\n\t"
		:
		: "r"(addr));
}

/* Put CPU to HIBERNATE mode
 *----------------------------------------------------------------------------
 * Power Management sleep sysctl interface
 *
 * Write "mem" to /sys/power/state invokes this function
 * which initiates a poweroff.
 */
void jz_pm_hibernate(void)
{
	jz_pm_do_hibernate();
}

/* Put CPU to SLEEP mode
 *----------------------------------------------------------------------------
 * Power Management sleep sysctl interface
 *
 * Write "disk" to /sys/power/state invokes this function
 * which initiates a sleep.
 */

int jz_pm_sleep(void)
{
	return jz_pm_do_sleep();
}

/*
 * valid states, only support mem(sleep) and disk(hibernate)
 */
static int jz4760_pm_valid(suspend_state_t state)
{
	return state == PM_SUSPEND_MEM;
}

/*
 * Jz CPU enter save power mode
 */
static int jz4760_pm_enter(suspend_state_t state)
{
	jz_pm_do_sleep();
	return 0;
}

static struct platform_suspend_ops jz4760_pm_ops = {
	.valid		= jz4760_pm_valid,
	.enter		= jz4760_pm_enter,
};

/*
 * Initialize power interface
 */
int __init jz_pm_init(void)
{
	printk("Power Management for JZ\n");

	suspend_set_ops(&jz4760_pm_ops);
	return 0;
}
