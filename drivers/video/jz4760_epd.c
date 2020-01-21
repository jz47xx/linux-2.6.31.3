/*
 * linux/drivers/video/jz4760_lcd.c -- Ingenic Jz4760 LCD frame buffer device
 *
 * Copyright (C) 2005-2008, Ingenic Semiconductor Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * --------------------------------
 * NOTE:
 * This LCD driver support TFT16 TFT32 LCD, not support STN and Special TFT LCD
 * now.
 * 	It seems not necessory to support STN and Special TFT.
 * 	If it's necessary, update this driver in the future.
 * 	<Wolfgang Wang, Jun 10 2008>
 */
/*
 * Added Electronic paper support <Cynthia zhao, Jun 2010>
 *
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/suspend.h>
#include <linux/pm.h>
#include <linux/leds.h>

#include <asm/irq.h>
#include <asm/pgtable.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/processor.h>
#include <asm/jzsoc.h>

#include "console/fbcon.h"

#include "jz4760_epd.h"

#include "jzepd.c"

#define DRIVER_NAME	"jz-lcd"

#ifdef CONFIG_JZ4760_SLCD_KGM701A3_TFT_SPFD5420A
#include "jz_kgm_spfd5420a.h"
#endif

MODULE_DESCRIPTION("Jz4760 LCD Controller driver");
MODULE_AUTHOR("Wolfgang Wang <lgwang@ingenic.cn>, Lemon Liu <zyliu@ingenic.cn>");
MODULE_LICENSE("GPL");

#define LCD_DEBUG
//#undef LCD_DEBUG

#ifdef LCD_DEBUG
#define dprintk(x...)	printk(x)
#define print_dbg(f, arg...) printk("dbg::" __FILE__ ",LINE(%d): " f "\n", __LINE__, ## arg)
#else
#define dprintk(x...)
#define print_dbg(f, arg...) do {} while (0)
#endif

#define print_err(f, arg...) printk(KERN_ERR DRIVER_NAME ": " f "\n", ## arg)
#define print_warn(f, arg...) printk(KERN_WARNING DRIVER_NAME ": " f "\n", ## arg)
#define print_info(f, arg...) printk(KERN_INFO DRIVER_NAME ": " f "\n", ## arg)

#define JZ_LCD_ID "jz-lcd"
#define ANDROID_NUMBER_OF_BUFFERS 2

struct lcd_cfb_info {
	struct fb_info		fb0;	/* foreground 0 */
	struct fb_info		fb;	/* foreground 1 */
	struct display_switch	*dispsw;
	signed int		currcon;
	int			func_use_count;

	struct {
		u16 red, green, blue;
	} palette[NR_PALETTE];
#ifdef CONFIG_PM
	struct pm_dev		*pm;
#endif
};

static int lcd_backlight_level = 100;
static struct lcd_cfb_info *jz4760fb_info;
static int current_dma0_id, current_dma1_id;
static struct jz4760_lcd_dma_desc *dma_desc_base;
static struct jz4760_lcd_dma_desc *dma0_desc_palette, *dma0_desc0, *dma0_desc1, *dma1_desc0, *dma1_desc1;

#define DMA_DESC_NUM 		9

unsigned char *lcd_palette;
static unsigned char *lcd_frame0;
static unsigned char *lcd_frame;

#define EPD_MODE_PAL 1

/*because sdram can't support above 16 frames, so we should do sth. By Cynthia 2010*/
#define NR_DMA1_DESC_EPD 1
#define NR_DMA_DESC_EPD_PER_GROUP 17 //16 frames + 1 palette
#define NR_EPD_PAL 12  // 12 paletteswe should surport 12 palette, 16 frame per palette, so we can support 12*16=192frames
#define NR_DMA_DESC_EPD NR_DMA_DESC_EPD_PER_GROUP*NR_EPD_PAL
static unsigned int palette_offset = 16*16*4; //(Bytes) per frame is 16 word, totally 16frame per palette
static struct jz4760_lcd_dma_desc *dma_desc_epd[NR_DMA_DESC_EPD];
static struct jz4760_lcd_dma_desc *dma0_desc_palette_epd[NR_EPD_PAL];


int use_fg1_only = 0;
int use_fg0_only = 0;
int use_2layer_Fg = 1;


/*Cynthia zhao add end*/


/* APP */

static void jz4760fb_deep_set_mode( struct jz4760lcd_info * lcd_info );
static void print_lcdc_registers(void);
#ifdef CONFIG_FB_JZ4760_TVE
static void jz4760lcd_info_switch_to_TVE(int mode);
static void jz4760lcd_info_switch_to_lcd(void);
#endif

static int jz4760fb0_foreground_resize(struct jz4760lcd_osd_t *lcd_osd_info);
static int jz4760fb0_foreground_move(struct jz4760lcd_osd_t *lcd_osd_info);

static int jz4760fb_foreground_resize(struct jz4760lcd_osd_t *lcd_osd_info);
static int jz4760fb_foreground_move(struct jz4760lcd_osd_t *lcd_osd_info);


struct jz4760lcd_info jz4760_lcd_panel = {
#if defined(CONFIG_JZ4760_LCD_SAMSUNG_LTP400WQF02)
	.panel = {
		.cfg = LCD_CFG_LCDPIN_LCD | LCD_CFG_RECOVER | /* Underrun recover */
		LCD_CFG_NEWDES | /* 8words descriptor */
		LCD_CFG_MODE_GENERIC_TFT | /* General TFT panel */
		LCD_CFG_MODE_TFT_18BIT | 	/* output 18bpp */
		LCD_CFG_HSP | 	/* Hsync polarity: active low */
		LCD_CFG_VSP,	/* Vsync polarity: leading edge is falling edge */
		.slcd_cfg = 0,
		.ctrl = LCD_CTRL_OFUM | LCD_CTRL_BST_16,	/* 16words burst, enable out FIFO underrun irq */
		480, 272, 60, 41, 10, 2, 2, 2, 2,
	},
	.osd = {
		.osd_cfg = LCD_OSDC_OSDEN, /* Use OSD mode */
		 .osd_ctrl = 0,		/* disable ipu,  */
		 .rgb_ctrl = 0,
		 .bgcolor = 0x000000, /* set background color Black */
		 .colorkey0 = 0, /* disable colorkey */
		 .colorkey1 = 0, /* disable colorkey */
		 .alpha = 0xA0,	/* alpha value */
		 .ipu_restart = 0x80001000, /* ipu restart */
		 .fg_change = FG_CHANGE_ALL, /* change all initially */
		 .fg0 = {32, 0, 0, 480, 272}, /* bpp, x, y, w, h */
		 .fg1 = {32, 0, 0, 480, 272}, /* bpp, x, y, w, h */
	 },
#elif defined(CONFIG_JZ4760_LCD_AUO_A043FL01V2)
	.panel = {
		.cfg = LCD_CFG_LCDPIN_LCD | LCD_CFG_RECOVER | /* Underrun recover */
		LCD_CFG_NEWDES | /* 8words descriptor */
		LCD_CFG_MODE_GENERIC_TFT | /* General TFT panel */
		LCD_CFG_MODE_TFT_24BIT | 	/* output 18bpp */
		LCD_CFG_HSP | 	/* Hsync polarity: active low */
		LCD_CFG_VSP,	/* Vsync polarity: leading edge is falling edge */
		.slcd_cfg = 0,
		.ctrl = LCD_CTRL_OFUM | LCD_CTRL_BST_16,	/* 16words burst, enable out FIFO underrun irq */
		480, 272, 60, 41, 10, 8, 4, 4, 2,
	},
	.osd = {
		.osd_cfg = LCD_OSDC_OSDEN | LCD_OSDC_ALPHAEN,// | /* Use OSD mode */
		.osd_ctrl = 0,		/* disable ipu,  */
		.rgb_ctrl = 0,
		.bgcolor = 0x000000, /* set background color Black */
		.colorkey0 = 0x80000000, /* disable colorkey */
//		.colorkey0 = 0, /* disable colorkey */
		.colorkey1 = 0, /* disable colorkey */
		.alpha = 0xff,	/* alpha value */
//		.alpha = 0xA0,	/* alpha value */
		.ipu_restart = 0x80001000, /* ipu restart */
		.fg_change = FG_CHANGE_ALL, /* change all initially */
		.fg0 = {32, 0, 0, 480, 272}, /* bpp, x, y, w, h */
		.fg1 = {32, 0, 0, 480, 272}, /* bpp, x, y, w, h */
	},
#elif defined(CONFIG_JZ4760_LCD_TRULY_TFT_GG1P0319LTSW_W)
	.panel = {
		 .cfg = LCD_CFG_LCDPIN_SLCD | /* Underrun recover*/
		 LCD_CFG_NEWDES | /* 8words descriptor */
		 LCD_CFG_MODE_SLCD, /* TFT Smart LCD panel */
		 .slcd_cfg = SLCD_CFG_DWIDTH_16BIT | SLCD_CFG_CWIDTH_16BIT | SLCD_CFG_CS_ACTIVE_LOW | SLCD_CFG_RS_CMD_LOW | SLCD_CFG_CLK_ACTIVE_FALLING | SLCD_CFG_TYPE_PARALLEL,
		 .ctrl = LCD_CTRL_OFUM | LCD_CTRL_BST_16,	/* 16words burst, enable out FIFO underrun irq */
		 240, 320, 60, 0, 0, 0, 0, 0, 0,
	 },
	.osd = {
		.osd_cfg = LCD_OSDC_OSDEN,/* Use OSD mode */
		 .osd_ctrl = 0,		/* disable ipu,  */
		 .rgb_ctrl = 0,
		 .bgcolor = 0x000000, /* set background color Black */
		 .colorkey0 = 0, /* disable colorkey */
		 .colorkey1 = 0, /* disable colorkey */
		 .alpha = 0xA0,	/* alpha value */
		 .ipu_restart = 0x80001000, /* ipu restart */
		 .fg_change = FG_CHANGE_ALL, /* change all initially */
		 .fg0 = {32, 0, 0, 240, 320}, /* bpp, x, y, w, h */
		 .fg1 = {32, 0, 0, 240, 320}, /* bpp, x, y, w, h */
	 },

#elif defined(CONFIG_JZ4760_LCD_FOXCONN_PT035TN01)
	.panel = {
		.cfg = LCD_CFG_LCDPIN_LCD | LCD_CFG_RECOVER | /* Underrun recover */
		LCD_CFG_NEWDES | /* 8words descriptor */
		LCD_CFG_MODE_GENERIC_TFT | /* General TFT panel */
		LCD_CFG_MODE_TFT_24BIT | 	/* output 24bpp */
		LCD_CFG_HSP | 	/* Hsync polarity: active low */
		LCD_CFG_VSP |	/* Vsync polarity: leading edge is falling edge */
		LCD_CFG_PCP,	/* Pix-CLK polarity: data translations at falling edge */
		.slcd_cfg = 0,
		.ctrl = LCD_CTRL_OFUM | LCD_CTRL_BST_16,	/* 16words burst, enable out FIFO underrun irq */
		320, 240, 80, 1, 1, 10, 50, 10, 13
	},
	.osd = {
		.osd_cfg = LCD_OSDC_OSDEN, /* Use OSD mode */
		 .osd_ctrl = 0,		/* disable ipu,  */
		 .rgb_ctrl = 0,
		 .bgcolor = 0x000000, /* set background color Black */
		 .colorkey0 = 0, /* disable colorkey */
		 .colorkey1 = 0, /* disable colorkey */
		 .alpha = 0xA0,	/* alpha value */
		 .ipu_restart = 0x80001000, /* ipu restart */
		 .fg_change = FG_CHANGE_ALL, /* change all initially */
		 .fg0 = {32, 0, 0, 320, 240}, /* bpp, x, y, w, h */
		 .fg1 = {32, 0, 0, 320, 240}, /* bpp, x, y, w, h */
	 },
#elif defined(CONFIG_JZ4760_LCD_INNOLUX_PT035TN01_SERIAL)
	.panel = {
		.cfg = LCD_CFG_LCDPIN_LCD | LCD_CFG_RECOVER | /* Underrun recover */
		LCD_CFG_NEWDES | /* 8words descriptor */
		LCD_CFG_MODE_SERIAL_TFT | /* Serial TFT panel */
		LCD_CFG_MODE_TFT_18BIT | 	/* output 18bpp */
		LCD_CFG_HSP | 	/* Hsync polarity: active low */
		LCD_CFG_VSP |	/* Vsync polarity: leading edge is falling edge */
		LCD_CFG_PCP,	/* Pix-CLK polarity: data translations at falling edge */
		.slcd_cfg = 0,
		.ctrl = LCD_CTRL_OFUM | LCD_CTRL_BST_16,	/* 16words burst, enable out FIFO underrun irq */
		320, 240, 60, 1, 1, 10, 50, 10, 13
	},
	.osd = {
		.osd_cfg = LCD_OSDC_OSDEN, /* Use OSD mode */
		.osd_ctrl = 0,		/* disable ipu,  */
		 .rgb_ctrl = 0,
		.bgcolor = 0x000000, /* set background color Black */
		.colorkey0 = 0, /* disable colorkey */
		.colorkey1 = 0, /* disable colorkey */
		.alpha = 0xA0,	/* alpha value */
		.ipu_restart = 0x80001000, /* ipu restart */
		.fg_change = FG_CHANGE_ALL, /* change all initially */
		.fg0 = {32, 0, 0, 320, 240}, /* bpp, x, y, w, h */
		.fg1 = {32, 0, 0, 320, 240}, /* bpp, x, y, w, h */
	},
#elif defined(CONFIG_JZ4760_SLCD_KGM701A3_TFT_SPFD5420A)
	.panel = {
		.cfg = LCD_CFG_LCDPIN_SLCD | /* Underrun recover*/
//		 LCD_CFG_DITHER | /* dither */
		LCD_CFG_NEWDES | /* 8words descriptor */
		LCD_CFG_MODE_SLCD, /* TFT Smart LCD panel */
		.slcd_cfg = SLCD_CFG_DWIDTH_18BIT | SLCD_CFG_CWIDTH_18BIT | SLCD_CFG_CS_ACTIVE_LOW | SLCD_CFG_RS_CMD_LOW | SLCD_CFG_CLK_ACTIVE_FALLING | SLCD_CFG_TYPE_PARALLEL,
		 .ctrl = LCD_CTRL_OFUM | LCD_CTRL_BST_16,	/* 16words burst, enable out FIFO underrun irq */
		 400, 240, 60, 0, 0, 0, 0, 0, 0,
	 },
	.osd = {
		.osd_cfg = LCD_OSDC_OSDEN, /* Use OSD mode */
		.osd_ctrl = 0,		/* disable ipu,  */
		.rgb_ctrl = 0,
		.bgcolor = 0x000000, /* set background color Black */
		.colorkey0 = 0, /* disable colorkey */
		.colorkey1 = 0, /* disable colorkey */
		.alpha = 0xA0,	/* alpha value */
		.ipu_restart = 0x80001000, /* ipu restart */
		.fg_change = FG_CHANGE_ALL, /* change all initially */
		.fg0 = {32, 0, 0, 400, 240}, /* bpp, x, y, w, h */
		.fg1 = {32, 0, 0, 400, 240}, /* bpp, x, y, w, h */
	},
#elif defined(CONFIG_JZ4760_VGA_DISPLAY)
	.panel = {
		.cfg = LCD_CFG_LCDPIN_LCD | LCD_CFG_RECOVER |/* Underrun recover */
		LCD_CFG_NEWDES | /* 8words descriptor */
		LCD_CFG_MODE_GENERIC_TFT | /* General TFT panel */
		LCD_CFG_MODE_TFT_24BIT | 	/* output 18bpp */
		LCD_CFG_HSP | 	/* Hsync polarity: active low */
		LCD_CFG_VSP,	/* Vsync polarity: leading edge is falling edge */
		.slcd_cfg = 0,
		.ctrl = LCD_CTRL_OFUM | LCD_CTRL_BST_16,	/* 16words burst, enable out FIFO underrun irq */
//		800, 600, 60, 128, 4, 40, 88, 0, 23
		640, 480, 54, 96, 2, 16, 48, 10, 33
//		1280, 720, 50, 152, 15, 22, 200, 14, 1
	},
	.osd = {
		 .osd_cfg = LCD_OSDC_OSDEN | /* Use OSD mode */
//		 LCD_OSDC_ALPHAEN | /* enable alpha */
//		 LCD_OSDC_F1EN | /* enable Foreground1 */
		 LCD_OSDC_F0EN,	/* enable Foreground0 */
		 .osd_ctrl = 0,		/* disable ipu,  */
		 .rgb_ctrl = 0,
		 .bgcolor = 0x000000, /* set background color Black */
		 .colorkey0 = 0, /* disable colorkey */
		 .colorkey1 = 0, /* disable colorkey */
		 .alpha = 0xA0,	/* alpha value */
		 .ipu_restart = 0x80001000, /* ipu restart */
		 .fg_change = FG_CHANGE_ALL, /* change all initially */
		 .fg0 = {32, 0, 0, 640, 480}, /* bpp, x, y, w, h */
		 .fg1 = {32, 0, 0, 640, 480}, /* bpp, x, y, w, h */
	 },
#elif defined(CONFIG_JZ4760_EPSON_EPD_DISPLAY)
	.panel = {
		.cfg = LCD_CFG_LCDPIN_LCD | LCD_CFG_RECOVER |/* Underrun recover */
		LCD_CFG_NEWDES | /* 8words descriptor */
		LCD_CFG_MODE_GENERIC_TFT | /* General TFT panel */
		LCD_CFG_MODE_TFT_24BIT | 	/* output 18bpp */
		LCD_CFG_HSP | 	/* Hsync polarity: active low */
		LCD_CFG_VSP,	/* Vsync polarity: leading edge is falling edge */
		.slcd_cfg = 0,
		.ctrl = LCD_CTRL_OFUM | LCD_CTRL_BST_32,	/* 16words burst, enable out FIFO underrun irq */
		800, 600, 60, 128, 4, 40, 88, 0, 23

//		1280, 720, 50, 152, 15, 22, 200, 14, 1
	},
	.osd = {
		 .osd_cfg = LCD_OSDC_OSDEN | /* Use OSD mode */
//		 LCD_OSDC_ALPHAEN | /* enable alpha */
#if EPD_MODE_PAL
		 LCD_OSDC_F1EN | /* enable Foreground1 */
		 LCD_OSDC_F0EN,	/* enable Foreground0 */
#else
		 LCD_OSDC_F1EN,
#endif
		 .osd_ctrl = 0,		/* disable ipu,  */
		 .rgb_ctrl = 0,
		 .bgcolor = 0x000000, /* set background color Black */
		 .colorkey0 = 0, /* disable colorkey */
		 .colorkey1 = 0, /* disable colorkey */
		 .alpha = 0x0,	/* alpha value */
		 .ipu_restart = 0x80001000, /* ipu restart */
		 .fg_change = FG_CHANGE_ALL, /* change all initially */
#if EPD_MODE_PAL

		 .fg0 = {4, 0, 0, 800, 600}, /* bpp, x, y, w, h */
//		 .fg0 = {4, 0, 0, 0, 0}, /* bpp, x, y, w, h */
		 .fg1 = {4, 0, 0, 800, 600}, /* bpp, x, y, w, h */
#else

		 .fg0 = {2, 0, 0, 800, 600}, /* bpp, x, y, w, h */
//		 .fg0 = {4, 0, 0, 0, 0}, /* bpp, x, y, w, h */
		 .fg1 = {2, 0, 0, 800, 600}, /* bpp, x, y, w, h */
#endif
	 },
#else
#error "Select LCD panel first!!!"
#endif
};

