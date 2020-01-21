/*
 * linux/arch/mips/jz4770/i2c.c
 *
 * Jz4770 I2C routines.
 *
 * Copyright (C) 2005,2006 Ingenic Semiconductor Inc.
 * Author: <cwjia@ingenic.cn>
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
#include <linux/module.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/addrspace.h>

#include <asm/jzsoc.h>

/* I2C protocol */
#define I2C_READ	1
#define I2C_WRITE	0

#define TIMEOUT         1000

/*
 * I2C_SCK, I2C_SDA
 */
#define __gpio_as_i2c()				\
do {						\
	REG_GPIO_PXFUNS(2) = 0x00000c00;	\
	REG_GPIO_PXTRGC(2) = 0x00000c00;	\
	REG_GPIO_PXSELS(2)  = 0x00000c00;	\
} while (0)

/*
 * I2C interface
 */

static int i2c_disable()
{
	int timeout = 0xfffff, i = 100;

	REG_I2C_ENB = 0; /*disable i2c*/
	while((REG_I2C_ENSTA & 0x1) && (timeout > 0)) {
		udelay(1);
		timeout --;
	}
	if(timeout)
		return 0;
	else
		return 1;
}
void i2c_open()
{
	__gpio_as_i2c();
}

void i2c_close(void)
{
}

void i2c_setclk(unsigned int i2cclk)
{

}

static void i2c_init_as_master(unsigned char address)
{
//	__gpio_as_i2c();
	if(i2c_disable())
		printk("i2c not disable\n");
//	REG_I2C_CTRL = 0x43;
	REG_I2C_CTRL = 0x45;
	REG_I2C_TAR = address; /* slave id needed write only once */
	REG_I2C_INTM = 0;
	REG_I2C_FHCNT =49;
	REG_I2C_FLCNT =62;
//	REG_I2C_SHCNT =0xc80; // 6k
//	REG_I2C_SLCNT =0xeb0; // 6k
//	REG_I2C_SHCNT =0x640; // 12k
//	REG_I2C_SLCNT =0x758; // 12k
//	REG_I2C_SHCNT =0x320; //26k
//	REG_I2C_SLCNT =0x3ac; //26k
//	REG_I2C_SHCNT =0x12c; //71k
//	REG_I2C_SLCNT =0x160; //71k
//	REG_I2C_SHCNT =0xc8;
//	REG_I2C_SLCNT =0xe8;
	REG_I2C_ENB = 1; /*enable i2c*/
}


/* now for fpga test , count == 1 */
int i2c_read(unsigned char device, unsigned char *buf,
	       unsigned char address, int count)
{
	int cnt = count;
	volatile int tmp;
	int timeout = 0xffff;

	i2c_init_as_master(device);

	address = address & 0xff;

	while((!(REG_I2C_STA & I2C_STA_TFNF)) && --timeout)
		;
	if (!(timeout))
		printk("***timeout*****************\n");

	REG_I2C_DC = (I2C_WRITE << 8) | address;

	while(cnt) {
		timeout = 0xffff;
		while((!(REG_I2C_STA & I2C_STA_TFNF)) && --timeout)
			;
		if (!(timeout))
			printk("***timeout*****************\n");

		REG_I2C_DC = (I2C_READ << 8);
#if 1
		while((!(REG_I2C_STA & I2C_STA_RFNE)) && timeout) {
			timeout--;
		}
#endif
		if (!(timeout))
			printk("***timeout*****************\n");

		*buf = REG_I2C_DC & 0xff;

		cnt--;
		buf++;
	}
	return count - cnt;
}

int i2c_write(unsigned char device, unsigned char *buf,
		unsigned char address, int count)
{
	int cnt_in_pg,cnt = count;
	unsigned char *tmpbuf;
	unsigned char tmpaddr;
	int timeout = 0xffff;
	int rw = 0;


	tmpbuf = (unsigned char *)buf;
	tmpaddr = address;
rewrite:
	i2c_init_as_master(device);

start_write_page:
	cnt_in_pg = 0;
	while((!(REG_I2C_STA & I2C_STA_TFNF)) && --timeout)
		;
	if (!(timeout))
		printk("00***timeout*****************\n");

		REG_I2C_DC = (I2C_WRITE << 8) | tmpaddr;
#if 1

		if ((REG_I2C_TXABRT & I2C_TXABRT_ABRT_7B_ADDR_NOACK) || ( REG_I2C_STA & I2C_STA_TFE)) {
			mdelay(3);
			int a = REG_I2C_CINTR;
			goto rewrite;
		}
#endif
	while(cnt) {

		timeout = 0xfffff;
#if 1
		if ((REG_I2C_TXABRT & I2C_TXABRT_ABRT_7B_ADDR_NOACK) || ( REG_I2C_STA & I2C_STA_TFE)) {
			mdelay(3);
			rw = 1;
			tmpbuf = (unsigned char *)buf;
			cnt = 16;
			int a = REG_I2C_CINTR;
			goto rewrite;
		}
#endif

		if (++cnt_in_pg > 16) { //8 or 16
			mdelay(3);
			tmpaddr += 16;
			goto start_write_page;
		}


		while((!(REG_I2C_STA & I2C_STA_TFNF)) && timeout)
			;

		if ((REG_I2C_TXABRT & I2C_TXABRT_ABRT_7B_ADDR_NOACK) || ( REG_I2C_STA & I2C_STA_TFE)) {
			mdelay(3);
			rw = 1;
			tmpbuf = (unsigned char *)buf;
			cnt = 16;
			int a = REG_I2C_CINTR;
			goto rewrite;
		}

		if(REG_I2C_TXABRT != 0)
			printk("\n\nREG_I2C_TXABRT==0x%x\n",REG_I2C_TXABRT);

		REG_I2C_DC = (I2C_WRITE << 8) | *tmpbuf;

		rw = 0;
		cnt--;
		tmpbuf++;
#if 1
		if ((REG_I2C_TXABRT & I2C_TXABRT_ABRT_7B_ADDR_NOACK) || ( REG_I2C_STA & I2C_STA_TFE)) {
			mdelay(3);
			rw = 1;
			tmpbuf = (unsigned char *)buf;
			cnt = 16;
			int a = REG_I2C_CINTR;
			goto rewrite;
		}
#endif
	}

	timeout = 0xffff;
	while(((REG_I2C_STA & I2C_STA_MSTACT)) && --timeout)
		;
	if (!(timeout))
		printk("22***timeout*****************\n");

	mdelay(2);
	udelay(450);

	if (REG_I2C_TXABRT & I2C_TXABRT_ABRT_7B_ADDR_NOACK) {
		mdelay(3);
		rw = 1;
		tmpbuf = (unsigned char *)buf;
		cnt = 16;
		int a = REG_I2C_CINTR;
		goto rewrite;
	}
	return count - cnt;
}

EXPORT_SYMBOL(i2c_open);
EXPORT_SYMBOL(i2c_close);
EXPORT_SYMBOL(i2c_setclk);
EXPORT_SYMBOL(i2c_read);
EXPORT_SYMBOL(i2c_write);