#ifdef CONFIG_FB_JZ4760_TVE
struct jz4760lcd_info jz4760_info_tve = {
	.panel = {
		.w = TVE_WIDTH_PAL, TVE_HEIGHT_PAL, TVE_FREQ_PAL, 0, 0, 0, 0, 0, 0,
	},
	.osd = {
		.rgb_ctrl = LCD_RGBC_YCC, /* enable RGB => YUV */
		.fg0 = {32,},	/*  */
		.fg1 = {32,},
	},
};
#endif

struct jz4760lcd_info *jz4760_lcd_info = &jz4760_lcd_panel; /* default output to lcd panel */



static inline u_int chan_to_field(u_int chan, struct fb_bitfield *bf)
{
        chan &= 0xffff;
        chan >>= 16 - bf->length;
        return chan << bf->offset;
}

#if defined(CONFIG_JZ4760_EPSON_EPD_DISPLAY)
void print_epd_regs(void)
{
	printk("REG_EPD_CTRL1 \t= 0x%08x\n",REG_EPD_CTRL1);
	printk("REG_EPD_CTRL2 \t= 0x%08x\n",REG_EPD_CTRL2);
	printk("REG_EPD_CTRL3 \t= 0x%08x\n",REG_EPD_CTRL3);
	printk("REG_EPD_CTRL4 \t= 0x%08x\n",REG_EPD_CTRL4);
	printk("REG_EPD_CTRL5 \t= 0x%08x\n",REG_EPD_CTRL5);
	printk("REG_EPD_CTRL6 \t= 0x%08x\n",REG_EPD_CTRL6);
	printk("REG_EPD_CTRL7 \t= 0x%08x\n",REG_EPD_CTRL7);
	printk("REG_EPD_CTRL8 \t= 0x%08x\n",REG_EPD_CTRL8);
	printk("REG_EPD_CTRL9 \t= 0x%08x\n",REG_EPD_CTRL9);
	printk("REG_LCD_VAT \t= 0x%08x\n",REG_LCD_VAT);
	printk("REG_LCD_PS \t= 0x%08x\n",REG_LCD_PS);
	printk("REG_LCD_CLS \t= 0x%08x\n",REG_LCD_CLS);
	printk("REG_LCD_VSYNC \t= 0x%08x\n",REG_LCD_VSYNC);
	printk("REG_LCD_HSYNC \t= 0x%08x\n",REG_LCD_HSYNC);
	printk("\n");
	printk("REG_EPD_CTRL1 \t= 0x%08x\n",(unsigned int)&REG_EPD_CTRL1);
	printk("REG_EPD_CTRL2 \t= 0x%08x\n",(unsigned int)&REG_EPD_CTRL2);
	printk("REG_EPD_CTRL3 \t= 0x%08x\n",(unsigned int)&REG_EPD_CTRL3);
	printk("REG_EPD_CTRL4 \t= 0x%08x\n",(unsigned int)&REG_EPD_CTRL4);
	printk("REG_EPD_CTRL5 \t= 0x%08x\n",(unsigned int)&REG_EPD_CTRL5);
	printk("REG_EPD_CTRL6 \t= 0x%08x\n",(unsigned int)&REG_EPD_CTRL6);
	printk("REG_EPD_CTRL7 \t= 0x%08x\n",(unsigned int)&REG_EPD_CTRL7);
	printk("REG_EPD_CTRL8 \t= 0x%08x\n",(unsigned int)&REG_EPD_CTRL8);
	printk("REG_EPD_CTRL9 \t= 0x%08x\n",(unsigned int)&REG_EPD_CTRL9);
	printk("REG_LCD_VAT \t= 0x%08x\n",(unsigned int)&REG_LCD_VAT);
	printk("REG_LCD_PS \t= 0x%08x\n",(unsigned int)&REG_LCD_PS);
	printk("REG_LCD_CLS \t= 0x%08x\n",(unsigned int)&REG_LCD_CLS);
	printk("REG_LCD_VSYNC \t= 0x%08x\n",(unsigned int)&REG_LCD_VSYNC);
	printk("REG_LCD_HSYNC \t= 0x%08x\n",(unsigned int)&REG_LCD_HSYNC);

	printk("REG_LCD_XYP0 \t= 0x%X\n",REG_LCD_XYP0);
	printk("REG_LCD_XYP1 \t= 0x%X\n",REG_LCD_XYP1);
	printk("REG_LCD_SIZE0 \t= 0x%X\n",REG_LCD_SIZE0);
	printk("REG_LCD_SIZE1 \t= 0x%X\n",REG_LCD_SIZE1);
	printk("REG_LCD_OSDCTRL \t= 0x%X\n",REG_LCD_OSDCTRL);
	printk("REG_LCD_OSDC \t= 0x%X\n",REG_LCD_OSDC);


	/* LCD Controller Resgisters */
	printk("REG_LCD_CFG:\t0x%08x\n", REG_LCD_CFG);
	printk("REG_LCD_CTRL:\t0x%08x\n", REG_LCD_CTRL);
	printk("REG_LCD_STATE:\t0x%08x\n", REG_LCD_STATE);
	printk("REG_LCD_OSDC:\t0x%08x\n", REG_LCD_OSDC);
	printk("REG_LCD_OSDCTRL:\t0x%08x\n", REG_LCD_OSDCTRL);
	printk("REG_LCD_OSDS:\t0x%08x\n", REG_LCD_OSDS);
	printk("REG_LCD_BGC:\t0x%08x\n", REG_LCD_BGC);
	printk("REG_LCD_KEK0:\t0x%08x\n", REG_LCD_KEY0);
	printk("REG_LCD_KEY1:\t0x%08x\n", REG_LCD_KEY1);
	printk("REG_LCD_ALPHA:\t0x%08x\n", REG_LCD_ALPHA);
	printk("REG_LCD_IPUR:\t0x%08x\n", REG_LCD_IPUR);
	printk("REG_LCD_VAT:\t0x%08x\n", REG_LCD_VAT);
	printk("REG_LCD_DAH:\t0x%08x\n", REG_LCD_DAH);
	printk("REG_LCD_DAV:\t0x%08x\n", REG_LCD_DAV);
	printk("REG_LCD_XYP0:\t0x%08x\n", REG_LCD_XYP0);
	printk("REG_LCD_XYP1:\t0x%08x\n", REG_LCD_XYP1);
	printk("REG_LCD_SIZE0:\t0x%08x\n", REG_LCD_SIZE0);
	printk("REG_LCD_SIZE1:\t0x%08x\n", REG_LCD_SIZE1);
	printk("REG_LCD_RGBC\t0x%08x\n", REG_LCD_RGBC);
	printk("REG_LCD_VSYNC:\t0x%08x\n", REG_LCD_VSYNC);
	printk("REG_LCD_HSYNC:\t0x%08x\n", REG_LCD_HSYNC);
	printk("REG_LCD_PS:\t0x%08x\n", REG_LCD_PS);
	printk("REG_LCD_CLS:\t0x%08x\n", REG_LCD_CLS);
	printk("REG_LCD_SPL:\t0x%08x\n", REG_LCD_SPL);
	printk("REG_LCD_REV:\t0x%08x\n", REG_LCD_REV);
	printk("REG_LCD_IID:\t0x%08x\n", REG_LCD_IID);
	printk("REG_LCD_DA0:\t0x%08x\n", REG_LCD_DA0);
	printk("REG_LCD_SA0:\t0x%08x\n", REG_LCD_SA0);
	printk("REG_LCD_FID0:\t0x%08x\n", REG_LCD_FID0);
	printk("REG_LCD_CMD0:\t0x%08x\n", REG_LCD_CMD0);
	printk("REG_LCD_OFFS0:\t0x%08x\n", REG_LCD_OFFS0);
	printk("REG_LCD_PW0:\t0x%08x\n", REG_LCD_PW0);
	printk("REG_LCD_CNUM0:\t0x%08x\n", REG_LCD_CNUM0);
	printk("REG_LCD_DESSIZE0:\t0x%08x\n", REG_LCD_DESSIZE0);
	printk("REG_LCD_DA1:\t0x%08x\n", REG_LCD_DA1);
	printk("REG_LCD_SA1:\t0x%08x\n", REG_LCD_SA1);
	printk("REG_LCD_FID1:\t0x%08x\n", REG_LCD_FID1);
	printk("REG_LCD_CMD1:\t0x%08x\n", REG_LCD_CMD1);
	printk("REG_LCD_OFFS1:\t0x%08x\n", REG_LCD_OFFS1);
	printk("REG_LCD_PW1:\t0x%08x\n", REG_LCD_PW1);
	printk("REG_LCD_CNUM1:\t0x%08x\n", REG_LCD_CNUM1);
	printk("REG_LCD_DESSIZE1:\t0x%08x\n", REG_LCD_DESSIZE1);
	printk("==================================\n");
	printk("REG_LCD_VSYNC:\t%d:%d\n", REG_LCD_VSYNC>>16, REG_LCD_VSYNC&0xfff);
	printk("REG_LCD_HSYNC:\t%d:%d\n", REG_LCD_HSYNC>>16, REG_LCD_HSYNC&0xfff);
	printk("REG_LCD_VAT:\t%d:%d\n", REG_LCD_VAT>>16, REG_LCD_VAT&0xfff);
	printk("REG_LCD_DAH:\t%d:%d\n", REG_LCD_DAH>>16, REG_LCD_DAH&0xfff);
	printk("REG_LCD_DAV:\t%d:%d\n", REG_LCD_DAV>>16, REG_LCD_DAV&0xfff);
	printk("==================================\n");

	/* Smart LCD Controller Resgisters */
	printk("REG_SLCD_CFG:\t0x%08x\n", REG_SLCD_CFG);
	printk("REG_SLCD_CTRL:\t0x%08x\n", REG_SLCD_CTRL);
	printk("REG_SLCD_STATE:\t0x%08x\n", REG_SLCD_STATE);
	printk("==================================\n");

	/* TVE Controller Resgisters */
	printk("REG_TVE_CTRL:\t0x%08x\n", REG_TVE_CTRL);
	printk("REG_TVE_FRCFG:\t0x%08x\n", REG_TVE_FRCFG);
	printk("REG_TVE_SLCFG1:\t0x%08x\n", REG_TVE_SLCFG1);
	printk("REG_TVE_SLCFG2:\t0x%08x\n", REG_TVE_SLCFG2);
	printk("REG_TVE_SLCFG3:\t0x%08x\n", REG_TVE_SLCFG3);
	printk("REG_TVE_LTCFG1:\t0x%08x\n", REG_TVE_LTCFG1);
	printk("REG_TVE_LTCFG2:\t0x%08x\n", REG_TVE_LTCFG2);
	printk("REG_TVE_CFREQ:\t0x%08x\n", REG_TVE_CFREQ);
	printk("REG_TVE_CPHASE:\t0x%08x\n", REG_TVE_CPHASE);
	printk("REG_TVE_CBCRCFG:\t0x%08x\n", REG_TVE_CBCRCFG);
	printk("REG_TVE_WSSCR:\t0x%08x\n", REG_TVE_WSSCR);
	printk("REG_TVE_WSSCFG1:\t0x%08x\n", REG_TVE_WSSCFG1);
	printk("REG_TVE_WSSCFG2:\t0x%08x\n", REG_TVE_WSSCFG2);
	printk("REG_TVE_WSSCFG3:\t0x%08x\n", REG_TVE_WSSCFG3);

	printk("==================================\n");

	if ( 0 ) {
		unsigned int * pii = (unsigned int *)dma_desc_base;
		int i, j;
		for (j=0;j< DMA_DESC_NUM ; j++) {
			printk("dma_desc%d(0x%08x):\n", j, (unsigned int)pii);
			for (i =0; i<8; i++ ) {
				printk("\t\t0x%08x\n", *pii++);
			}
		}
	}


	if ( 0 ) {
		unsigned int * pii = (unsigned int *)dma_desc_base;
		int i, j;
		for (j=0;j< NR_DMA_DESC_EPD ; j++) {
			printk("zhihui::dma_desc%d(0x%08x):\n", j, (unsigned int)pii);
			for (i =0; i<8; i++ ) {
				printk("\t\t0x%08x\n", *pii++);
			}
		}
	}


}

void init_epd_controller(void)
{

	REG_EPD_CTRL1 = 0x04ea6600; /*c0*/ //for eink
//	REG_EPD_CTRL1 = 0x04ea4600; /*c0*/ // for oed 20100607
//	REG_EPD_CTRL1 = 0x04ea7600; /*c0*/
	REG_EPD_CTRL2 = 0x0fa20c82; /*c0*/
	REG_EPD_CTRL3 = 0x00320032; /*c0*/
	REG_EPD_CTRL4 = 0xff01580e; /*cc*/

	REG_EPD_CTRL4 = (0xff01580e); /*cc*/    //epd dma interrupt enable

// 	REG_EPD_CTRL4 = 0xff01500e; /*cc*/ /* each frame irq enable */
	REG_EPD_CTRL5 = 0x035a1024; /*d0*/ /* each update irq enable */
//	REG_EPD_CTRL5 = 0x035a2525; /*d0*/ /* each update irq enable */
	REG_EPD_CTRL6 = 0x01ad01ad; /*d4*/
	REG_EPD_CTRL7 = 0x04e40034; /*d8*/
	REG_EPD_CTRL8 = 0x00010001; /*dc*/
	REG_EPD_CTRL9 = 0x00010001; /*e0*/
	REG_LCD_VAT   = 0x04ea026a; /* 0c*/
//	REG_LCD_PS    = 0x00380358; /*18*/
//	REG_LCD_CLS   = 0x00080260; /*1c*/
	REG_LCD_PS    = 0x00380358; /*18*/
	REG_LCD_CLS   = 0x00080260; /*1c*/

	REG_LCD_VSYNC = 0x00000004; /*04*/
	REG_LCD_HSYNC = 0x04ea0028; /*08*/

	REG_SLCD_CTRL |= SLCD_CTRL_DMA_MODE;

	__lcd_set_ena();

	print_epd_regs();

	REG_EPD_CTRL5 |= 1<<0 ;//EPD_CTRL5_EPD_EN;

}

#endif

/************************************
 *      Jz475X Framebuffer ops
 ************************************/

static int jz4760fb_setcolreg(u_int regno, u_int red, u_int green, u_int blue,
			  u_int transp, struct fb_info *info)
{

	if (use_fg1_only || use_2layer_Fg){

		struct fb_info *fb = info;
		if (regno >= NR_PALETTE)
			return 1;
		if (fb->var.bits_per_pixel <= 16) {
			red	>>= 8;
			green	>>= 8;
			blue	>>= 8;

			red	&= 0xff;
			green	&= 0xff;
			blue	&= 0xff;
		}

		switch (fb->var.bits_per_pixel) {
		case 15:
			if (regno < 16)
				((u32 *)fb->pseudo_palette)[regno] =
					((red >> 3) << 10) |
					((green >> 3) << 5) |
					(blue >> 3);
			break;
		case 16:
			if (regno < 16) {
				((u32 *)fb->pseudo_palette)[regno] =
					((red >> 3) << 11) |
					((green >> 2) << 5) |
					(blue >> 3);
			}
			break;
		case 30:
			if (regno < 16)
				((u32 *)fb->pseudo_palette)[regno] =
					(red << 20) |
					(green << 10) |
					(blue << 0);
		case 17 ... 29:
		case 31 ... 32:
			if (regno < 16)
				((u32 *)fb->pseudo_palette)[regno] =
					(red << 16) |
					(green << 8) |
					(blue << 0);

			break;
		}

	}
	if (use_fg0_only || use_2layer_Fg){
		struct lcd_cfb_info *cfb = (struct lcd_cfb_info *)info;
		unsigned short *ptr, ctmp;

		if (regno >= NR_PALETTE)
			return 1;

		cfb->palette[regno].red		= red ;
		cfb->palette[regno].green	= green;
		cfb->palette[regno].blue	= blue;
		if (cfb->fb0.var.bits_per_pixel <= 16) {
			red	>>= 8;
			green	>>= 8;
			blue	>>= 8;

			red	&= 0xff;
			green	&= 0xff;
			blue	&= 0xff;
		}
		switch (cfb->fb0.var.bits_per_pixel) {
		case 1:
		case 2:
		case 4:
		case 8:
			if (((jz4760_lcd_info->panel.cfg & LCD_CFG_MODE_MASK) == LCD_CFG_MODE_SINGLE_MSTN ) ||
			    ((jz4760_lcd_info->panel.cfg & LCD_CFG_MODE_MASK) == LCD_CFG_MODE_DUAL_MSTN )) {
				ctmp = (77L * red + 150L * green + 29L * blue) >> 8;
				ctmp = ((ctmp >> 3) << 11) | ((ctmp >> 2) << 5) |
					(ctmp >> 3);
			} else {
				/* RGB 565 */
				if (((red >> 3) == 0) && ((red >> 2) != 0))
					red = 1 << 3;
				if (((blue >> 3) == 0) && ((blue >> 2) != 0))
					blue = 1 << 3;
				ctmp = ((red >> 3) << 11)
					| ((green >> 2) << 5) | (blue >> 3);
			}

			ptr = (unsigned short *)lcd_palette;
			ptr = (unsigned short *)(((u32)ptr)|0xa0000000);
			ptr[regno] = ctmp;

			break;

		case 15:
			if (regno < 16)
				((u32 *)cfb->fb0.pseudo_palette)[regno] =
					((red >> 3) << 10) |
					((green >> 3) << 5) |
					(blue >> 3);
			break;
		case 16:
			if (regno < 16) {
				((u32 *)cfb->fb0.pseudo_palette)[regno] =
					((red >> 3) << 11) |
					((green >> 2) << 5) |
					(blue >> 3);
			}
			break;
		case 17 ... 29:
		case 31 ... 32:
			if (regno < 16)
				((u32 *)cfb->fb0.pseudo_palette)[regno] =
					(red << 16) |
					(green << 8) |
					(blue << 0);


			break;
		}




	}

	return 0;

}
static int jz4760fb_ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
        void __user *argp = (void __user *)arg;


	switch (cmd) {

	case GET_EPD_INFO:
	{

		struct epd_info {
			void * frame_index_buffer;
			unsigned long frame_index_buffer_phys;
			unsigned long frame_index_buffer_size;
			void * frame_buffer_old;
			unsigned long frame_buffer_old_phys;
			unsigned long frame_buffer_old_size;
			void * frame_buffer_new;
			unsigned long frame_buffer_new_phys;
			unsigned long frame_buffer_new_size;
		}epd;
		epd.frame_index_buffer = lcd_palette;
		epd.frame_index_buffer_phys = virt_to_phys(lcd_palette);
		epd.frame_index_buffer_size = 4096;

		epd.frame_buffer_old = lcd_frame;
		epd.frame_buffer_old_phys = virt_to_phys(lcd_frame);

		epd.frame_buffer_old_size = 800*600/2;
		epd.frame_buffer_new = lcd_frame0;
		epd.frame_buffer_new_phys = virt_to_phys(lcd_frame0);

		epd.frame_buffer_new_size = 800*600/2;
		copy_to_user(argp, &epd, sizeof(epd));


		break;
	}
	case START_EPD_TRANS:
	{

		REG_EPD_CTRL2 |= EPD_CTRL2_PWRON;
		__lcd_clr_ena();

		break;
	}
	case SET_EPD_MOD:
	{
		set_epd_mod(arg);
		break;
	}

	case SET_GRAY_LEVEL:
	{
		epd_gray_level = *(unsigned long *)arg;
		break;
	}
	case SET_HAND_WRITING:
	{
		handwriting_palette();
		break;
	}
	case SET_HAND_WRITING_DMA:
	{
		REG_LCD_OSDCTRL &= ~0x7 ;
		REG_LCD_OSDCTRL |= 1 << 0 ;
		REG_LCD_OSDC &= ~(1<<3);
		REG_LCD_CMD1 = (800*600/2)/8;
		use_fg1_only=1;
		use_fg0_only=0;
		use_2layer_Fg =0;
		break;
	}
	case CANCEL_HANDWRITING_DMA:
	{
		REG_LCD_OSDCTRL &= ~0x7 ;
		REG_LCD_OSDCTRL |= 1 << 1 ;
		REG_LCD_OSDC |= (1<<3);
		REG_LCD_CMD1 = (800*600/2)/4;
		use_fg1_only=0;
		use_2layer_Fg =1;
		break;
	}

	default:
		printk("%s, unknown command(0x%x)", __FILE__, cmd);
		break;
	}

	return ret;
}


/* Use mmap /dev/fb can only get a non-cacheable Virtual Address. */
static int jz4760fb_mmap(struct fb_info *info, struct vm_area_struct *vma)
{
	if (use_fg1_only || use_2layer_Fg){
		struct fb_info *fb = info;
		unsigned long start;
		unsigned long off;
		u32 len;
		off = vma->vm_pgoff << PAGE_SHIFT;
		//fb->fb_get_fix(&fix, PROC_CONSOLE(info), info);

		/* frame buffer memory */
		start = fb->fix.smem_start;
		len = PAGE_ALIGN((start & ~PAGE_MASK) + fb->fix.smem_len);
		start &= PAGE_MASK;

		if ((vma->vm_end - vma->vm_start + off) > len)
			return -EINVAL;
		off += start;

		vma->vm_pgoff = off >> PAGE_SHIFT;
		vma->vm_flags |= VM_IO;
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);	/* Uncacheable */


		pgprot_val(vma->vm_page_prot) &= ~_CACHE_MASK;
		pgprot_val(vma->vm_page_prot) |= _CACHE_UNCACHED;		/* Uncacheable */

		if (io_remap_pfn_range(vma, vma->vm_start, off >> PAGE_SHIFT,
				       vma->vm_end - vma->vm_start,
				       vma->vm_page_prot)) {
			return -EAGAIN;
		}
	}
	if (use_fg1_only || use_2layer_Fg){
		struct lcd_cfb_info *cfb = (struct lcd_cfb_info *)info;
		unsigned long start;
		unsigned long off;
		u32 len;

		off = vma->vm_pgoff << PAGE_SHIFT;
		//fb->fb_get_fix(&fix, PROC_CONSOLE(info), info);

		/* frame buffer memory */
		start = cfb->fb0.fix.smem_start;
		len = PAGE_ALIGN((start & ~PAGE_MASK) + cfb->fb0.fix.smem_len);
		start &= PAGE_MASK;

		if ((vma->vm_end - vma->vm_start + off) > len)
			return -EINVAL;
		off += start;

		vma->vm_pgoff = off >> PAGE_SHIFT;
		vma->vm_flags |= VM_IO;
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);	/* Uncacheable */


		pgprot_val(vma->vm_page_prot) &= ~_CACHE_MASK;
		pgprot_val(vma->vm_page_prot) |= _CACHE_UNCACHED;		/* Uncacheable */

		if (io_remap_pfn_range(vma, vma->vm_start, off >> PAGE_SHIFT,
				       vma->vm_end - vma->vm_start,
				       vma->vm_page_prot)) {
			return -EAGAIN;
		}
	}
	return 0;
}


/* checks var and eventually tweaks it to something supported,
 * DO NOT MODIFY PAR */
static int jz4760fb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{

	if((var->rotate & 1) != (info->var.rotate & 1)) {
		if((var->xres != info->var.yres) ||
		   (var->yres != info->var.xres) ||
		   (var->xres_virtual != info->var.yres) ||
		   (var->yres_virtual >
		    info->var.xres * ANDROID_NUMBER_OF_BUFFERS) ||
		   (var->yres_virtual < info->var.xres )) {
			return -EINVAL;
		}
	}
	else {
		if((var->xres != info->var.xres) ||
		   (var->yres != info->var.yres) ||
		   (var->xres_virtual != info->var.xres) ||
		   (var->yres_virtual >
		    info->var.yres * ANDROID_NUMBER_OF_BUFFERS) ||
		   (var->yres_virtual < info->var.yres )) {
			return -EINVAL;
		}
	}
	if((var->xoffset != info->var.xoffset) ||
	   (var->bits_per_pixel != info->var.bits_per_pixel)) {// ||
//	   (var->grayscale != info->var.grayscale)) {
		return -EINVAL;
	}
	return 0;
}


/*
 * set the video mode according to info->var
 */
static int jz4760fb_set_par(struct fb_info *info)
{
	dprintk("jz4760fb_set_par, not implemented\n");
	return 0;
}


/*
 * (Un)Blank the display.
 * Fix me: should we use VESA value?
 */
static int jz4760fb_blank(int blank_mode, struct fb_info *info)
{
	printk("jz4760 fb_blank %d %p", blank_mode, info);
	switch (blank_mode) {
	case FB_BLANK_UNBLANK:
		//case FB_BLANK_NORMAL:
			/* Turn on panel */
		__lcd_set_ena();
		__lcd_display_on();
		break;

	case FB_BLANK_NORMAL:
	case FB_BLANK_VSYNC_SUSPEND:
	case FB_BLANK_HSYNC_SUSPEND:
	case FB_BLANK_POWERDOWN:
	/* Turn off panel */
		break;
	default:
		break;

	}
	return 0;
}

/*
 * pan display
 */
static int jz4760fb_pan_display(struct fb_var_screeninfo *var, struct fb_info *info)
{
	struct fb_info *fb = info;
	int dy;
	if (!var || !fb) {
		return -EINVAL;
	}

	if (var->xoffset - fb->var.xoffset) {
		/* No support for X panning for now! */
		return -EINVAL;
	}
	/* TODO: Wait for current frame to finished */
	dy = var->yoffset;// - fb->var.yoffset;
	if(use_fg1_only){
		if (dy) {
			dma1_desc0->databuf = (unsigned int)virt_to_phys((void *)lcd_frame + (fb->fix.line_length * dy));
			dma_cache_wback((unsigned int)(dma1_desc0), sizeof(struct jz4760_lcd_dma_desc));

		}
		else {
			dma1_desc0->databuf = (unsigned int)virt_to_phys((void *)lcd_frame);
			dma_cache_wback((unsigned int)(dma1_desc0), sizeof(struct jz4760_lcd_dma_desc));
		}
	}
	else{
		if (dy) {
			dma0_desc0->databuf = (unsigned int)virt_to_phys((void *)lcd_frame0 + (fb->fix.line_length * dy));
			dma_cache_wback((unsigned int)(dma0_desc0), sizeof(struct jz4760_lcd_dma_desc));

		}
		else {
			dma0_desc0->databuf = (unsigned int)virt_to_phys((void *)lcd_frame0);
			dma_cache_wback((unsigned int)(dma0_desc0), sizeof(struct jz4760_lcd_dma_desc));
		}
	}
	return 0;
}


static struct fb_ops jz4760fb_ops = {
	.owner			= THIS_MODULE,
	.fb_setcolreg		= jz4760fb_setcolreg,
	.fb_check_var 		= jz4760fb_check_var,
	.fb_set_par 		= jz4760fb_set_par,
	.fb_blank		= jz4760fb_blank,
	.fb_pan_display		= jz4760fb_pan_display,
	.fb_fillrect		= cfb_fillrect,
	.fb_copyarea		= cfb_copyarea,
	.fb_imageblit		= cfb_imageblit,
	.fb_mmap		= jz4760fb_mmap,
	.fb_ioctl		= jz4760fb_ioctl,
};

static int jz4760fb_set_var(struct fb_var_screeninfo *var, int con,
			struct fb_info *info)
{

	struct fb_info *fb = info;
	struct jz4760lcd_info *lcd_info = jz4760_lcd_info;
	int chgvar = 0;

	if (con == 0) {
		var->height	            = lcd_info->osd.fg0.h;	/* tve mode */
		var->width	            = lcd_info->osd.fg0.w;
		var->bits_per_pixel	    = lcd_info->osd.fg0.bpp;
	}
	else {
		var->height	            = lcd_info->osd.fg1.h;
		var->width	            = lcd_info->osd.fg1.w;
		var->bits_per_pixel	    = lcd_info->osd.fg1.bpp;
	}

	var->vmode                  = FB_VMODE_NONINTERLACED;
//	var->vmode                  = FB_VMODE_DOUBLE
	var->activate               = fb->var.activate;
	var->xres                   = var->width;
	var->yres                   = var->height;
	var->xres_virtual           = var->width;
	var->yres_virtual           = var->height * ANDROID_NUMBER_OF_BUFFERS;
	var->xoffset                = 0;
	var->yoffset                = 0;
	var->pixclock               = KHZ2PICOS(jz_clocks.pixclk/1000);

	var->left_margin            = lcd_info->panel.elw;
	var->right_margin           = lcd_info->panel.blw;
	var->upper_margin           = lcd_info->panel.efw;
	var->lower_margin           = lcd_info->panel.bfw;
	var->hsync_len              = lcd_info->panel.hsw;
	var->vsync_len              = lcd_info->panel.vsw;
	var->sync                   = 0;
	var->activate               = FB_ACTIVATE_NOW;


	/*
	 * CONUPDATE and SMOOTH_XPAN are equal.  However,
	 * SMOOTH_XPAN is only used internally by fbcon.
	 */
	if (var->vmode & FB_VMODE_CONUPDATE) {
		var->vmode |= FB_VMODE_YWRAP;
		var->xoffset = fb->var.xoffset;
		var->yoffset = fb->var.yoffset;
	}

	if (var->activate & FB_ACTIVATE_TEST)
		return 0;

	if ((var->activate & FB_ACTIVATE_MASK) != FB_ACTIVATE_NOW)
		return -EINVAL;

	if (fb->var.xres != var->xres)
		chgvar = 1;
	if (fb->var.yres != var->yres)
		chgvar = 1;
	if (fb->var.xres_virtual != var->xres_virtual)
		chgvar = 1;
	if (fb->var.yres_virtual != var->yres_virtual)
		chgvar = 1;
	if (fb->var.bits_per_pixel != var->bits_per_pixel)
		chgvar = 1;

	//display = fb_display + con;

	var->red.msb_right	= 0;
	var->green.msb_right	= 0;
	var->blue.msb_right	= 0;

	switch(var->bits_per_pixel){
	case 1:	/* Mono */
		fb->fix.visual	= FB_VISUAL_MONO01;
		fb->fix.line_length	= (var->xres * var->bits_per_pixel) / 8;
		break;
	case 2:	/* Mono */
		var->red.offset		= 0;
		var->red.length		= 2;
		var->green.offset	= 0;
		var->green.length	= 2;
		var->blue.offset	= 0;
		var->blue.length	= 2;

		fb->fix.visual	= FB_VISUAL_PSEUDOCOLOR;
		fb->fix.line_length	= (var->xres * var->bits_per_pixel) / 8;
		break;
	case 4:	/* PSEUDOCOLOUR*/
		var->red.offset		= 0;
		var->red.length		= 4;
		var->green.offset	= 0;
		var->green.length	= 4;
		var->blue.offset	= 0;
		var->blue.length	= 4;

		fb->fix.visual	= FB_VISUAL_PSEUDOCOLOR;
		fb->fix.line_length	= var->xres / 2;
		break;
	case 8:	/* PSEUDOCOLOUR, 256 */
		var->red.offset		= 0;
		var->red.length		= 8;
		var->green.offset	= 0;
		var->green.length	= 8;
		var->blue.offset	= 0;
		var->blue.length	= 8;

		fb->fix.visual	= FB_VISUAL_PSEUDOCOLOR;
		fb->fix.line_length	= var->xres ;
		break;
	case 15: /* DIRECTCOLOUR, 32k */
		var->bits_per_pixel	= 15;
		var->red.offset		= 10;
		var->red.length		= 5;
		var->green.offset	= 5;
		var->green.length	= 5;
		var->blue.offset	= 0;
		var->blue.length	= 5;

		fb->fix.visual	= FB_VISUAL_DIRECTCOLOR;
		fb->fix.line_length	= var->xres_virtual * 2;
		break;
	case 16: /* DIRECTCOLOUR, 64k */
		var->bits_per_pixel	= 16;
		var->red.offset		= 11;
		var->red.length		= 5;
		var->green.offset	= 5;
		var->green.length	= 6;
		var->blue.offset	= 0;
		var->blue.length	= 5;

		fb->fix.visual	= FB_VISUAL_TRUECOLOR;
		fb->fix.line_length	= var->xres_virtual * 2;
		break;
	case 30:
		/* DIRECTCOLOUR, 256 */
		var->bits_per_pixel	= 32;

		var->red.offset		= 20;
		var->red.length		= 10;
		var->green.offset	= 10;
		var->green.length	= 10;
		var->blue.offset	= 0;
		var->blue.length	= 10;
		var->transp.offset  	= 30;
		var->transp.length 	= 2;

		fb->fix.visual	= FB_VISUAL_TRUECOLOR;
		fb->fix.line_length	= var->xres_virtual * 4;
		break;
	case 17 ... 29:
	case 31 ... 32:
		/* DIRECTCOLOUR, 256 */
		var->bits_per_pixel	= 32;

		var->red.offset		= 16;
		var->red.length		= 8;
		var->green.offset	= 8;
		var->green.length	= 8;
		var->blue.offset	= 0;
		var->blue.length	= 8;
		var->transp.offset  	= 24;
		var->transp.length 	= 8;

		fb->fix.visual	= FB_VISUAL_TRUECOLOR;
		fb->fix.line_length	= var->xres_virtual * 4;
		break;

	default: /* in theory this should never happen */
		printk(KERN_WARNING "%s: don't support for %dbpp\n",
		       fb->fix.id, var->bits_per_pixel);
		break;
	}

	fb->var = *var;
	fb->var.activate &= ~FB_ACTIVATE_ALL;

	/*
	 * Update the old var.  The fbcon drivers still use this.
	 * Once they are using cfb->fb.var, this can be dropped.
	 *					--rmk
	 */
	//display->var = cfb->fb.var;
	/*
	 * If we are setting all the virtual consoles, also set the
	 * defaults used to create new consoles.
	 */
	fb_set_cmap(&fb->cmap, fb);
	return 0;
}

static struct lcd_cfb_info * jz4760fb_alloc_fb_info(void)
{
	struct lcd_cfb_info *cfb;
	cfb = kmalloc(sizeof(struct lcd_cfb_info) + sizeof(u32) * 16, GFP_KERNEL);

	if (!cfb)
		return NULL;

	jz4760fb_info = cfb;

	memset(cfb, 0, sizeof(struct lcd_cfb_info) );

	cfb->currcon		= -1;

	if (use_fg1_only || use_2layer_Fg){
	/* Foreground 1 -- fb */
		strcpy(cfb->fb.fix.id, "jzlcd-fg1");
		cfb->fb.flags		= FBINFO_FLAG_DEFAULT;
		cfb->fb.fix.type	= FB_TYPE_PACKED_PIXELS;
		cfb->fb.fix.type_aux	= 0;
		cfb->fb.fix.xpanstep	= 1;
		cfb->fb.fix.ypanstep	= 1;
		cfb->fb.fix.ywrapstep	= 0;
		cfb->fb.fix.accel	= FB_ACCEL_NONE;

		cfb->fb.var.nonstd	= 0;
		cfb->fb.var.activate	= FB_ACTIVATE_NOW;
		cfb->fb.var.height	= -1;
		cfb->fb.var.width	= -1;
		cfb->fb.var.accel_flags	= FB_ACCELF_TEXT;

		cfb->fb.fbops		= &jz4760fb_ops;
		cfb->fb.flags		= FBINFO_FLAG_DEFAULT;

		cfb->fb.pseudo_palette	= (void *)(cfb + 1);

		switch (jz4760_lcd_info->osd.fg1.bpp) {
		case 1:
			fb_alloc_cmap(&cfb->fb.cmap, 4, 0);
			break;
		case 2:
			fb_alloc_cmap(&cfb->fb.cmap, 8, 0);
			break;
		case 4:
			fb_alloc_cmap(&cfb->fb.cmap, 32, 0);
			break;
		case 8:

		default:
			fb_alloc_cmap(&cfb->fb.cmap, 256, 0);
			break;
		}
	}

	if (use_fg0_only || use_2layer_Fg){
		/* Foreground 0 -- fb0 */
		strcpy(cfb->fb0.fix.id, "jzlcd-fg0");
		cfb->fb0.fix.type	= FB_TYPE_PACKED_PIXELS;
		cfb->fb0.fix.type_aux	= 0;
		cfb->fb0.fix.xpanstep	= 1;
		cfb->fb0.fix.ypanstep	= 1;
		cfb->fb0.fix.ywrapstep	= 0;
		cfb->fb0.fix.accel	= FB_ACCEL_NONE;

		cfb->fb0.var.nonstd	= 0;
		cfb->fb0.var.activate	= FB_ACTIVATE_NOW;
		cfb->fb0.var.height	= -1;
		cfb->fb0.var.width	= -1;
		cfb->fb0.var.accel_flags	= FB_ACCELF_TEXT;

		cfb->fb0.fbops		= &jz4760fb_ops;
		cfb->fb0.flags		= FBINFO_FLAG_DEFAULT;

		cfb->fb0.pseudo_palette	= (void *)(cfb + 1);

		switch (jz4760_lcd_info->osd.fg0.bpp) {
		case 1:
			fb_alloc_cmap(&cfb->fb0.cmap, 4, 0);
			break;
		case 2:
			fb_alloc_cmap(&cfb->fb0.cmap, 8, 0);
			break;
		case 4:
			fb_alloc_cmap(&cfb->fb0.cmap, 32, 0);
			break;
		case 8:

		default:
			fb_alloc_cmap(&cfb->fb0.cmap, 256, 0);
			break;
		}
	}
	dprintk("fb_alloc_cmap, fb.cmap.len:%d, fb0.cmap.len:%d....\n", cfb->fb.cmap.len, cfb->fb0.cmap.len);
	return cfb;
}
static void fill_fb_newbuff(unsigned char *fb)
{
	int i,j;
	unsigned char *c = (unsigned char *)fb;
	unsigned char dat = 0;
#if EPD_MODE_PAL
	memset(c,0xff,800*600/4);
	return;
#endif
	for(i = 0;i < 600;i++)
		for(j = 0; j < 400;j++)
		{

			*c = (dat & 0xf) | (((dat + 1) & 0xf) << 4);
			c++;
			dat += 2;

		}


}
static void fill_fb_oldbuff(unsigned char *fb)
{
	int i,j,count = 1;
	unsigned char *csave = (unsigned char *)fb;
	unsigned char *c = csave;
	unsigned char dat = 0;
#if EPD_MODE_PAL
	memset(c,0xff,800*600/4);
	return;
#endif
	for(i = 0;i < 600;i++)
		for(j = 0; j < 400;j++)
		{

			*c = (dat & 0xf) | ((dat & 0xf) << 4);
			if((count & 0x7) == 0)
				dat++;
			count++;
			c++;
		}

}
static void check_fb_new_old_buf(unsigned char *newfb,unsigned char *oldfb)
{
	unsigned char *new = newfb;
	unsigned char newc,oldc;
	unsigned char *c = (unsigned char *)oldfb;
	int i;
	for(i = 0;i < 100;i++)
	{

		newc = *new++;
		oldc = ((*c++ & 0xf) << 4);
		if(((i+1) % 16) == 0)
			c++;
		//	printk("dat = 0x%X 0x%X\n",(oldc | (newc & 0xf)),(oldc | ((newc >> 4) & 0xf)));
	}

}
/*
 * Map screen memory
 */
static int jz4760fb_map_smem(struct lcd_cfb_info *cfb)
{
	unsigned long page;
	unsigned int page_shift, needroom = 0, needroom1=0, bpp, w, h;
	unsigned char *fb_palette, *fb_frame;

	/* caculate the mem size of Foreground 0 */
	if (use_fg0_only || use_2layer_Fg)
	{
		bpp = jz4760_lcd_info->osd.fg0.bpp;
		if (bpp == 18 || bpp == 24)
			bpp = 32;
		if (bpp == 15)
			bpp = 16;
#ifndef CONFIG_FB_JZ4760_TVE
		w = jz4760_lcd_info->osd.fg0.w;
		h = jz4760_lcd_info->osd.fg0.h;
#else
		w = (jz4760_lcd_info->osd.fg0.w > TVE_WIDTH_PAL) ? jz4760_lcd_info->osd.fg0.w : TVE_WIDTH_PAL;
		h = (jz4760_lcd_info->osd.fg0.h > TVE_HEIGHT_PAL) ? jz4760_lcd_info->osd.fg0.h : TVE_HEIGHT_PAL;
#endif

		needroom1 = needroom = ((w * bpp + 7) >> 3) * h * ANDROID_NUMBER_OF_BUFFERS;
	}

	/* caculate the mem size of Foreground 1 */
	if (use_fg1_only || use_2layer_Fg){
		bpp = jz4760_lcd_info->osd.fg1.bpp;
		if (bpp == 18 || bpp == 24)
			bpp = 32;
		if (bpp == 15)
			bpp = 16;
#ifndef CONFIG_FB_JZ4760_TVE
		w = jz4760_lcd_info->osd.fg1.w;
		h = jz4760_lcd_info->osd.fg1.h;
#else	/* CONFIG_FB_JZ4760_TVE */
		w = (jz4760_lcd_info->osd.fg1.w > TVE_WIDTH_PAL) ? jz4760_lcd_info->osd.fg1.w : TVE_WIDTH_PAL;
		h = (jz4760_lcd_info->osd.fg1.h > TVE_HEIGHT_PAL) ? jz4760_lcd_info->osd.fg1.h : TVE_HEIGHT_PAL;
#endif
		needroom += ((w * bpp + 7) >> 3) * h;

	}
/* end of alloc Foreground 1 mem */
		needroom += PAGE_SIZE; // for
	/* Alloc memory */
		for (page_shift = 0; page_shift < 12; page_shift++)
			if ((PAGE_SIZE << page_shift) >= needroom)
				break;
//	fb_palette = (unsigned char *)__get_free_pages(GFP_KERNEL, 0);
		fb_palette = (unsigned char *)__get_free_pages(GFP_KERNEL, 1); // for init palette which need big room

		fb_frame = (unsigned char *)__get_free_pages(GFP_KERNEL, page_shift);
		if ((!fb_palette) || (!fb_frame))
			return -ENOMEM;
		memset((void *)fb_palette, 0, PAGE_SIZE);
		memset((void *)fb_frame, 0, PAGE_SIZE << page_shift);

//	memset((void *)fb_frame, 0xff, PAGE_SIZE << page_shift);

		lcd_palette = fb_palette;
//	dma_desc_base = (struct jz4760_lcd_dma_desc *)__get_free_pages(GFP_KERNEL, 0);
		dma_desc_base = (struct jz4760_lcd_dma_desc *)__get_free_pages(GFP_KERNEL, 1);

		/*
		 * Set page reserved so that mmap will work. This is necessary
		 * since we'll be remapping normal memory.
	 */
		page = (unsigned long)lcd_palette;
		SetPageReserved(virt_to_page((void*)page));

		page = (unsigned long)dma_desc_base;
		SetPageReserved(virt_to_page((void*)page));

		for (page = (unsigned long)fb_frame;
		     page < PAGE_ALIGN((unsigned long)fb_frame + (PAGE_SIZE<<page_shift));
		     page += PAGE_SIZE) {
			SetPageReserved(virt_to_page((void*)page));
		}

		if (use_fg1_only || use_2layer_Fg){

			lcd_frame = (unsigned char *)(((unsigned int)fb_frame + needroom1 + PAGE_MASK) & PAGE_MASK);
			cfb->fb.fix.smem_start = virt_to_phys((void *)lcd_frame);
			cfb->fb.fix.smem_len = needroom - needroom1; /* page_shift/2 ??? */
			cfb->fb.screen_base =
				(unsigned char *)(((unsigned int)lcd_frame&0x1fffffff) | 0xa0000000);
			if (!cfb->fb.screen_base) {
				printk("jz4760fb, %s: unable to map screen memory\n", cfb->fb.fix.id);
				return -ENOMEM;
			}
#if defined(CONFIG_JZ4760_EPSON_EPD_DISPLAY)

			memset((void *)lcd_frame, 0xff, cfb->fb.fix.smem_len);

			printk("%s %d set lcd frame 0xff\n",__FUNCTION__,__LINE__);
#endif
		}

		if (use_fg0_only || use_2layer_Fg){
			lcd_frame0 = fb_frame;
			cfb->fb0.fix.smem_start = virt_to_phys((void *)lcd_frame0);
			cfb->fb0.fix.smem_len = needroom1; /* page_shift/2 ??? */

			cfb->fb0.screen_base =
				(unsigned char *)(((unsigned int)lcd_frame0&0x1fffffff) | 0xa0000000);
			if (!cfb->fb0.screen_base) {
				printk("jz4760fb0, %s: unable to map screen memory\n", cfb->fb0.fix.id);
				return -ENOMEM;
			}
#if defined(CONFIG_JZ4760_EPSON_EPD_DISPLAY)

			memset((void *)lcd_frame0, 0xff, cfb->fb0.fix.smem_len);
			{
				unsigned char * newfb = (unsigned char *)lcd_frame0;
				unsigned char * oldfb = (unsigned char *)lcd_frame;
				fill_fb_newbuff(newfb);
				fill_fb_oldbuff(oldfb);
				check_fb_new_old_buf(newfb,oldfb);
			}
			dma_cache_wback_inv((unsigned int)lcd_frame0,800*600/2);
			dma_cache_wback_inv((unsigned int)lcd_frame,800*600/2);

#endif
		}

		return 0;
}

static void jz4760fb_free_fb_info(struct lcd_cfb_info *cfb)
{
	if (cfb) {
		fb_alloc_cmap(&cfb->fb.cmap, 0, 0);
		kfree(cfb);
	}
}

static void jz4760fb_unmap_smem(struct lcd_cfb_info *cfb)
{
	struct page * map = NULL;
	unsigned char *tmp;
	unsigned int page_shift, needroom, bpp, w, h;
	bpp = jz4760_lcd_info->osd.fg0.bpp;
	if ( bpp == 18 || bpp == 24)
		bpp = 32;
	if ( bpp == 15 )
		bpp = 16;
	w = jz4760_lcd_info->osd.fg0.w;
	h = jz4760_lcd_info->osd.fg0.h;
	needroom = ((w * bpp + 7) >> 3) * h;
	if (use_2layer_Fg){
		bpp = jz4760_lcd_info->osd.fg1.bpp;
		if ( bpp == 18 || bpp == 24)
			bpp = 32;
		if ( bpp == 15 )
			bpp = 16;
		w = jz4760_lcd_info->osd.fg1.w;
		h = jz4760_lcd_info->osd.fg1.h;
		needroom += ((w * bpp + 7) >> 3) * h;
	}

	for (page_shift = 0; page_shift < 12; page_shift++)
		if ((PAGE_SIZE << page_shift) >= needroom)
			break;

	if (cfb && cfb->fb.screen_base) {
		iounmap(cfb->fb.screen_base);
		cfb->fb.screen_base = NULL;
		release_mem_region(cfb->fb.fix.smem_start,
				   cfb->fb.fix.smem_len);
	}

	if (lcd_palette) {
		map = virt_to_page(lcd_palette);
		clear_bit(PG_reserved, &map->flags);
		free_pages((int)lcd_palette, 0);
	}

	if (lcd_frame0) {
		for (tmp=(unsigned char *)lcd_frame0;
		     tmp < lcd_frame0 + (PAGE_SIZE << page_shift);
		     tmp += PAGE_SIZE) {
			map = virt_to_page(tmp);
			clear_bit(PG_reserved, &map->flags);
		}
		free_pages((int)lcd_frame0, page_shift);
	}
}

/************************************
 *      Jz475X Chipset OPS
 ************************************/

/*
 * switch to tve mode from lcd mode
 * mode:
 * 	PANEL_MODE_TVE_PAL: switch to TVE_PAL mode
 * 	PANEL_MODE_TVE_NTSC: switch to TVE_NTSC mode
 */
static void print_lcdc_registers(void)	/* debug */
{
#ifdef  DEBUG
	/* LCD Controller Resgisters */
	printk("REG_LCD_CFG:\t0x%08x\n", REG_LCD_CFG);
	printk("REG_LCD_CTRL:\t0x%08x\n", REG_LCD_CTRL);
	printk("REG_LCD_STATE:\t0x%08x\n", REG_LCD_STATE);
	printk("REG_LCD_OSDC:\t0x%08x\n", REG_LCD_OSDC);
	printk("REG_LCD_OSDCTRL:\t0x%08x\n", REG_LCD_OSDCTRL);
	printk("REG_LCD_OSDS:\t0x%08x\n", REG_LCD_OSDS);
	printk("REG_LCD_BGC:\t0x%08x\n", REG_LCD_BGC);
	printk("REG_LCD_KEK0:\t0x%08x\n", REG_LCD_KEY0);
	printk("REG_LCD_KEY1:\t0x%08x\n", REG_LCD_KEY1);
	printk("REG_LCD_ALPHA:\t0x%08x\n", REG_LCD_ALPHA);
	printk("REG_LCD_IPUR:\t0x%08x\n", REG_LCD_IPUR);
	printk("REG_LCD_VAT:\t0x%08x\n", REG_LCD_VAT);
	printk("REG_LCD_DAH:\t0x%08x\n", REG_LCD_DAH);
	printk("REG_LCD_DAV:\t0x%08x\n", REG_LCD_DAV);
	printk("REG_LCD_XYP0:\t0x%08x\n", REG_LCD_XYP0);
	printk("REG_LCD_XYP1:\t0x%08x\n", REG_LCD_XYP1);
	printk("REG_LCD_SIZE0:\t0x%08x\n", REG_LCD_SIZE0);
	printk("REG_LCD_SIZE1:\t0x%08x\n", REG_LCD_SIZE1);
	printk("REG_LCD_RGBC\t0x%08x\n", REG_LCD_RGBC);
	printk("REG_LCD_VSYNC:\t0x%08x\n", REG_LCD_VSYNC);
	printk("REG_LCD_HSYNC:\t0x%08x\n", REG_LCD_HSYNC);
	printk("REG_LCD_PS:\t0x%08x\n", REG_LCD_PS);
	printk("REG_LCD_CLS:\t0x%08x\n", REG_LCD_CLS);
	printk("REG_LCD_SPL:\t0x%08x\n", REG_LCD_SPL);
	printk("REG_LCD_REV:\t0x%08x\n", REG_LCD_REV);
	printk("REG_LCD_IID:\t0x%08x\n", REG_LCD_IID);
	printk("REG_LCD_DA0:\t0x%08x\n", REG_LCD_DA0);
	printk("REG_LCD_SA0:\t0x%08x\n", REG_LCD_SA0);
	printk("REG_LCD_FID0:\t0x%08x\n", REG_LCD_FID0);
	printk("REG_LCD_CMD0:\t0x%08x\n", REG_LCD_CMD0);
	printk("REG_LCD_OFFS0:\t0x%08x\n", REG_LCD_OFFS0);
	printk("REG_LCD_PW0:\t0x%08x\n", REG_LCD_PW0);
	printk("REG_LCD_CNUM0:\t0x%08x\n", REG_LCD_CNUM0);
	printk("REG_LCD_DESSIZE0:\t0x%08x\n", REG_LCD_DESSIZE0);
	printk("REG_LCD_DA1:\t0x%08x\n", REG_LCD_DA1);
	printk("REG_LCD_SA1:\t0x%08x\n", REG_LCD_SA1);
	printk("REG_LCD_FID1:\t0x%08x\n", REG_LCD_FID1);
	printk("REG_LCD_CMD1:\t0x%08x\n", REG_LCD_CMD1);
	printk("REG_LCD_OFFS1:\t0x%08x\n", REG_LCD_OFFS1);
	printk("REG_LCD_PW1:\t0x%08x\n", REG_LCD_PW1);
	printk("REG_LCD_CNUM1:\t0x%08x\n", REG_LCD_CNUM1);
	printk("REG_LCD_DESSIZE1:\t0x%08x\n", REG_LCD_DESSIZE1);
	printk("==================================\n");
	printk("REG_LCD_VSYNC:\t%d:%d\n", REG_LCD_VSYNC>>16, REG_LCD_VSYNC&0xfff);
	printk("REG_LCD_HSYNC:\t%d:%d\n", REG_LCD_HSYNC>>16, REG_LCD_HSYNC&0xfff);
	printk("REG_LCD_VAT:\t%d:%d\n", REG_LCD_VAT>>16, REG_LCD_VAT&0xfff);
	printk("REG_LCD_DAH:\t%d:%d\n", REG_LCD_DAH>>16, REG_LCD_DAH&0xfff);
	printk("REG_LCD_DAV:\t%d:%d\n", REG_LCD_DAV>>16, REG_LCD_DAV&0xfff);
	printk("==================================\n");

	/* Smart LCD Controller Resgisters */
	printk("REG_SLCD_CFG:\t0x%08x\n", REG_SLCD_CFG);
	printk("REG_SLCD_CTRL:\t0x%08x\n", REG_SLCD_CTRL);
	printk("REG_SLCD_STATE:\t0x%08x\n", REG_SLCD_STATE);
	printk("==================================\n");

	/* TVE Controller Resgisters */
	printk("REG_TVE_CTRL:\t0x%08x\n", REG_TVE_CTRL);
	printk("REG_TVE_FRCFG:\t0x%08x\n", REG_TVE_FRCFG);
	printk("REG_TVE_SLCFG1:\t0x%08x\n", REG_TVE_SLCFG1);
	printk("REG_TVE_SLCFG2:\t0x%08x\n", REG_TVE_SLCFG2);
	printk("REG_TVE_SLCFG3:\t0x%08x\n", REG_TVE_SLCFG3);
	printk("REG_TVE_LTCFG1:\t0x%08x\n", REG_TVE_LTCFG1);
	printk("REG_TVE_LTCFG2:\t0x%08x\n", REG_TVE_LTCFG2);
	printk("REG_TVE_CFREQ:\t0x%08x\n", REG_TVE_CFREQ);
	printk("REG_TVE_CPHASE:\t0x%08x\n", REG_TVE_CPHASE);
	printk("REG_TVE_CBCRCFG:\t0x%08x\n", REG_TVE_CBCRCFG);
	printk("REG_TVE_WSSCR:\t0x%08x\n", REG_TVE_WSSCR);
	printk("REG_TVE_WSSCFG1:\t0x%08x\n", REG_TVE_WSSCFG1);
	printk("REG_TVE_WSSCFG2:\t0x%08x\n", REG_TVE_WSSCFG2);
	printk("REG_TVE_WSSCFG3:\t0x%08x\n", REG_TVE_WSSCFG3);

	printk("==================================\n");

	if ( 1 ) {
		unsigned int * pii = (unsigned int *)dma_desc_base;
		int i, j;
		//for (j=0;j< DMA_DESC_NUM ; j++) {
		for (j=0;j< NR_DMA_DESC_EPD ; j++) {
			printk("dma_desc%d(0x%08x):\n", j, (unsigned int)pii);
			for (i =0; i<8; i++ ) {
				printk("\t\t0x%08x\n", *pii++);
			}
		}
	}



#endif
}

#ifdef CONFIG_FB_JZ4760_TVE
static void jz4760lcd_info_switch_to_TVE(int mode)
{
	struct jz4760lcd_info *info;
	struct jz4760lcd_osd_t *osd_lcd;
	struct jz4760lcd_panel_t *panel_lcd;
	int x, y, w, h;

	info = &jz4760_info_tve;

	/* Set to tve mode */
	info->panel.cfg	= jz4760_lcd_panel.panel.cfg;
	info->panel.cfg |= LCD_CFG_TVEN | LCD_CFG_MODE_INTER_CCIR656; /* Interlace CCIR656 mode */
	info->panel.ctrl = jz4760_lcd_panel.panel.ctrl;
	info->osd.rgb_ctrl = LCD_RGBC_YCC; /* enable RGB => YUV */

	/* Copy current to keep the old style */
	osd_lcd = &jz4760_lcd_panel.osd;
	panel_lcd = &jz4760_lcd_panel.panel;

	switch ( mode ) {
	case PANEL_MODE_TVE_PAL:
		info->panel.cfg |= LCD_CFG_TVEPEH; /* TVE PAL enable extra halfline signal */
		info->panel.w = TVE_WIDTH_PAL;
		info->panel.h = TVE_HEIGHT_PAL;
		info->panel.fclk = TVE_FREQ_PAL;

		/* set Foreground 0 */
		w = osd_lcd->fg0.w / panel_lcd->w * TVE_WIDTH_PAL;
		h = osd_lcd->fg0.h / panel_lcd->h * TVE_HEIGHT_PAL;
		x = osd_lcd->fg0.x / panel_lcd->w * TVE_WIDTH_PAL;
		y = osd_lcd->fg0.y / panel_lcd->h * TVE_HEIGHT_PAL;
		info->osd.fg0.bpp = osd_lcd->fg0.bpp;
		info->osd.fg0.x = x;
		info->osd.fg0.y = y;
		info->osd.fg0.w = w;
		info->osd.fg0.h = h;

		/* set Foreground 1 */
		w = osd_lcd->fg1.w / panel_lcd->w * TVE_WIDTH_PAL;
		h = osd_lcd->fg1.h / panel_lcd->h * TVE_HEIGHT_PAL;
		x = osd_lcd->fg1.x / panel_lcd->w * TVE_WIDTH_PAL;
		y = osd_lcd->fg1.y / panel_lcd->h * TVE_HEIGHT_PAL;
		info->osd.fg1.bpp = 32;	/* use RGB888 in TVE mode*/
		info->osd.fg1.x = x;
		info->osd.fg1.y = y;
		info->osd.fg1.w = w;
		info->osd.fg1.h = h;
		break;

	case PANEL_MODE_TVE_NTSC:
		info->panel.cfg &= ~LCD_CFG_TVEPEH; /* TVE NTSC disable extra halfline signal */
		info->panel.w = TVE_WIDTH_NTSC;
		info->panel.h = TVE_HEIGHT_NTSC;
		info->panel.fclk = TVE_FREQ_NTSC;
		w = osd_lcd->fg0.w / panel_lcd->w * TVE_WIDTH_PAL;
		h = osd_lcd->fg0.h / panel_lcd->h * TVE_HEIGHT_NTSC;
		x = osd_lcd->fg0.x / panel_lcd->w * TVE_WIDTH_NTSC;
		y = osd_lcd->fg0.y / panel_lcd->h * TVE_HEIGHT_NTSC;
		info->osd.fg0.bpp = osd_lcd->fg0.bpp;
		info->osd.fg0.x = x;
		info->osd.fg0.y = y;
		info->osd.fg0.w = w;
		info->osd.fg0.h = h;

		w = osd_lcd->fg1.w / panel_lcd->w * TVE_WIDTH_PAL;
		h = osd_lcd->fg1.h / panel_lcd->h * TVE_HEIGHT_NTSC;
		x = osd_lcd->fg1.x / panel_lcd->w * TVE_WIDTH_NTSC;
		y = osd_lcd->fg1.y / panel_lcd->h * TVE_HEIGHT_NTSC;
		info->osd.fg1.bpp = 32;	/* use RGB888 int TVE mode */
		info->osd.fg1.x = x;
		info->osd.fg1.y = y;
		info->osd.fg1.w = w;
		info->osd.fg1.h = h;
		break;
	default:
		printk("%s, %s: Unknown tve mode\n", __FILE__, __FUNCTION__);
	}
	jz4760_lcd_info = &jz4760_info_tve;
}

/*
 * switch to lcd mode from TVE mode
 */

static void jz4760lcd_info_switch_to_lcd(void)
{
	struct jz4760lcd_info *info;
	struct jz4760lcd_osd_t *osd_lcd;
	struct jz4760lcd_panel_t *panel_lcd;
	int x, y, w, h;

	info = &jz4760_lcd_panel;

	/* set to tve mode */
	info->panel.cfg	= jz4760_info_tve.panel.cfg;
	info->panel.cfg &= ~(LCD_CFG_TVEN | LCD_CFG_MODE_INTER_CCIR656); /* Interlace CCIR656 mode */
	info->panel.ctrl = jz4760_info_tve.panel.ctrl;
	info->osd.rgb_ctrl &= ~LCD_RGBC_YCC; /* enable YUV => RGB*/

	/*  */
	osd_lcd = &jz4760_info_tve.osd;
	panel_lcd = &jz4760_info_tve.panel;

	/* set Foreground 0 */
	w = osd_lcd->fg0.w / panel_lcd->w * info->panel.w;
	h = osd_lcd->fg0.h / panel_lcd->h * info->panel.h;
	x = osd_lcd->fg0.x / panel_lcd->w * info->panel.w;
	y = osd_lcd->fg0.y / panel_lcd->h * info->panel.h;
	info->osd.fg0.x = x;
	info->osd.fg0.y = y;
	info->osd.fg0.w = w;
	info->osd.fg0.h = h;

	/* set Foreground 1 */
	w = osd_lcd->fg1.w / panel_lcd->w * info->panel.w;
	h = osd_lcd->fg1.h / panel_lcd->h * info->panel.h;
	x = osd_lcd->fg1.x / panel_lcd->w * info->panel.w;
	y = osd_lcd->fg1.y / panel_lcd->h * info->panel.h;
	info->osd.fg1.x = x;
	info->osd.fg1.y = y;
	info->osd.fg1.w = w;
	info->osd.fg1.h = h;

	jz4760_lcd_info = &jz4760_lcd_panel;
}
#endif
/*for epd: we use 12 palettes, with 16 per palette, tatally 192 frames . BY Cynthia zhzhao 20100602*/

static void jz4760fb_epd_descriptor_init( struct jz4760lcd_info * lcd_info )
{
	unsigned int pal_size;
	int fg0_line_size, fg0_frm_size, fg1_line_size, fg1_frm_size;
	int size0, size1;
	int i;
	unsigned int palette_id ;//which palette
//	pal_size = palette_offset;
	pal_size = 1024;
	/* DMA Descriptor. */
	for (i = 0; i < NR_DMA_DESC_EPD + NR_DMA1_DESC_EPD; i++)
		dma_desc_epd[i] = dma_desc_base+i;

	/* Foreground 0, caculate size */
	if ( lcd_info->osd.fg0.x >= lcd_info->panel.w )
		lcd_info->osd.fg0.x = lcd_info->panel.w - 1;
	if ( lcd_info->osd.fg0.y >= lcd_info->panel.h )
		lcd_info->osd.fg0.y = lcd_info->panel.h - 1;
	if ( lcd_info->osd.fg0.x + lcd_info->osd.fg0.w > lcd_info->panel.w )
		lcd_info->osd.fg0.w = lcd_info->panel.w - lcd_info->osd.fg0.x;
	if ( lcd_info->osd.fg0.y + lcd_info->osd.fg0.h > lcd_info->panel.h )
		lcd_info->osd.fg0.h = lcd_info->panel.h - lcd_info->osd.fg0.y;

	size0 = lcd_info->osd.fg0.h << 16 | lcd_info->osd.fg0.w;
	fg0_line_size = (lcd_info->osd.fg0.w * (lcd_info->osd.fg0.bpp) / 8);
	fg0_line_size = ((fg0_line_size + 3) >> 2) << 2; /* word aligned */
	fg0_frm_size = fg0_line_size * lcd_info->osd.fg0.h;

	dma0_desc_palette = dma_desc_epd[0];

	/*The first Palette Descriptor */
	palette_id=0;
	dma0_desc_palette_epd[palette_id]= dma_desc_epd[0];
	dma0_desc_palette_epd[palette_id]->next_desc 	= (unsigned int)virt_to_phys(dma_desc_epd[palette_id+1]);
	dma0_desc_palette_epd[palette_id]->databuf 	= (unsigned int)virt_to_phys((void *)(lcd_palette + palette_offset*palette_id));
	dma0_desc_palette_epd[palette_id]->frame_id 	= (unsigned int)0xaaaaaaaa;
	dma0_desc_palette_epd[palette_id]->cmd 		= LCD_CMD_PAL | pal_size; /* Palette Descriptor */
	dma0_desc_palette_epd[palette_id]->offsize	= 0x00000000; /* Palette Descriptor */

//	printk("dma0_desc_palette_epd[%d]->databuf addr(%p) desc(%p) nex_Desc (%p) frame_id (0x%08x) cmd (0x%08x) \n",palette_id,(void*)dma0_desc_palette_epd[palette_id]->databuf,(void *)dma0_desc_palette_epd[palette_id],(void *)dma0_desc_palette_epd[palette_id]->next_desc,dma0_desc_palette_epd[palette_id]->frame_id ,dma0_desc_palette_epd[palette_id]->cmd);



       /* next */
	for(i=1;i<NR_DMA_DESC_EPD_PER_GROUP;i++){

		dma0_desc0 = dma_desc_epd[i];
		if (i != (NR_DMA_DESC_EPD_PER_GROUP*(palette_id+1) - 1))
			dma0_desc0->next_desc = (unsigned int)virt_to_phys(dma0_desc0+1);
		else{
			dma0_desc_palette_epd[palette_id+1] = dma0_desc0+1;
			dma0_desc0->next_desc = (unsigned int)virt_to_phys(dma0_desc_palette_epd[palette_id+1]);
		}
                /* frame phys addr */
		dma0_desc0->databuf = virt_to_phys((void *)(lcd_frame0));
		/* frame id */
		dma0_desc0->frame_id = (unsigned int)0x0000da00; /* DMA0'0 */
		/* others */
		dma0_desc0->cmd = fg0_frm_size/4;
		dma0_desc0->offsize =0;
		dma0_desc0->page_width = 0;
		dma0_desc0->desc_size = size0;

//		printk("dma_desc_epd[%d](0x%08x)  databuf(0x%08x) nex_Desc(0x%08x) frame_id(0x%08x) cmd(0x%08x) \n",i,dma_desc_epd[i],dma0_desc0->databuf,dma0_desc0->next_desc,dma0_desc0->frame_id ,dma0_desc0->cmd);

	}


	/*The second Palette Descriptor */
	palette_id=1;

	dma0_desc_palette_epd[palette_id]->next_desc 	= (unsigned int)virt_to_phys(dma_desc_epd[NR_DMA_DESC_EPD_PER_GROUP*palette_id+1]);
	dma0_desc_palette_epd[palette_id]->databuf 	= (unsigned int)virt_to_phys((void *)(lcd_palette + palette_offset*palette_id));
	dma0_desc_palette_epd[palette_id]->frame_id 	= (unsigned int)0xaaaaaaaa;
	dma0_desc_palette_epd[palette_id]->cmd 		= LCD_CMD_PAL | pal_size; /* Palette Descriptor */
	dma0_desc_palette_epd[palette_id]->offsize	= 0x00000000; /* Palette Descriptor */

	//printk("dma0_desc_palette_epd[%d]->databuf addr(%p) nex_Desc (%p) frame_id (0x%08x) cmd (0x%08x) \n",palette_id,(void*)dma0_desc_palette_epd[palette_id]->databuf,(void *)dma0_desc_palette_epd[palette_id]->next_desc,dma0_desc_palette_epd[palette_id]->frame_id ,dma0_desc_palette_epd[palette_id]->cmd);



       /* next */
	for(i=(NR_DMA_DESC_EPD_PER_GROUP*palette_id+1);i<(NR_DMA_DESC_EPD_PER_GROUP*(palette_id+1));i++){

		dma0_desc0 = dma_desc_epd[i];
		if (i != (NR_DMA_DESC_EPD_PER_GROUP*(palette_id+1) - 1))
			dma0_desc0->next_desc = (unsigned int)virt_to_phys(dma0_desc0+1);
		else{
			dma0_desc_palette_epd[palette_id+1] = dma0_desc0+1;/* frame phys addr */
			dma0_desc0->next_desc = (unsigned int)virt_to_phys(dma0_desc_palette_epd[palette_id+1]);
		}
		dma0_desc0->databuf = virt_to_phys((void *)lcd_frame0);
		/* frame id */
		dma0_desc0->frame_id = (unsigned int)0x0000da00; /* DMA0'0 */
		/* others */
		dma0_desc0->cmd = fg0_frm_size/4;
		dma0_desc0->offsize =0;
		dma0_desc0->page_width = 0;
		dma0_desc0->desc_size = size0;

		//printk("dma_desc_epd[%d](0x%08x)  databuf(0x%08x) nex_Desc(0x%08x) frame_id(0x%08x) cmd(0x%08x) \n",i,dma_desc_epd[i],dma0_desc0->databuf,dma0_desc0->next_desc,dma0_desc0->frame_id ,dma0_desc0->cmd);

	}


	/* The third Palette Descriptor */
	palette_id=2;

	dma0_desc_palette_epd[palette_id]->next_desc 	= (unsigned int)virt_to_phys(dma_desc_epd[NR_DMA_DESC_EPD_PER_GROUP*palette_id+1]);
	dma0_desc_palette_epd[palette_id]->databuf 	= (unsigned int)virt_to_phys((void *)(lcd_palette + palette_offset*palette_id));
	dma0_desc_palette_epd[palette_id]->frame_id 	= (unsigned int)0xaaaaaaaa;
	dma0_desc_palette_epd[palette_id]->cmd 		= LCD_CMD_PAL | pal_size; /* Palette Descriptor */
	dma0_desc_palette_epd[palette_id]->offsize	= 0x00000000; /* Palette Descriptor */

	//printk("dma0_desc_palette_epd[%d]->databuf addr(%p) nex_Desc (%p) frame_id (0x%08x) cmd (0x%08x) \n",palette_id,(void*)dma0_desc_palette_epd[palette_id]->databuf,(void *)dma0_desc_palette_epd[palette_id]->next_desc,dma0_desc_palette_epd[palette_id]->frame_id ,dma0_desc_palette_epd[palette_id]->cmd);



       /* next */
	for(i=(NR_DMA_DESC_EPD_PER_GROUP*palette_id+1);i<(NR_DMA_DESC_EPD_PER_GROUP*(palette_id+1));i++){

		dma0_desc0 = dma_desc_epd[i];
		if (i != (NR_DMA_DESC_EPD_PER_GROUP*(palette_id+1) - 1))
			dma0_desc0->next_desc = (unsigned int)virt_to_phys(dma0_desc0+1);
		else{
			dma0_desc_palette_epd[palette_id+1] = dma0_desc0+1;/* frame phys addr */
			dma0_desc0->next_desc = (unsigned int)virt_to_phys(dma0_desc_palette_epd[palette_id+1]);
		}
		dma0_desc0->databuf = virt_to_phys((void *)lcd_frame0);
		/* frame id */
		dma0_desc0->frame_id = (unsigned int)0x0000da00; /* DMA0'0 */
		/* others */
		dma0_desc0->cmd = fg0_frm_size/4;
		dma0_desc0->offsize =0;
		dma0_desc0->page_width = 0;
		dma0_desc0->desc_size = size0;
		//printk("dma_desc_epd[%d](0x%08x)  databuf(0x%08x) nex_Desc(0x%08x) frame_id(0x%08x) cmd(0x%08x) \n",i,dma_desc_epd[i],dma0_desc0->databuf,dma0_desc0->next_desc,dma0_desc0->frame_id ,dma0_desc0->cmd);


	}

	/* The forth Palette Descriptor */
	palette_id=3;

	dma0_desc_palette_epd[palette_id]->next_desc 	= (unsigned int)virt_to_phys(dma_desc_epd[NR_DMA_DESC_EPD_PER_GROUP*palette_id+1]);
	dma0_desc_palette_epd[palette_id]->databuf 	= (unsigned int)virt_to_phys((void *)(lcd_palette + palette_offset*palette_id));
	dma0_desc_palette_epd[palette_id]->frame_id 	= (unsigned int)0xaaaaaaaa;
	dma0_desc_palette_epd[palette_id]->cmd 		= LCD_CMD_PAL | pal_size; /* Palette Descriptor */
	dma0_desc_palette_epd[palette_id]->offsize	= 0x00000000; /* Palette Descriptor */

	//printk("dma0_desc_palette_epd[%d]->databuf addr(%p) nex_Desc (%p) frame_id (0x%08x) cmd (0x%08x) \n",palette_id,(void*)dma0_desc_palette_epd[palette_id]->databuf,(void *)dma0_desc_palette_epd[palette_id]->next_desc,dma0_desc_palette_epd[palette_id]->frame_id ,dma0_desc_palette_epd[palette_id]->cmd);



       /* next */
	for(i=(NR_DMA_DESC_EPD_PER_GROUP*palette_id+1);i<(NR_DMA_DESC_EPD_PER_GROUP*(palette_id+1));i++){

		dma0_desc0 = dma_desc_epd[i];
		if (i != (NR_DMA_DESC_EPD_PER_GROUP*(palette_id+1) - 1))
			dma0_desc0->next_desc = (unsigned int)virt_to_phys(dma0_desc0+1);
		else{
			dma0_desc_palette_epd[palette_id+1] = dma0_desc0+1;/* frame phys addr */
			dma0_desc0->next_desc = (unsigned int)virt_to_phys(dma0_desc_palette_epd[palette_id+1]);
		}
		dma0_desc0->databuf = virt_to_phys((void *)lcd_frame0);
		/* frame id */
		dma0_desc0->frame_id = (unsigned int)0x0000da00; /* DMA0'0 */
		/* others */
		dma0_desc0->cmd = fg0_frm_size/4;
		dma0_desc0->offsize =0;
		dma0_desc0->page_width = 0;
		dma0_desc0->desc_size = size0;

		//	printk("dma_desc_epd[%d](0x%08x)  databuf(0x%08x) nex_Desc(0x%08x) frame_id(0x%08x) cmd(0x%08x) \n",i,dma_desc_epd[i],dma0_desc0->databuf,dma0_desc0->next_desc,dma0_desc0->frame_id ,dma0_desc0->cmd);

	}

	/* The fifth Palette Descriptor */
	palette_id=4;

	dma0_desc_palette_epd[palette_id]->next_desc 	= (unsigned int)virt_to_phys(dma_desc_epd[NR_DMA_DESC_EPD_PER_GROUP*palette_id+1]);
	dma0_desc_palette_epd[palette_id]->databuf 	= (unsigned int)virt_to_phys((void *)(lcd_palette + palette_offset*palette_id));
	dma0_desc_palette_epd[palette_id]->frame_id 	= (unsigned int)0xaaaaaaaa;
	dma0_desc_palette_epd[palette_id]->cmd 		= LCD_CMD_PAL | pal_size; /* Palette Descriptor */
	dma0_desc_palette_epd[palette_id]->offsize	= 0x00000000; /* Palette Descriptor */

	//printk("dma0_desc_palette_epd[%d]->databuf addr(%p) nex_Desc (%p) frame_id (0x%08x) cmd (0x%08x) \n",palette_id,(void*)dma0_desc_palette_epd[palette_id]->databuf,(void *)dma0_desc_palette_epd[palette_id]->next_desc,dma0_desc_palette_epd[palette_id]->frame_id ,dma0_desc_palette_epd[palette_id]->cmd);



       /* next */
	for(i=(NR_DMA_DESC_EPD_PER_GROUP*palette_id+1);i<(NR_DMA_DESC_EPD_PER_GROUP*(palette_id+1));i++){

		dma0_desc0 = dma_desc_epd[i];
		if (i != (NR_DMA_DESC_EPD_PER_GROUP*(palette_id+1) - 1))
			dma0_desc0->next_desc = (unsigned int)virt_to_phys(dma0_desc0+1);
		else{
			dma0_desc_palette_epd[palette_id+1] = dma0_desc0+1;/* frame phys addr */
			dma0_desc0->next_desc = (unsigned int)virt_to_phys(dma0_desc_palette_epd[palette_id+1]);
		}
		dma0_desc0->databuf = virt_to_phys((void *)lcd_frame0);
		/* frame id */
		dma0_desc0->frame_id = (unsigned int)0x0000da00; /* DMA0'0 */
		/* others */
		dma0_desc0->cmd = fg0_frm_size/4;
		dma0_desc0->offsize =0;
		dma0_desc0->page_width = 0;
		dma0_desc0->desc_size = size0;
		//printk("dma0_desc0->databuf 0x%08x nex_Desc (0x%08x) frame_id (0x%08x) cmd (0x%08x) \n",dma0_desc0->databuf,dma0_desc0->next_desc,dma0_desc0->frame_id ,dma0_desc0->cmd);



	}

	/* The sixth Palette Descriptor */
	palette_id=5;

	dma0_desc_palette_epd[palette_id]->next_desc 	= (unsigned int)virt_to_phys(dma_desc_epd[NR_DMA_DESC_EPD_PER_GROUP*palette_id+1]);
	dma0_desc_palette_epd[palette_id]->databuf 	= (unsigned int)virt_to_phys((void *)(lcd_palette + palette_offset*palette_id));
	dma0_desc_palette_epd[palette_id]->frame_id 	= (unsigned int)0xaaaaaaaa;
	dma0_desc_palette_epd[palette_id]->cmd 		= LCD_CMD_PAL | pal_size; /* Palette Descriptor */
	dma0_desc_palette_epd[palette_id]->offsize	= 0x00000000; /* Palette Descriptor */

	//printk("dma0_desc_palette_epd[%d]->databuf addr(%p) nex_Desc (%p) frame_id (0x%08x) cmd (0x%08x) \n",palette_id,(void*)dma0_desc_palette_epd[palette_id]->databuf,(void *)dma0_desc_palette_epd[palette_id]->next_desc,dma0_desc_palette_epd[palette_id]->frame_id ,dma0_desc_palette_epd[palette_id]->cmd);



       /* next */
	for(i=(NR_DMA_DESC_EPD_PER_GROUP*palette_id+1);i<(NR_DMA_DESC_EPD_PER_GROUP*(palette_id+1));i++){

		dma0_desc0 = dma_desc_epd[i];
		if (i != (NR_DMA_DESC_EPD_PER_GROUP*(palette_id+1) - 1))
			dma0_desc0->next_desc = (unsigned int)virt_to_phys(dma0_desc0+1);
		else{
			dma0_desc_palette_epd[palette_id+1] = dma0_desc0+1;/* frame phys addr */
			dma0_desc0->next_desc = (unsigned int)virt_to_phys(dma0_desc_palette_epd[palette_id+1]);
		}
		dma0_desc0->databuf = virt_to_phys((void *)lcd_frame0);
		/* frame id */
		dma0_desc0->frame_id = (unsigned int)0x0000da00; /* DMA0'0 */
		/* others */
		dma0_desc0->cmd = fg0_frm_size/4;
		dma0_desc0->offsize =0;
		dma0_desc0->page_width = 0;
		dma0_desc0->desc_size = size0;
		//printk("dma0_desc0->databuf 0x%08x nex_Desc (0x%08x) frame_id (0x%08x) cmd (0x%08x) \n",dma0_desc0->databuf,dma0_desc0->next_desc,dma0_desc0->frame_id ,dma0_desc0->cmd);


	}



	/* The seventh Palette Descriptor */
	palette_id=6;

	dma0_desc_palette_epd[palette_id]->next_desc 	= (unsigned int)virt_to_phys(dma_desc_epd[NR_DMA_DESC_EPD_PER_GROUP*palette_id+1]);
	dma0_desc_palette_epd[palette_id]->databuf 	= (unsigned int)virt_to_phys((void *)(lcd_palette + palette_offset*palette_id));
	dma0_desc_palette_epd[palette_id]->frame_id 	= (unsigned int)0xaaaaaaaa;
	dma0_desc_palette_epd[palette_id]->cmd 		= LCD_CMD_PAL | pal_size; /* Palette Descriptor */
	dma0_desc_palette_epd[palette_id]->offsize	= 0x00000000; /* Palette Descriptor */

	//printk("dma0_desc_palette_epd[%d]->databuf addr(%p) nex_Desc (%p) frame_id (0x%08x) cmd (0x%08x) \n",palette_id,(void*)dma0_desc_palette_epd[palette_id]->databuf,(void *)dma0_desc_palette_epd[palette_id]->next_desc,dma0_desc_palette_epd[palette_id]->frame_id ,dma0_desc_palette_epd[palette_id]->cmd);


       /* next */
	for(i=(NR_DMA_DESC_EPD_PER_GROUP*palette_id+1);i<(NR_DMA_DESC_EPD_PER_GROUP*(palette_id+1));i++){

		dma0_desc0 = dma_desc_epd[i];
		if (i != (NR_DMA_DESC_EPD_PER_GROUP*(palette_id+1) - 1))
			dma0_desc0->next_desc = (unsigned int)virt_to_phys(dma0_desc0+1);
		else{
			dma0_desc_palette_epd[palette_id+1] = dma0_desc0+1;/* frame phys addr */
			dma0_desc0->next_desc = (unsigned int)virt_to_phys(dma0_desc_palette_epd[palette_id+1]);

		}
		dma0_desc0->databuf = virt_to_phys((void *)lcd_frame0);
		/* frame id */
		dma0_desc0->frame_id = (unsigned int)0x0000da00; /* DMA0'0 */
		/* others */
		dma0_desc0->cmd = fg0_frm_size/4;
		dma0_desc0->offsize =0;
		dma0_desc0->page_width = 0;
		dma0_desc0->desc_size = size0;
		//printk("dma0_desc0->databuf 0x%08x nex_Desc (0x%08x) frame_id (0x%08x) cmd (0x%08x) \n",dma0_desc0->databuf,dma0_desc0->next_desc,dma0_desc0->frame_id ,dma0_desc0->cmd);

	}



       /* The eighth Palette Descriptor */
	palette_id=7;

	dma0_desc_palette_epd[palette_id]->next_desc 	= (unsigned int)virt_to_phys(dma_desc_epd[NR_DMA_DESC_EPD_PER_GROUP*palette_id+1]);
	dma0_desc_palette_epd[palette_id]->databuf 	= (unsigned int)virt_to_phys((void *)(lcd_palette + palette_offset*palette_id));
	dma0_desc_palette_epd[palette_id]->frame_id 	= (unsigned int)0xaaaaaaaa;
	dma0_desc_palette_epd[palette_id]->cmd 		= LCD_CMD_PAL | pal_size; /* Palette Descriptor */
	dma0_desc_palette_epd[palette_id]->offsize	= 0x00000000; /* Palette Descriptor */

	//printk("dma0_desc_palette_epd[%d]->databuf addr(%p) nex_Desc (%p) frame_id (0x%08x) cmd (0x%08x) \n",palette_id,(void*)dma0_desc_palette_epd[palette_id]->databuf,(void *)dma0_desc_palette_epd[palette_id]->next_desc,dma0_desc_palette_epd[palette_id]->frame_id ,dma0_desc_palette_epd[palette_id]->cmd);


       /* next */
	for(i=(NR_DMA_DESC_EPD_PER_GROUP*palette_id+1);i<(NR_DMA_DESC_EPD_PER_GROUP*(palette_id+1));i++){

		dma0_desc0 = dma_desc_epd[i];
		if (i != (NR_DMA_DESC_EPD_PER_GROUP*(palette_id+1) - 1))
			dma0_desc0->next_desc = (unsigned int)virt_to_phys(dma0_desc0+1);
		else{
			dma0_desc_palette_epd[palette_id+1] = dma0_desc0+1;/* frame phys addr */
			dma0_desc0->next_desc = (unsigned int)virt_to_phys(dma0_desc_palette_epd[palette_id+1]);

		}
		dma0_desc0->databuf = virt_to_phys((void *)lcd_frame0);
		/* frame id */
		dma0_desc0->frame_id = (unsigned int)0x0000da00; /* DMA0'0 */
		/* others */
		dma0_desc0->cmd = fg0_frm_size/4;
		dma0_desc0->offsize =0;
		dma0_desc0->page_width = 0;
		dma0_desc0->desc_size = size0;
		//printk("dma0_desc0->databuf 0x%08x nex_Desc (0x%08x) frame_id (0x%08x) cmd (0x%08x) \n",dma0_desc0->databuf,dma0_desc0->next_desc,dma0_desc0->frame_id ,dma0_desc0->cmd);


	}


	/* The ninth Palette Descriptor */
	palette_id=8;

	dma0_desc_palette_epd[palette_id]->next_desc 	= (unsigned int)virt_to_phys(dma_desc_epd[NR_DMA_DESC_EPD_PER_GROUP*palette_id+1]);
	dma0_desc_palette_epd[palette_id]->databuf 	= (unsigned int)virt_to_phys((void *)(lcd_palette + palette_offset*palette_id));
	dma0_desc_palette_epd[palette_id]->frame_id 	= (unsigned int)0xaaaaaaaa;
	dma0_desc_palette_epd[palette_id]->cmd 		= LCD_CMD_PAL | pal_size; /* Palette Descriptor */
	dma0_desc_palette_epd[palette_id]->offsize	= 0x00000000; /* Palette Descriptor */

	//printk("dma0_desc_palette_epd[%d]->databuf addr(%p) nex_Desc (%p) frame_id (0x%08x) cmd (0x%08x) \n",palette_id,(void*)dma0_desc_palette_epd[palette_id]->databuf,(void *)dma0_desc_palette_epd[palette_id]->next_desc,dma0_desc_palette_epd[palette_id]->frame_id ,dma0_desc_palette_epd[palette_id]->cmd);


       /* next */
	for(i=(NR_DMA_DESC_EPD_PER_GROUP*palette_id+1);i<(NR_DMA_DESC_EPD_PER_GROUP*(palette_id+1));i++){

		dma0_desc0 = dma_desc_epd[i];
		if (i != (NR_DMA_DESC_EPD_PER_GROUP*(palette_id+1) - 1))
			dma0_desc0->next_desc = (unsigned int)virt_to_phys(dma0_desc0+1);
		else{
			dma0_desc_palette_epd[palette_id+1] = dma0_desc0+1;/* frame phys addr */
			dma0_desc0->next_desc = (unsigned int)virt_to_phys(dma0_desc_palette_epd[palette_id+1]);

		}
		dma0_desc0->databuf = virt_to_phys((void *)lcd_frame0);
		/* frame id */
		dma0_desc0->frame_id = (unsigned int)0x0000da00; /* DMA0'0 */
		/* others */
		dma0_desc0->cmd = fg0_frm_size/4;
		dma0_desc0->offsize =0;
		dma0_desc0->page_width = 0;
		dma0_desc0->desc_size = size0;
		//printk("dma0_desc0->databuf 0x%08x nex_Desc (0x%08x) frame_id (0x%08x) cmd (0x%08x) \n",dma0_desc0->databuf,dma0_desc0->next_desc,dma0_desc0->frame_id ,dma0_desc0->cmd);



	}


	/* The tenth Palette Descriptor */
	palette_id=9;

	dma0_desc_palette_epd[palette_id]->next_desc 	= (unsigned int)virt_to_phys(dma_desc_epd[NR_DMA_DESC_EPD_PER_GROUP*palette_id+1]);
	dma0_desc_palette_epd[palette_id]->databuf 	= (unsigned int)virt_to_phys((void *)(lcd_palette + palette_offset*palette_id));
	dma0_desc_palette_epd[palette_id]->frame_id 	= (unsigned int)0xaaaaaaaa;
	dma0_desc_palette_epd[palette_id]->cmd 		= LCD_CMD_PAL | pal_size; /* Palette Descriptor */
	dma0_desc_palette_epd[palette_id]->offsize	= 0x00000000; /* Palette Descriptor */

	//printk("dma0_desc_palette_epd[%d]->databuf addr(%p) nex_Desc (%p) frame_id (0x%08x) cmd (0x%08x) \n",palette_id,(void*)dma0_desc_palette_epd[palette_id]->databuf,(void *)dma0_desc_palette_epd[palette_id]->next_desc,dma0_desc_palette_epd[palette_id]->frame_id ,dma0_desc_palette_epd[palette_id]->cmd);



       /* next */
	for(i=(NR_DMA_DESC_EPD_PER_GROUP*palette_id+1);i<(NR_DMA_DESC_EPD_PER_GROUP*(palette_id+1));i++){

		dma0_desc0 = dma_desc_epd[i];
		if (i != (NR_DMA_DESC_EPD_PER_GROUP*(palette_id+1) - 1))
			dma0_desc0->next_desc = (unsigned int)virt_to_phys(dma0_desc0+1);
		else{
			dma0_desc_palette_epd[palette_id+1] = dma0_desc0+1;/* frame phys addr */
			dma0_desc0->next_desc = (unsigned int)virt_to_phys(dma0_desc_palette_epd[palette_id+1]);

		}
		dma0_desc0->databuf = virt_to_phys((void *)lcd_frame0);
		/* frame id */
		dma0_desc0->frame_id = (unsigned int)0x0000da00; /* DMA0'0 */
		/* others */
		dma0_desc0->cmd = fg0_frm_size/4;
		dma0_desc0->offsize =0;
		dma0_desc0->page_width = 0;
		dma0_desc0->desc_size = size0;
		//printk("dma0_desc0->databuf 0x%08x nex_Desc (0x%08x) frame_id (0x%08x) cmd (0x%08x) \n",dma0_desc0->databuf,dma0_desc0->next_desc,dma0_desc0->frame_id ,dma0_desc0->cmd);


	}


	/* The eleventh Palette Descriptor */
	palette_id=10;

	dma0_desc_palette_epd[palette_id]->next_desc 	= (unsigned int)virt_to_phys(dma_desc_epd[NR_DMA_DESC_EPD_PER_GROUP*palette_id+1]);
	dma0_desc_palette_epd[palette_id]->databuf 	= (unsigned int)virt_to_phys((void *)(lcd_palette + palette_offset*palette_id));
	dma0_desc_palette_epd[palette_id]->frame_id 	= (unsigned int)0xaaaaaaaa;
	dma0_desc_palette_epd[palette_id]->cmd 		= LCD_CMD_PAL | pal_size; /* Palette Descriptor */
	dma0_desc_palette_epd[palette_id]->offsize	= 0x00000000; /* Palette Descriptor */

	//printk("dma0_desc_palette_epd[%d]->databuf addr(%p) nex_Desc (%p) frame_id (0x%08x) cmd (0x%08x) \n",palette_id,(void*)dma0_desc_palette_epd[palette_id]->databuf,(void *)dma0_desc_palette_epd[palette_id]->next_desc,dma0_desc_palette_epd[palette_id]->frame_id ,dma0_desc_palette_epd[palette_id]->cmd);



       /* next */
	for(i=(NR_DMA_DESC_EPD_PER_GROUP*palette_id+1);i<(NR_DMA_DESC_EPD_PER_GROUP*(palette_id+1));i++){

		dma0_desc0 = dma_desc_epd[i];
		if (i != (NR_DMA_DESC_EPD_PER_GROUP*(palette_id+1) - 1))
			dma0_desc0->next_desc = (unsigned int)virt_to_phys(dma0_desc0+1);
		else{
			dma0_desc_palette_epd[palette_id+1] = dma0_desc0+1;/* frame phys addr */
			dma0_desc0->next_desc = (unsigned int)virt_to_phys(dma0_desc_palette_epd[palette_id+1]);
		}
		dma0_desc0->databuf = virt_to_phys((void *)lcd_frame0);
		/* frame id */
		dma0_desc0->frame_id = (unsigned int)0x0000da00; /* DMA0'0 */
		/* others */
		dma0_desc0->cmd = fg0_frm_size/4;
		dma0_desc0->offsize =0;
		dma0_desc0->page_width = 0;
		dma0_desc0->desc_size = size0;
		//printk("dma0_desc0->databuf 0x%08x nex_Desc (0x%08x) frame_id (0x%08x) cmd (0x%08x) \n",dma0_desc0->databuf,dma0_desc0->next_desc,dma0_desc0->frame_id ,dma0_desc0->cmd);


	}


	/* The tewlvth Palette Descriptor */
	palette_id=11;

	dma0_desc_palette_epd[palette_id]->next_desc 	= (unsigned int)virt_to_phys(dma_desc_epd[NR_DMA_DESC_EPD_PER_GROUP*palette_id+1]);
	dma0_desc_palette_epd[palette_id]->databuf 	= (unsigned int)virt_to_phys((void *)((lcd_palette + palette_offset*palette_id)));
	dma0_desc_palette_epd[palette_id]->frame_id 	= (unsigned int)0xaaaaaaaa;
	dma0_desc_palette_epd[palette_id]->cmd 		= LCD_CMD_PAL | pal_size; /* Palette Descriptor */
	dma0_desc_palette_epd[palette_id]->offsize	= 0x00000000; /* Palette Descriptor */

	//printk("dma0_desc_palette_epd[%d]->databuf addr(%p) nex_Desc (%p) frame_id (0x%08x) cmd (0x%08x) \n",palette_id,(void*)dma0_desc_palette_epd[palette_id]->databuf,(void *)dma0_desc_palette_epd[palette_id]->next_desc,dma0_desc_palette_epd[palette_id]->frame_id ,dma0_desc_palette_epd[palette_id]->cmd);



      /* next */
	for(i=(NR_DMA_DESC_EPD_PER_GROUP*palette_id+1);i<(NR_DMA_DESC_EPD_PER_GROUP*(palette_id+1));i++){

		dma0_desc0 = dma_desc_epd[i];
		if (i != (NR_DMA_DESC_EPD_PER_GROUP*(palette_id+1) - 1))
			dma0_desc0->next_desc = (unsigned int)virt_to_phys(dma0_desc0+1);

		else{
			dma0_desc0->next_desc = (unsigned int)virt_to_phys(dma_desc_epd[0]);

		}

		dma0_desc0->databuf = virt_to_phys((void *)lcd_frame0);
               /* frame id */
		dma0_desc0->frame_id = (unsigned int)0x0000da00; /* DMA0'0 */
		/* others */
		dma0_desc0->cmd = fg0_frm_size/4;
		dma0_desc0->offsize =0;
		dma0_desc0->page_width = 0;
		dma0_desc0->desc_size = size0;
		//	printk("dma0_desc0->databuf 0x%08x nex_Desc (0x%08x) frame_id (0x%08x) cmd (0x%08x) \n",dma0_desc0->databuf,dma0_desc0->next_desc,dma0_desc0->frame_id ,dma0_desc0->cmd);



	}


	REG_LCD_DA0 = virt_to_phys(dma0_desc_palette_epd[0]);

#if EPD_MODE_PAL
	REG_LCD_SIZE0 = size0;
#else
	dma0_desc0->cmd = 0;
	dma0_desc0->desc_size = 0;
	REG_LCD_DA0 = virt_to_phys(dma0_desc0); //tft
	REG_LCD_SIZE0 = 0;

#endif
	current_dma0_id = 0;//dma0_desc0;

        /*===========  fg1 descriptor ===========*/
	dma1_desc0 = dma_desc_base + NR_DMA_DESC_EPD;
	/* Foreground 1, caculate size */
	if ( lcd_info->osd.fg1.x >= lcd_info->panel.w )
		lcd_info->osd.fg1.x = lcd_info->panel.w - 1;
	if ( lcd_info->osd.fg1.y >= lcd_info->panel.h )
		lcd_info->osd.fg1.y = lcd_info->panel.h - 1;
	if ( lcd_info->osd.fg1.x + lcd_info->osd.fg1.w > lcd_info->panel.w )
		lcd_info->osd.fg1.w = lcd_info->panel.w - lcd_info->osd.fg1.x;
	if ( lcd_info->osd.fg1.y + lcd_info->osd.fg1.h > lcd_info->panel.h )
		lcd_info->osd.fg1.h = lcd_info->panel.h - lcd_info->osd.fg1.y;


	size1 = lcd_info->osd.fg1.h << 16 | lcd_info->osd.fg1.w;
	fg1_line_size = lcd_info->osd.fg1.w*lcd_info->osd.fg1.bpp/8;
	fg1_line_size = ((fg1_line_size+3)>>2)<<2; /* word aligned */
	fg1_frm_size = fg1_line_size * lcd_info->osd.fg1.h;
	printk("%s: fg1_frm_size: %d \n",__func__,fg1_frm_size/4);
	dma1_desc0->next_desc = (unsigned int)virt_to_phys(dma1_desc0);
			/* frame phys addr */
	dma1_desc0->databuf  = virt_to_phys((void *)lcd_frame);

		/* frame id */
	dma1_desc0->frame_id = (unsigned int)0x0da10000; /* DMA1'0 */

	dma1_desc0->cmd = fg1_frm_size/4;
	dma1_desc0->offsize = 0;
	dma1_desc0->page_width = 0;
	dma1_desc0->desc_size = size1;

	//printk("dma1_desc0(0x%08x) databuf(0x%08x) nex_Desc(0x%08x) frame_id(0x%08x) cmd(0x%08x) \n",dma1_desc0,dma1_desc0->databuf,dma1_desc0->next_desc,dma1_desc0->frame_id ,dma1_desc0->cmd);

	REG_LCD_DA1 = virt_to_phys(dma1_desc0);	/* set Dma-chan1's Descripter Addrress */
	REG_LCD_SIZE1 = size1;
	current_dma1_id = 0;//dma1_desc0;


	dma_cache_wback((unsigned int)(dma_desc_base), (NR_DMA_DESC_EPD + NR_DMA1_DESC_EPD)*sizeof(struct jz4760_lcd_dma_desc));
}

static void jz4760fb_set_panel_mode(struct jz4760lcd_info * lcd_info)
{
	struct jz4760lcd_panel_t *panel = &lcd_info->panel;

	/* set bpp */
	lcd_info->panel.ctrl &= ~LCD_CTRL_BPP_MASK;
	if ( lcd_info->osd.fg0.bpp == 1 )
		lcd_info->panel.ctrl |= LCD_CTRL_BPP_1;
	else if ( lcd_info->osd.fg0.bpp == 2 )
		lcd_info->panel.ctrl |= LCD_CTRL_BPP_2;
	else if ( lcd_info->osd.fg0.bpp == 4 )
		lcd_info->panel.ctrl |= LCD_CTRL_BPP_4;
	else if ( lcd_info->osd.fg0.bpp == 8 )
		lcd_info->panel.ctrl |= LCD_CTRL_BPP_8;
	else if ( lcd_info->osd.fg0.bpp == 15 ) {
		lcd_info->osd.fg0.bpp = 16;
		lcd_info->panel.ctrl |= LCD_CTRL_BPP_16 | LCD_CTRL_RGB555;
	}
	else if ( lcd_info->osd.fg0.bpp == 16 ) {
		lcd_info->panel.ctrl |= LCD_CTRL_BPP_16 | LCD_CTRL_RGB565;
		lcd_info->panel.ctrl &= ~LCD_CTRL_RGB555;
	}
	else if ( lcd_info->osd.fg0.bpp == 30) {
		lcd_info->osd.fg0.bpp = 32;
		lcd_info->panel.ctrl |= LCD_CTRL_BPP_30;
	}
	else if ( lcd_info->osd.fg0.bpp > 16 && lcd_info->osd.fg0.bpp < 32+1 ) {
		lcd_info->osd.fg0.bpp = 32;
		lcd_info->panel.ctrl |= LCD_CTRL_BPP_18_24;
	}
	else {
		printk("The BPP %d is not supported\n", lcd_info->osd.fg0.bpp);
		lcd_info->osd.fg0.bpp = 32;
		lcd_info->panel.ctrl |= LCD_CTRL_BPP_18_24;
	}

	lcd_info->panel.cfg |= LCD_CFG_NEWDES; /* use 8words descriptor always */
	REG_LCD_CTRL = lcd_info->panel.ctrl; /* LCDC Controll Register */

	REG_LCD_CFG = lcd_info->panel.cfg; /* LCDC Configure Register */
	REG_SLCD_CFG = lcd_info->panel.slcd_cfg; /* Smart LCD Configure Register */

	if ( lcd_info->panel.cfg & LCD_CFG_LCDPIN_SLCD ) /* enable Smart LCD DMA */
		REG_SLCD_CTRL = SLCD_CTRL_DMA_EN;

	switch ( lcd_info->panel.cfg & LCD_CFG_MODE_MASK ) {
	case LCD_CFG_MODE_GENERIC_TFT:
	case LCD_CFG_MODE_INTER_CCIR656:
	case LCD_CFG_MODE_NONINTER_CCIR656:
	case LCD_CFG_MODE_SLCD:
	default:		/* only support TFT16 TFT32, not support STN and Special TFT by now(10-06-2008)*/
		REG_LCD_VAT = (((panel->blw + panel->w + panel->elw + panel->hsw)) << 16) | (panel->vsw + panel->bfw + panel->h + panel->efw);
		REG_LCD_DAH = ((panel->hsw + panel->blw) << 16) | (panel->hsw + panel->blw + panel->w);
		REG_LCD_DAV = ((panel->vsw + panel->bfw) << 16) | (panel->vsw + panel->bfw + panel->h);
		REG_LCD_HSYNC = (0 << 16) | panel->hsw;
		REG_LCD_VSYNC = (0 << 16) | panel->vsw;
		break;
	}
}


//static void jz4760fb_set_osd_mode( struct jz4760lcd_info * lcd_info )
static void jz4760fb_set_osd_mode(struct jz4760lcd_osd_t *lcd_osd_info)
{
	lcd_osd_info->osd_ctrl &= ~(LCD_OSDCTRL_OSDBPP_MASK);
	if(lcd_osd_info->fg1.bpp == 2)
		lcd_osd_info->osd_ctrl |= LCD_OSDCTRL_OSDBPP_2;
	else if(lcd_osd_info->fg1.bpp == 4)
		lcd_osd_info->osd_ctrl |= LCD_OSDCTRL_OSDBPP_4;
	else if ( lcd_osd_info->fg1.bpp == 15 )
		lcd_osd_info->osd_ctrl |= LCD_OSDCTRL_OSDBPP_15_16|LCD_OSDCTRL_RGB555;
	else if ( lcd_osd_info->fg1.bpp == 16 ) {
		lcd_osd_info->osd_ctrl |= LCD_OSDCTRL_OSDBPP_15_16;
		lcd_osd_info->osd_ctrl &= ~LCD_OSDCTRL_RGB555;
	}
	else if (lcd_osd_info->fg1.bpp == 30) {
		lcd_osd_info->osd_ctrl |= LCD_OSDCTRL_OSDBPP_30;
	}
	else {
		lcd_osd_info->fg1.bpp = 32;
		lcd_osd_info->osd_ctrl |= LCD_OSDCTRL_OSDBPP_18_24;
	}


	REG_LCD_OSDC 	= lcd_osd_info->osd_cfg; /* F0, F1, alpha, */

	REG_LCD_OSDCTRL = lcd_osd_info->osd_ctrl; /* IPUEN, bpp */
	REG_LCD_RGBC  	= lcd_osd_info->rgb_ctrl;
	REG_LCD_BGC  	= lcd_osd_info->bgcolor;
	REG_LCD_KEY0 	= lcd_osd_info->colorkey0;
	REG_LCD_KEY1 	= lcd_osd_info->colorkey1;
	REG_LCD_ALPHA 	= lcd_osd_info->alpha;
	REG_LCD_IPUR 	= lcd_osd_info->ipu_restart;
}



/* Change Position of Foreground 0 */
static int jz4760fb0_foreground_move(struct jz4760lcd_osd_t *lcd_osd_info)
{
	int pos;
	int j, count;
	/*
	 * Foreground, only one of the following can be change at one time:
	 * 	1. F0 size, 2. F0 position, 3. F1 size, 4. F1 position
	 *
	 * The rules of fg0 position:
	 * 	fg0.x + fg0.w <= panel.w;
	 * 	fg0.y + fg0.h <= panel.h;
	 *
	 * When output is LCD panel, fg.y can be odd number or even number.
	 * When output is TVE, as the TVE has odd frame and even frame,
	 * to simplified operation, fg.y should be even number always.
	 *
	 */

	/* Foreground 0  */
	if (lcd_osd_info->fg0.x + lcd_osd_info->fg0.w > jz4760_lcd_info->panel.w)
		lcd_osd_info->fg0.x = jz4760_lcd_info->panel.w - lcd_osd_info->fg0.w;
	if (lcd_osd_info->fg0.y + lcd_osd_info->fg0.h > jz4760_lcd_info->panel.h)
		lcd_osd_info->fg0.y = jz4760_lcd_info->panel.h - lcd_osd_info->fg0.h;

	if (lcd_osd_info->fg0.x >= jz4760_lcd_info->panel.w)
		lcd_osd_info->fg0.x = jz4760_lcd_info->panel.w - 1;
	if (lcd_osd_info->fg0.y >= jz4760_lcd_info->panel.h)
		lcd_osd_info->fg0.y = jz4760_lcd_info->panel.h - 1;

	pos = lcd_osd_info->fg0.y << 16 | lcd_osd_info->fg0.x;
	if (REG_LCD_XYP0 == pos){
		printk("FG0: same position\n");
		return 0;
	}

	REG_LCD_XYP0 = pos;
	REG_LCD_OSDCTRL |= LCD_OSDCTRL_CHANGES;
	while(!(REG_LCD_OSDS & LCD_OSDS_READY));
	j = count;
	msleep(40);
	while((REG_LCD_OSDCTRL & LCD_OSDCTRL_CHANGES) && j--);
	if(j == 0) {
		printk("Error FG0 Position: Wait change fail.\n");
		return -EFAULT;
	}

	return 0;
}
/* Change Window size of Foreground 0 */
static int jz4760fb0_foreground_resize(struct jz4760lcd_osd_t *lcd_osd_info)
{
	struct lcd_cfb_info *cfb = jz4760fb_info;
	int size, fg0_line_size, fg0_frm_size;
//	int desc_len = sizeof(struct jz4760_lcd_dma_desc);
	/*
	 * NOTE:
	 * Foreground change sequence:
	 * 	1. Change Position Registers -> LCD_OSDCTL.Change;
	 * 	2. LCD_OSDCTRL.Change -> descripter->Size
	 * Foreground, only one of the following can be change at one time:
	 * 	1. F0 size;
	 *	2. F0 position
	 * 	3. F1 size
	 *	4. F1 position
	 */

	/*
	 * The rules of f0, f1's position:
	 * 	f0.x + f0.w <= panel.w;
	 * 	f0.y + f0.h <= panel.h;
	 *
	 * When output is LCD panel, fg.y and fg.h can be odd number or even number.
	 * When output is TVE, as the TVE has odd frame and even frame,
	 * to simplified operation, fg.y and fg.h should be even number always.
	 *
	 */
	/* Foreground 0  */
	if (lcd_osd_info->fg0.x + lcd_osd_info->fg0.w > jz4760_lcd_info->panel.w)
		lcd_osd_info->fg0.w = jz4760_lcd_info->panel.w - lcd_osd_info->fg0.x;
	if (lcd_osd_info->fg0.y + lcd_osd_info->fg0.h > jz4760_lcd_info->panel.h)
		lcd_osd_info->fg0.h = jz4760_lcd_info->panel.h - lcd_osd_info->fg0.y;

	size = lcd_osd_info->fg0.h << 16 | lcd_osd_info->fg0.w;

	if (REG_LCD_SIZE0 == size) {
		printk("FG0: same size\n");
		return 0;
	}

	fg0_line_size = lcd_osd_info->fg0.w * lcd_osd_info->fg0.bpp / 8;
	fg0_line_size = ((fg0_line_size + 3) >> 2) << 2; /* word aligned */
	fg0_frm_size = fg0_line_size * lcd_osd_info->fg0.h;

	REG_LCD_OSDCTRL |= LCD_OSDCTRL_CHANGES;
	/* set change bit */
	REG_LCD_OSDCTRL |= LCD_OSDCTRL_CHANGES;

	if (jz4760_lcd_info->panel.cfg & LCD_CFG_TVEN ) { /* output to TV */
		dma0_desc0->cmd = dma0_desc1->cmd = (fg0_frm_size/4)/2;
		dma0_desc0->offsize = dma0_desc1->offsize
			= fg0_line_size/4;
		dma0_desc0->page_width = dma0_desc1->page_width
			= fg0_line_size/4;
		dma0_desc1->databuf = virt_to_phys((void *)(lcd_frame0 + fg0_line_size));
	}
	else {
		dma0_desc0->cmd = dma0_desc1->cmd = fg0_frm_size/4;
		dma0_desc0->offsize = dma0_desc1->offsize =0;
		dma0_desc0->page_width = dma0_desc1->page_width = 0;
		}

	dma0_desc0->desc_size = dma0_desc1->desc_size = size;
//			= lcd_osd_info->fg0.h << 16 | lcd_osd_info->fg0.w;
	REG_LCD_SIZE0 = size;
//	REG_LCD_SIZE0 = (lcd_osd_info->fg0.h << 16) | lcd_osd_info->fg0.w;

	dma_cache_wback((unsigned int)(dma_desc_base), (DMA_DESC_NUM)*sizeof(struct jz4760_lcd_dma_desc));

	jz4760fb_set_var(&cfb->fb0.var, 0, &cfb->fb0);
	return 0;
}

/* Change Position of Foreground 1 */
static int jz4760fb_foreground_move(struct jz4760lcd_osd_t *lcd_osd_info)
{
	int pos;
	int j, count = 100000;
	/*
	 * Foreground, only one of the following can be change at one time:
	 * 	1. F0 size, 2. F0 position, 3. F1 size, 4. F1 position
	 *
	 * The rules of fg1 position:
	 * 	fg1.x + fg1.w <= panel.w;
	 * 	fg1.y + fg1.h <= panel.h;
	 *
	 * When output is LCD panel, fg.y can be odd number or even number.
	 * When output is TVE, as the TVE has odd frame and even frame,
	 * to simplified operation, fg.y should be even number always.
	 *
	 */

	/* Foreground 0  */
	if (lcd_osd_info->fg1.x + lcd_osd_info->fg1.w > jz4760_lcd_info->panel.w)
		lcd_osd_info->fg1.x = jz4760_lcd_info->panel.w - lcd_osd_info->fg1.w;
	if (lcd_osd_info->fg1.y + lcd_osd_info->fg1.h > jz4760_lcd_info->panel.h)
		lcd_osd_info->fg1.y = jz4760_lcd_info->panel.h - lcd_osd_info->fg1.h;

	if (lcd_osd_info->fg1.x >= jz4760_lcd_info->panel.w)
		lcd_osd_info->fg1.x = jz4760_lcd_info->panel.w - 1;
	if (lcd_osd_info->fg1.y >= jz4760_lcd_info->panel.h)
		lcd_osd_info->fg1.y = jz4760_lcd_info->panel.h - 1;

	pos = lcd_osd_info->fg1.y << 16 | lcd_osd_info->fg1.x;
	if (REG_LCD_XYP1 == pos){
		printk("FG1: same position\n");
		return 0;
	}

	/*********************************************/
	REG_LCD_XYP1 = pos;
	REG_LCD_OSDCTRL |= LCD_OSDCTRL_CHANGES;
	while(!(REG_LCD_OSDS & LCD_OSDS_READY));
	j = count;
	msleep(40);
	while((REG_LCD_OSDCTRL & LCD_OSDCTRL_CHANGES) && j--);
	if(j == 0) {
		printk("Error FG1 Position: Wait change fail.\n");
		return -EFAULT;

	}
	return 0;
}

/* Change window size of Foreground 1 */
static int jz4760fb_foreground_resize(struct jz4760lcd_osd_t *lcd_osd_info)
{
	struct lcd_cfb_info *cfb = jz4760fb_info;
	int size, fg1_line_size, fg1_frm_size;
//	int desc_len = sizeof(struct jz4760_lcd_dma_desc);
	/*
	 * NOTE:
	 * Foreground change sequence:
	 * 	1. Change Position Registers -> LCD_OSDCTL.Change;
	 * 	2. LCD_OSDCTRL.Change -> descripter->Size
	 * Foreground, only one of the following can be change at one time:
	 * 	1. F0 size;
	 *	2. F0 position
	 * 	3. F1 size
	 *	4. F1 position
	 */

	/*
	 * The rules of f0, f1's position:
	 * 	f0.x + f0.w <= panel.w;
	 * 	f0.y + f0.h <= panel.h;
	 *
	 * When output is LCD panel, fg.y and fg.h can be odd number or even number.
	 * When output is TVE, as the TVE has odd frame and even frame,
	 * to simplified operation, fg.y and fg.h should be even number always.
	 *
	 */

	/* Foreground 0  */
	if (lcd_osd_info->fg1.x + lcd_osd_info->fg1.w > jz4760_lcd_info->panel.w)
		lcd_osd_info->fg1.w = jz4760_lcd_info->panel.w - lcd_osd_info->fg1.x;
	if (lcd_osd_info->fg1.y + lcd_osd_info->fg1.h > jz4760_lcd_info->panel.h)
		lcd_osd_info->fg1.h = jz4760_lcd_info->panel.h - lcd_osd_info->fg1.y;
//	size = lcd_info->osd.fg1.h << 16|lcd_info->osd.fg1.w;
	size = lcd_osd_info->fg1.h << 16|lcd_osd_info->fg1.w;
	if (REG_LCD_SIZE1 == size) {
		printk("FG1: same size\n");
		return 0;// -EFAULT;
	}

//	fg1_line_size = lcd_osd_info->fg1.w * ((lcd_osd_info->fg1.bpp + 7) / 8);
	fg1_line_size = lcd_osd_info->fg1.w * lcd_osd_info->fg1.bpp / 8;
	fg1_line_size = ((fg1_line_size + 3) >> 2) << 2; /* word aligned */
	fg1_frm_size = fg1_line_size * lcd_osd_info->fg1.h;

	/* set change bit */
	REG_LCD_OSDCTRL |= LCD_OSDCTRL_CHANGES;
	if ( jz4760_lcd_info->panel.cfg & LCD_CFG_TVEN ) { /* output to TV */
		dma1_desc0->cmd = dma1_desc1->cmd = (fg1_frm_size/4)/2;
		dma1_desc0->offsize = dma1_desc1->offsize = fg1_line_size/4;
		dma1_desc0->page_width = dma1_desc1->page_width = fg1_line_size/4;
		dma1_desc1->databuf = virt_to_phys((void *)(lcd_frame + fg1_line_size));
	}
	else {
		dma1_desc0->cmd = dma1_desc1->cmd = fg1_frm_size/4;
		dma1_desc0->offsize = dma1_desc1->offsize = 0;
		dma1_desc0->page_width = dma1_desc1->page_width = 0;
	}

	dma1_desc0->desc_size = dma1_desc1->desc_size = size;
	REG_LCD_SIZE1 = size;

	dma_cache_wback((unsigned int)(dma_desc_base), (DMA_DESC_NUM)*sizeof(struct jz4760_lcd_dma_desc));

	jz4760fb_set_var(&cfb->fb.var, 1, &cfb->fb);
	return 0;
}



/*
 * Set lcd pixel clock
 */
static void jz4760fb_change_clock( struct jz4760lcd_info * lcd_info )
{
#if defined(CONFIG_FPGA)   /* FPGA test, pixdiv */
//	REG_LCD_REV = 0x0000002;
	REG_LCD_REV = 0x0000001;
	printk("Fuwa test, pixclk divide REG_LCD_REV=0x%08x\n", REG_LCD_REV);
	printk("Fuwa test, pixclk %d\n", JZ_EXTAL/(((REG_LCD_REV&0xFF)+1)*2));
#else
	unsigned int val = 0;
	unsigned int pclk;
	/* Timing setting */
	__cpm_stop_lcd();

	val = lcd_info->panel.fclk; /* frame clk */

	if ( (lcd_info->panel.cfg & LCD_CFG_MODE_MASK) != LCD_CFG_MODE_SERIAL_TFT) {
		pclk = val * (lcd_info->panel.w + lcd_info->panel.hsw + lcd_info->panel.elw + lcd_info->panel.blw) * (lcd_info->panel.h + lcd_info->panel.vsw + lcd_info->panel.efw + lcd_info->panel.bfw); /* Pixclk */
	}
	else {
		/* serial mode: Hsync period = 3*Width_Pixel */
		pclk = val * (lcd_info->panel.w*3 + lcd_info->panel.hsw + lcd_info->panel.elw + lcd_info->panel.blw) * (lcd_info->panel.h + lcd_info->panel.vsw + lcd_info->panel.efw + lcd_info->panel.bfw); /* Pixclk */
	}

	/********* In TVE mode PCLK = 27MHz ***********/
	if ( lcd_info->panel.cfg & LCD_CFG_TVEN ) { 		/* LCDC output to TVE */
		pclk = 27000000;
		__cpm_select_pixclk_tve();
	}
	else { 		/* LCDC output to  LCD panel */
		__cpm_select_pixclk_lcd();
	}

#if defined(CONFIG_JZ4760_EPSON_EPD_DISPLAY)
	pclk = 30000000;
	val = __cpm_get_pllout2() / pclk; /* pclk */
#else
	val = __cpm_get_pllout2() / pclk; /* pclk */
#endif

	val--;
	dprintk("ratio: val = %d\n", val);
	if ( val > 0x7ff ) {
		printk("pixel clock divid is too large, set it to 0x7ff\n");
		val = 0x7ff;
	}


	__cpm_set_pixdiv(val);
//	REG_CPM_LPCDR = 0x0000000e;
	dprintk("REG_CPM_LPCDR = 0x%08x\n", REG_CPM_LPCDR);
	__cpm_enable_pll_change();

	dprintk("REG_CPM_LPCDR=0x%08x\n", REG_CPM_LPCDR);
	dprintk("REG_CPM_CPCCR=0x%08x\n", REG_CPM_CPCCR);

	jz_clocks.pixclk = __cpm_get_pixclk();
	printk("LCDC: PixClock:%d\n", jz_clocks.pixclk);
	__cpm_start_lcd();
	udelay(1000);

	/*
	 * set lcd device clock and lcd pixel clock.
	 * what about TVE mode???
	 *
	 */
#endif

}


/*
 * jz4760fb_deep_set_mode,
 *
 */
static void jz4760fb_deep_set_mode( struct jz4760lcd_info * lcd_info )
{
	/* configurate sequence:
	 * 1. disable lcdc.
	 * 2. init frame descriptor.
	 * 3. set panel mode
	 * 4. set osd mode
	 * 5. start lcd clock in CPM
	 * 6. enable lcdc.
	 */
	struct lcd_cfb_info *cfb = jz4760fb_info;

//	__lcd_clr_ena();	/* Quick Disable */
	lcd_info->osd.fg_change = FG_CHANGE_ALL; /* change FG0, FG1 size, postion??? */
	jz4760fb_set_osd_mode(&lcd_info->osd);
	jz4760fb_set_panel_mode(lcd_info);

	jz4760fb_epd_descriptor_init(lcd_info);

	jz4760fb_change_clock(lcd_info);
if (use_fg0_only || use_2layer_Fg)
	jz4760fb_set_var(&cfb->fb0.var, 0, &cfb->fb0);

if (use_fg1_only || use_2layer_Fg)
	jz4760fb_set_var(&cfb->fb.var, 1, &cfb->fb);

//	__lcd_set_ena();	/* enable lcdc */
}


static irqreturn_t jz4760fb_interrupt_handler(int irq, void *dev_id)
{
	unsigned int state;
	static int irqcnt=0;
	static int framecnt = 0;

	state = REG_LCD_STATE;
//	printk("-------------In the lcd interrupt handler, state=0x%x--------\n", state);
	if (state & EPD_STATE_PWRUP) {
		REG_LCD_STATE = state & ~EPD_STATE_PWRUP;
		mdelay(1);
#if EPD_MODE_PAL
		{
			//	REG_LCD_DA0 = virt_to_phys(dma0_desc_palette);
			REG_LCD_DA0 = virt_to_phys(dma0_desc_palette_epd[framecnt]);
			REG_LCD_DA1 = virt_to_phys(dma1_desc0);

			//REG_SLCD_CTRL |= (1 << 1);
			REG_SLCD_CTRL = 0;
			__lcd_set_ena();	/* enalbe LCD Controller */
		}
#else
		{
			dma0_desc0->cmd = 0;
			REG_SLCD_CTRL = 0;
			__lcd_set_ena();	/* enalbe LCD Controller */
		}

#endif
//		printk("power on!");
		REG_LCD_CTRL |= LCD_CTRL_PEDN;
		REG_EPD_CTRL4 |= EPD_CTRL4_FEN;
	}
	if (state & EPD_STATE_PWRDN) {
		REG_LCD_STATE = state & ~EPD_STATE_PWRDN;
//		printk("EPD Power Down interrupt\n");
	}

	if (state & EPD_STATE_FEND) {
		REG_LCD_STATE = state & ~EPD_STATE_FEND;
		/* IC only support 16 frames, in this situation epd works well,
		   but for more than 16 frames we should do sth
		   Added by Cynthia */

                __lcd_clr_ena();

		 if( totally_time > (++framecnt)){
			 REG_LCD_DA0 = virt_to_phys(dma0_desc_palette_epd[framecnt]);
			 REG_LCD_DA1 = virt_to_phys(dma1_desc0);
			 REG_SLCD_CTRL |= SLCD_CTRL_DMA_START;
			 REG_EPD_CTRL4 |= EPD_CTRL4_FEN;
 //			printk("totally_time=%d , framecnt=%d \n",totally_time, framecnt);
		 }
		 else{

			 framecnt =0;
			 REG_EPD_CTRL4 &= ~EPD_CTRL4_FEN;
			 REG_EPD_CTRL2 |= EPD_CTRL2_PWROFF;
		 }


		 __lcd_set_ena();
//		 printk("EPD Frame End interrupt state = 0x%x cnt= %d \n ", state,framecnt);

/* ended by Cynthia */


	}
	if (state & LCD_STATE_EOF) /* End of frame */
	{
		REG_LCD_STATE = state & ~LCD_STATE_EOF;
		//	irqcnt++;
		printk("======== End of frame =  \n");

	}
	if (state & LCD_STATE_IFU0) {
		//	printk("%s, In FiFo0 underrun\n", __FUNCTION__);
		REG_LCD_STATE = state & ~LCD_STATE_IFU0;

	}

	if (state & LCD_STATE_IFU1) {
		REG_LCD_STATE = state & ~LCD_STATE_IFU1;
//		printk("%s, InFiFo1 underrun\n", __FUNCTION__);

	}

	if (state & LCD_STATE_OFU) { /* Out fifo underrun */
		REG_LCD_STATE = state & ~LCD_STATE_OFU;
		if ( irqcnt++ > 100 ) {
			//__lcd_disable_ofu_intr();
			//printk("disable Out FiFo underrun irq.\n");
		}
		printk("%s, Out FiFo underrun.\n", __FUNCTION__);
		//
//	printk("REG_LCD_CMD0:\t0x%08x, REG_LCD_CMD1:\t0x%08x\n", REG_LCD_CMD0,REG_LCD_CMD1);
	}
	return IRQ_HANDLED;
}

#ifdef CONFIG_PM

/*
 * Suspend the LCDC.
 */
static int jz4760fb_suspend(struct platform_device *pdev, pm_message_t state)
{
	__lcd_clr_ena(); /* Quick Disable */
	__lcd_display_off();
	__cpm_stop_lcd();

	return 0;
}

/*
 * Resume the LCDC.
 */
static int jz4760fb_resume(struct platform_device *pdev)
{
	__cpm_start_lcd();
	REG_LCD_DA1 = virt_to_phys(dma1_desc0);
	__lcd_set_ena();
	__lcd_display_on();

return 0;
}
#else
static int jz4760fb_suspend(struct device *dev, pm_message_t state) {return 0;}
static int jz4760fb_resume(struct device *dev) {return 0;}
#endif /* CONFIG_PM */

/* The following routine is only for test */

static void jz4760_lcd_gpio_init(void)
{
	/* gpio init __gpio_as_lcd */
	if (jz4760_lcd_info->panel.cfg & LCD_CFG_MODE_TFT_16BIT)
		__gpio_as_lcd_16bit();
	else if (jz4760_lcd_info->panel.cfg & LCD_CFG_MODE_TFT_24BIT)
		__gpio_as_lcd_24bit();
	else
		__gpio_as_lcd_18bit();

	/* Configure SLCD module for setting smart lcd control registers */
#if defined(CONFIG_FB_JZ4760_SLCD)
	__lcd_as_smart_lcd();
	__slcd_disable_dma();
	__init_slcd_bus();	/* Note: modify this depend on you lcd */

#endif
	__lcd_display_pin_init();
}

static void jz4760_lcd_init_cfg(void)
{
	if (use_fg0_only || use_2layer_Fg)
		jz4760_lcd_info->osd.osd_cfg |= LCD_OSDC_F0EN; /* only open fg0 */

//	jz4760_lcd_info->osd.osd_cfg |= LCD_OSDC_F1EN; /* only open fg1 */

	/* In special mode, we only need init special pin,
	 * as general lcd pin has init in uboot */
#if defined(CONFIG_SOC_JZ4760)
	switch (jz4760_lcd_info->panel.cfg & LCD_CFG_MODE_MASK) {
	case LCD_CFG_MODE_SPECIAL_TFT_1:
	case LCD_CFG_MODE_SPECIAL_TFT_2:
	case LCD_CFG_MODE_SPECIAL_TFT_3:
		__gpio_as_lcd_special();
		break;
	default:
		break;
	}
#endif
}
#ifdef CONFIG_LEDS_CLASS
static void lcd_set_backlight_level(struct led_classdev *led_cdev, enum led_brightness value) {
	__lcd_set_backlight_level((int)value);
}

static struct led_classdev lcd_backlight_led = {
	.name			= "lcd-backlight",
	.brightness_set		= lcd_set_backlight_level,
};
#endif


static int __init jz4760fb_probe(struct platform_device *pdev)
{
	struct lcd_cfb_info *cfb;
	int err = 0;
	__lcd_close_backlight();
	if (!pdev)
		return -EINVAL;
	jz4760_lcd_gpio_init();		/* gpio init */
	__gpio_as_epd();
	jz4760_lcd_init_cfg();		/* first config of lcd */

	__lcd_clr_dis();
	__lcd_clr_ena();

	/* init clock */
	__lcd_slcd_special_on();

	cfb = jz4760fb_alloc_fb_info();
	if (!cfb)
		goto failed;

	err = jz4760fb_map_smem(cfb);
	if (err)
		goto failed;

	jz4760fb_deep_set_mode( jz4760_lcd_info );

	/* registers frame buffer devices */

	if (use_fg0_only || use_2layer_Fg)
	{	/* register fg0 */
		err = register_framebuffer(&cfb->fb0);
		if (err < 0) {
			dprintk("jz4760fb_init(): register framebuffer err.\n");
			goto failed;
		}
		printk("fb%d: %s frame buffer device, using %dK of video memory\n",
		       cfb->fb0.node, cfb->fb0.fix.id, cfb->fb0.fix.smem_len>>10);

	}

	if (use_fg1_only || use_2layer_Fg)
	{/* register fg1 */
		err = register_framebuffer(&cfb->fb);
		if (err < 0) {
			dprintk("jz4760fb_init(): register framebuffer err.\n");
			goto failed;
		}
		printk("fb%d: %s frame buffer device, using %dK of video memory\n",
		       cfb->fb.node, cfb->fb.fix.id, cfb->fb.fix.smem_len>>10);
	}


	get_temp_sensor();
	epd_gray_level = 8;
	fill_init_palette();

	if (request_irq(IRQ_LCD, jz4760fb_interrupt_handler, IRQF_DISABLED,
			"lcd", 0)) {
		err = -EBUSY;
		goto failed;
	}

#ifdef CONFIG_LEDS_CLASS
	err = led_classdev_register(&pdev->dev, &lcd_backlight_led);
	if (err < 0)
		goto failed;
#endif
#if defined(CONFIG_JZ4760_EPSON_EPD_DISPLAY)
	init_epd_controller();
	REG_SLCD_CTRL |= SLCD_CTRL_DMA_START;
#endif

#if defined(CONFIG_JZ4760_EPSON_EPD_DISPLAY)
	REG_EPD_CTRL2 |= (EPD_CTRL2_PWRON);
#else
	__lcd_set_ena();	/* enalbe LCD Controller */
	__lcd_display_on();
#endif

//	print_lcdc_registers();
	pm_set_vt_switch(0); /*disable VT switch during suspend/resume*/
	return 0;

failed:
	print_dbg();
	jz4760fb_unmap_smem(cfb);
	jz4760fb_free_fb_info(cfb);

	return err;
}

static int jz4760fb_remove(struct platform_device *pdev)
{
	struct lcd_cfb_info *cfb = platform_get_drvdata(pdev);

	jz4760fb_unmap_smem(cfb);
	jz4760fb_free_fb_info(cfb);
	return 0;
}



static struct platform_driver jz_lcd_driver = {
	.probe = jz4760fb_probe,
	.remove = jz4760fb_remove,
#ifdef CONFIG_PM
	.suspend = jz4760fb_suspend,
	.resume = jz4760fb_resume,
#endif
	.driver = {
		   .name = DRIVER_NAME,
		   },
};

static int __init jz4760fb_init(void)
{
	return platform_driver_register(&jz_lcd_driver);
}

static void __exit jz4760fb_cleanup(void)
{
	platform_driver_unregister(&jz_lcd_driver);
}

module_init(jz4760fb_init);
module_exit(jz4760fb_cleanup);
