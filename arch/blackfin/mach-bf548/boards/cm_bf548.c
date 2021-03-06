/*
 * File:         arch/blackfin/mach-bf548/boards/cm_bf548.c
 * Based on:     arch/blackfin/mach-bf537/boards/ezkit.c
 * Author:       Aidan Williams <aidan@nicta.com.au>
 *
 * Created:
 * Description:
 *
 * Modified:
 *               Copyright 2005 National ICT Australia (NICTA)
 *               Copyright 2004-2008 Analog Devices Inc.
 *
 * Bugs:         Enter bugs at http://blackfin.uclinux.org/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see the file COPYING, or write
 * to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/physmap.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/usb/musb.h>
#include <asm/bfin5xx_spi.h>
#include <asm/dma.h>
#include <asm/gpio.h>
#include <asm/nand.h>
#include <asm/portmux.h>
#include <asm/bfin_sdh.h>
#include <mach/bf54x_keys.h>
#include <asm/dpmc.h>
#include <linux/input.h>
#include <linux/spi/ad7877.h>

/*
 * Name the Board for the /proc/cpuinfo
 */
const char bfin_board_name[] = "Bluetechnix CM-BF548";

/*
 *  Driver needs to know address, irq and flag pin.
 */

#if defined(CONFIG_FB_BF54X_LQ043) || defined(CONFIG_FB_BF54X_LQ043_MODULE)

#include <mach/bf54x-lq043.h>

static struct bfin_bf54xfb_mach_info bf54x_lq043_data = {
	.width =	480,
	.height =	272,
	.xres =		{480, 480, 480},
	.yres =		{272, 272, 272},
	.bpp =		{24, 24, 24},
	.disp =		GPIO_PE3,
};

static struct resource bf54x_lq043_resources[] = {
	{
		.start = IRQ_EPPI0_ERR,
		.end = IRQ_EPPI0_ERR,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device bf54x_lq043_device = {
	.name		= "bf54x-lq043",
	.id		= -1,
	.num_resources 	= ARRAY_SIZE(bf54x_lq043_resources),
	.resource 	= bf54x_lq043_resources,
	.dev		= {
		.platform_data = &bf54x_lq043_data,
	},
};
#endif

#if defined(CONFIG_KEYBOARD_BFIN) || defined(CONFIG_KEYBOARD_BFIN_MODULE)
static unsigned int bf548_keymap[] = {
	KEYVAL(0, 0, KEY_ENTER),
	KEYVAL(0, 1, KEY_HELP),
	KEYVAL(0, 2, KEY_0),
	KEYVAL(0, 3, KEY_BACKSPACE),
	KEYVAL(1, 0, KEY_TAB),
	KEYVAL(1, 1, KEY_9),
	KEYVAL(1, 2, KEY_8),
	KEYVAL(1, 3, KEY_7),
	KEYVAL(2, 0, KEY_DOWN),
	KEYVAL(2, 1, KEY_6),
	KEYVAL(2, 2, KEY_5),
	KEYVAL(2, 3, KEY_4),
	KEYVAL(3, 0, KEY_UP),
	KEYVAL(3, 1, KEY_3),
	KEYVAL(3, 2, KEY_2),
	KEYVAL(3, 3, KEY_1),
};

static struct bfin_kpad_platform_data bf54x_kpad_data = {
	.rows			= 4,
	.cols			= 4,
	.keymap 		= bf548_keymap,
	.keymapsize 		= ARRAY_SIZE(bf548_keymap),
	.repeat			= 0,
	.debounce_time		= 5000,	/* ns (5ms) */
	.coldrive_time		= 1000, /* ns (1ms) */
	.keyup_test_interval	= 50, /* ms (50ms) */
};

static struct resource bf54x_kpad_resources[] = {
	{
		.start = IRQ_KEY,
		.end = IRQ_KEY,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device bf54x_kpad_device = {
	.name		= "bf54x-keys",
	.id		= -1,
	.num_resources 	= ARRAY_SIZE(bf54x_kpad_resources),
	.resource 	= bf54x_kpad_resources,
	.dev		= {
		.platform_data = &bf54x_kpad_data,
	},
};
#endif

#if defined(CONFIG_RTC_DRV_BFIN) || defined(CONFIG_RTC_DRV_BFIN_MODULE)
static struct platform_device rtc_device = {
	.name = "rtc-bfin",
	.id   = -1,
};
#endif

#if defined(CONFIG_SERIAL_BFIN) || defined(CONFIG_SERIAL_BFIN_MODULE)
static struct resource bfin_uart_resources[] = {
#ifdef CONFIG_SERIAL_BFIN_UART0
	{
		.start = 0xFFC00400,
		.end = 0xFFC004FF,
		.flags = IORESOURCE_MEM,
	},
#endif
#ifdef CONFIG_SERIAL_BFIN_UART1
	{
		.start = 0xFFC02000,
		.end = 0xFFC020FF,
		.flags = IORESOURCE_MEM,
	},
#endif
#ifdef CONFIG_SERIAL_BFIN_UART2
	{
		.start = 0xFFC02100,
		.end = 0xFFC021FF,
		.flags = IORESOURCE_MEM,
	},
#endif
#ifdef CONFIG_SERIAL_BFIN_UART3
	{
		.start = 0xFFC03100,
		.end = 0xFFC031FF,
		.flags = IORESOURCE_MEM,
	},
#endif
};

static struct platform_device bfin_uart_device = {
	.name = "bfin-uart",
	.id = 1,
	.num_resources = ARRAY_SIZE(bfin_uart_resources),
	.resource = bfin_uart_resources,
};
#endif

#if defined(CONFIG_BFIN_SIR) || defined(CONFIG_BFIN_SIR_MODULE)
#ifdef CONFIG_BFIN_SIR0
static struct resource bfin_sir0_resources[] = {
	{
		.start = 0xFFC00400,
		.end = 0xFFC004FF,
		.flags = IORESOURCE_MEM,
	},
	{
		.start = IRQ_UART0_RX,
		.end = IRQ_UART0_RX+1,
		.flags = IORESOURCE_IRQ,
	},
	{
		.start = CH_UART0_RX,
		.end = CH_UART0_RX+1,
		.flags = IORESOURCE_DMA,
	},
};
static struct platform_device bfin_sir0_device = {
	.name = "bfin_sir",
	.id = 0,
	.num_resources = ARRAY_SIZE(bfin_sir0_resources),
	.resource = bfin_sir0_resources,
};
#endif
#ifdef CONFIG_BFIN_SIR1
static struct resource bfin_sir1_resources[] = {
	{
		.start = 0xFFC02000,
		.end = 0xFFC020FF,
		.flags = IORESOURCE_MEM,
	},
	{
		.start = IRQ_UART1_RX,
		.end = IRQ_UART1_RX+1,
		.flags = IORESOURCE_IRQ,
	},
	{
		.start = CH_UART1_RX,
		.end = CH_UART1_RX+1,
		.flags = IORESOURCE_DMA,
	},
};
static struct platform_device bfin_sir1_device = {
	.name = "bfin_sir",
	.id = 1,
	.num_resources = ARRAY_SIZE(bfin_sir1_resources),
	.resource = bfin_sir1_resources,
};
#endif
#ifdef CONFIG_BFIN_SIR2
static struct resource bfin_sir2_resources[] = {
	{
		.start = 0xFFC02100,
		.end = 0xFFC021FF,
		.flags = IORESOURCE_MEM,
	},
	{
		.start = IRQ_UART2_RX,
		.end = IRQ_UART2_RX+1,
		.flags = IORESOURCE_IRQ,
	},
	{
		.start = CH_UART2_RX,
		.end = CH_UART2_RX+1,
		.flags = IORESOURCE_DMA,
	},
};
static struct platform_device bfin_sir2_device = {
	.name = "bfin_sir",
	.id = 2,
	.num_resources = ARRAY_SIZE(bfin_sir2_resources),
	.resource = bfin_sir2_resources,
};
#endif
#ifdef CONFIG_BFIN_SIR3
static struct resource bfin_sir3_resources[] = {
	{
		.start = 0xFFC03100,
		.end = 0xFFC031FF,
		.flags = IORESOURCE_MEM,
	},
	{
		.start = IRQ_UART3_RX,
		.end = IRQ_UART3_RX+1,
		.flags = IORESOURCE_IRQ,
	},
	{
		.start = CH_UART3_RX,
		.end = CH_UART3_RX+1,
		.flags = IORESOURCE_DMA,
	},
};
static struct platform_device bfin_sir3_device = {
	.name = "bfin_sir",
	.id = 3,
	.num_resources = ARRAY_SIZE(bfin_sir3_resources),
	.resource = bfin_sir3_resources,
};
#endif
#endif

#if defined(CONFIG_SMSC911X) || defined(CONFIG_SMSC911X_MODULE)
static struct resource smsc911x_resources[] = {
	{
		.name = "smsc911x-memory",
		.start = 0x24000000,
		.end = 0x24000000 + 0xFF,
		.flags = IORESOURCE_MEM,
	},
	{
		.start = IRQ_PE6,
		.end = IRQ_PE6,
		.flags = IORESOURCE_IRQ | IORESOURCE_IRQ_LOWLEVEL,
	},
};
static struct platform_device smsc911x_device = {
	.name = "smsc911x",
	.id = 0,
	.num_resources = ARRAY_SIZE(smsc911x_resources),
	.resource = smsc911x_resources,
};
#endif

#if defined(CONFIG_USB_MUSB_HDRC) || defined(CONFIG_USB_MUSB_HDRC_MODULE)
static struct resource musb_resources[] = {
	[0] = {
		.start	= 0xFFC03C00,
		.end	= 0xFFC040FF,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {	/* general IRQ */
		.start	= IRQ_USB_INT0,
		.end	= IRQ_USB_INT0,
		.flags	= IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL,
	},
	[2] = {	/* DMA IRQ */
		.start	= IRQ_USB_DMA,
		.end	= IRQ_USB_DMA,
		.flags	= IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL,
	},
};

static struct musb_hdrc_config musb_config = {
	.multipoint	= 0,
	.dyn_fifo	= 0,
	.soft_con	= 1,
	.dma		= 1,
	.num_eps	= 8,
	.dma_channels	= 8,
	.gpio_vrsel	= GPIO_PH6,
};

static struct musb_hdrc_platform_data musb_plat = {
#if defined(CONFIG_USB_MUSB_OTG)
	.mode		= MUSB_OTG,
#elif defined(CONFIG_USB_MUSB_HDRC_HCD)
	.mode		= MUSB_HOST,
#elif defined(CONFIG_USB_GADGET_MUSB_HDRC)
	.mode		= MUSB_PERIPHERAL,
#endif
	.config		= &musb_config,
};

static u64 musb_dmamask = ~(u32)0;

static struct platform_device musb_device = {
	.name		= "musb_hdrc",
	.id		= 0,
	.dev = {
		.dma_mask		= &musb_dmamask,
		.coherent_dma_mask	= 0xffffffff,
		.platform_data		= &musb_plat,
	},
	.num_resources	= ARRAY_SIZE(musb_resources),
	.resource	= musb_resources,
};
#endif

static struct resource bfin_gpios_resources = {
	.start = 0,
	.end   = MAX_BLACKFIN_GPIOS - 1,
	.flags = IORESOURCE_IRQ,
};

static struct platform_device bfin_gpios_device = {
	.name = "simple-gpio",
	.id = -1,
	.num_resources = 1,
	.resource = &bfin_gpios_resources,
};

#if defined(CONFIG_PATA_BF54X) || defined(CONFIG_PATA_BF54X_MODULE)
static struct resource bfin_atapi_resources[] = {
	{
		.start = 0xFFC03800,
		.end = 0xFFC0386F,
		.flags = IORESOURCE_MEM,
	},
	{
		.start = IRQ_ATAPI_ERR,
		.end = IRQ_ATAPI_ERR,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device bfin_atapi_device = {
	.name = "pata-bf54x",
	.id = -1,
	.num_resources = ARRAY_SIZE(bfin_atapi_resources),
	.resource = bfin_atapi_resources,
};
#endif

#if defined(CONFIG_MTD_NAND_BF5XX) || defined(CONFIG_MTD_NAND_BF5XX_MODULE)
static struct mtd_partition partition_info[] = {
	{
		.name = "linux kernel(nand)",
		.offset = 0,
		.size = 4 * 1024 * 1024,
	},
	{
		.name = "file system(nand)",
		.offset = 4 * 1024 * 1024,
		.size = (256 - 4) * 1024 * 1024,
	},
};

static struct bf5xx_nand_platform bf5xx_nand_platform = {
	.page_size = NFC_PG_SIZE_256,
	.data_width = NFC_NWIDTH_8,
	.partitions = partition_info,
	.nr_partitions = ARRAY_SIZE(partition_info),
	.rd_dly = 3,
	.wr_dly = 3,
};

static struct resource bf5xx_nand_resources[] = {
	{
		.start = 0xFFC03B00,
		.end = 0xFFC03B4F,
		.flags = IORESOURCE_MEM,
	},
	{
		.start = CH_NFC,
		.end = CH_NFC,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device bf5xx_nand_device = {
	.name = "bf5xx-nand",
	.id = 0,
	.num_resources = ARRAY_SIZE(bf5xx_nand_resources),
	.resource = bf5xx_nand_resources,
	.dev = {
		.platform_data = &bf5xx_nand_platform,
	},
};
#endif

#if defined(CONFIG_SDH_BFIN) || defined(CONFIG_SDH_BFIN_MODULE)
static struct bfin_sd_host bfin_sdh_data = {
	.dma_chan = CH_SDH,
	.irq_int0 = IRQ_SDH_MASK0,
	.pin_req = {P_SD_D0, P_SD_D1, P_SD_D2, P_SD_D3, P_SD_CLK, P_SD_CMD, 0},
};

static struct platform_device bf54x_sdh_device = {
	.name = "bfin-sdh",
	.id = 0,
	.dev = {
		.platform_data = &bfin_sdh_data,
	},
};
#endif

#if defined(CONFIG_MTD_PHYSMAP) || defined(CONFIG_MTD_PHYSMAP_MODULE)
static struct mtd_partition para_partitions[] = {
	{
		.name       = "bootloader(nor)",
		.size       = 0x40000,
		.offset     = 0,
	}, {
		.name       = "linux kernel(nor)",
		.size       = 0x400000,
		.offset     = MTDPART_OFS_APPEND,
	}, {
		.name       = "file system(nor)",
		.size       = MTDPART_SIZ_FULL,
		.offset     = MTDPART_OFS_APPEND,
	}
};

static struct physmap_flash_data para_flash_data = {
	.width      = 2,
	.parts      = para_partitions,
	.nr_parts   = ARRAY_SIZE(para_partitions),
};

static struct resource para_flash_resource = {
	.start = 0x20000000,
	.end   = 0x207fffff,
	.flags = IORESOURCE_MEM,
};

static struct platform_device para_flash_device = {
	.name          = "physmap-flash",
	.id            = 0,
	.dev = {
		.platform_data = &para_flash_data,
	},
	.num_resources = 1,
	.resource      = &para_flash_resource,
};
#endif

#if defined(CONFIG_SPI_BFIN) || defined(CONFIG_SPI_BFIN_MODULE)
/* all SPI peripherals info goes here */
#if defined(CONFIG_MTD_M25P80) \
	|| defined(CONFIG_MTD_M25P80_MODULE)
/* SPI flash chip (m25p16) */
static struct mtd_partition bfin_spi_flash_partitions[] = {
	{
		.name = "bootloader(spi)",
		.size = 0x00040000,
		.offset = 0,
		.mask_flags = MTD_CAP_ROM
	}, {
		.name = "linux kernel(spi)",
		.size = 0x1c0000,
		.offset = 0x40000
	}
};

static struct flash_platform_data bfin_spi_flash_data = {
	.name = "m25p80",
	.parts = bfin_spi_flash_partitions,
	.nr_parts = ARRAY_SIZE(bfin_spi_flash_partitions),
	.type = "m25p16",
};

static struct bfin5xx_spi_chip spi_flash_chip_info = {
	.enable_dma = 0,         /* use dma transfer with this chip*/
	.bits_per_word = 8,
	.cs_change_per_word = 0,
};
#endif

#if defined(CONFIG_TOUCHSCREEN_AD7877) || defined(CONFIG_TOUCHSCREEN_AD7877_MODULE)
static struct bfin5xx_spi_chip spi_ad7877_chip_info = {
	.cs_change_per_word = 0,
	.enable_dma = 0,
	.bits_per_word = 16,
};

static const struct ad7877_platform_data bfin_ad7877_ts_info = {
	.model			= 7877,
	.vref_delay_usecs	= 50,	/* internal, no capacitor */
	.x_plate_ohms		= 419,
	.y_plate_ohms		= 486,
	.pressure_max		= 1000,
	.pressure_min		= 0,
	.stopacq_polarity 	= 1,
	.first_conversion_delay = 3,
	.acquisition_time 	= 1,
	.averaging 		= 1,
	.pen_down_acc_interval 	= 1,
};
#endif

#if defined(CONFIG_SPI_SPIDEV) || defined(CONFIG_SPI_SPIDEV_MODULE)
static struct bfin5xx_spi_chip spidev_chip_info = {
	.enable_dma = 0,
	.bits_per_word = 8,
};
#endif

static struct spi_board_info bf54x_spi_board_info[] __initdata = {
#if defined(CONFIG_MTD_M25P80) \
	|| defined(CONFIG_MTD_M25P80_MODULE)
	{
		/* the modalias must be the same as spi device driver name */
		.modalias = "m25p80", /* Name of spi_driver for this device */
		.max_speed_hz = 25000000,     /* max spi clock (SCK) speed in HZ */
		.bus_num = 0, /* Framework bus number */
		.chip_select = 1, /* SPI_SSEL1*/
		.platform_data = &bfin_spi_flash_data,
		.controller_data = &spi_flash_chip_info,
		.mode = SPI_MODE_3,
	},
#endif
#if defined(CONFIG_TOUCHSCREEN_AD7877) || defined(CONFIG_TOUCHSCREEN_AD7877_MODULE)
{
	.modalias		= "ad7877",
	.platform_data		= &bfin_ad7877_ts_info,
	.irq			= IRQ_PJ11,
	.max_speed_hz		= 12500000,     /* max spi clock (SCK) speed in HZ */
	.bus_num		= 0,
	.chip_select  		= 2,
	.controller_data = &spi_ad7877_chip_info,
},
#endif
#if defined(CONFIG_SPI_SPIDEV) || defined(CONFIG_SPI_SPIDEV_MODULE)
	{
		.modalias = "spidev",
		.max_speed_hz = 3125000,     /* max spi clock (SCK) speed in HZ */
		.bus_num = 0,
		.chip_select = 1,
		.controller_data = &spidev_chip_info,
	},
#endif
};

/* SPI (0) */
static struct resource bfin_spi0_resource[] = {
	[0] = {
		.start = SPI0_REGBASE,
		.end   = SPI0_REGBASE + 0xFF,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = CH_SPI0,
		.end   = CH_SPI0,
		.flags = IORESOURCE_DMA,
	},
	[2] = {
		.start = IRQ_SPI0,
		.end   = IRQ_SPI0,
		.flags = IORESOURCE_IRQ,
	}
};

/* SPI (1) */
static struct resource bfin_spi1_resource[] = {
	[0] = {
		.start = SPI1_REGBASE,
		.end   = SPI1_REGBASE + 0xFF,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = CH_SPI1,
		.end   = CH_SPI1,
		.flags = IORESOURCE_DMA,
	},
	[2] = {
		.start = IRQ_SPI1,
		.end   = IRQ_SPI1,
		.flags = IORESOURCE_IRQ,
	}
};

/* SPI controller data */
static struct bfin5xx_spi_master bf54x_spi_master_info0 = {
	.num_chipselect = 8,
	.enable_dma = 1,  /* master has the ability to do dma transfer */
	.pin_req = {P_SPI0_SCK, P_SPI0_MISO, P_SPI0_MOSI, 0},
};

static struct platform_device bf54x_spi_master0 = {
	.name = "bfin-spi",
	.id = 0, /* Bus number */
	.num_resources = ARRAY_SIZE(bfin_spi0_resource),
	.resource = bfin_spi0_resource,
	.dev = {
		.platform_data = &bf54x_spi_master_info0, /* Passed to driver */
		},
};

static struct bfin5xx_spi_master bf54x_spi_master_info1 = {
	.num_chipselect = 8,
	.enable_dma = 1,  /* master has the ability to do dma transfer */
	.pin_req = {P_SPI1_SCK, P_SPI1_MISO, P_SPI1_MOSI, 0},
};

static struct platform_device bf54x_spi_master1 = {
	.name = "bfin-spi",
	.id = 1, /* Bus number */
	.num_resources = ARRAY_SIZE(bfin_spi1_resource),
	.resource = bfin_spi1_resource,
	.dev = {
		.platform_data = &bf54x_spi_master_info1, /* Passed to driver */
		},
};
#endif  /* spi master and devices */

#if defined(CONFIG_I2C_BLACKFIN_TWI) || defined(CONFIG_I2C_BLACKFIN_TWI_MODULE)
static struct resource bfin_twi0_resource[] = {
	[0] = {
		.start = TWI0_REGBASE,
		.end   = TWI0_REGBASE + 0xFF,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_TWI0,
		.end   = IRQ_TWI0,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device i2c_bfin_twi0_device = {
	.name = "i2c-bfin-twi",
	.id = 0,
	.num_resources = ARRAY_SIZE(bfin_twi0_resource),
	.resource = bfin_twi0_resource,
};

#if !defined(CONFIG_BF542)	/* The BF542 only has 1 TWI */
static struct resource bfin_twi1_resource[] = {
	[0] = {
		.start = TWI1_REGBASE,
		.end   = TWI1_REGBASE + 0xFF,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_TWI1,
		.end   = IRQ_TWI1,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device i2c_bfin_twi1_device = {
	.name = "i2c-bfin-twi",
	.id = 1,
	.num_resources = ARRAY_SIZE(bfin_twi1_resource),
	.resource = bfin_twi1_resource,
};
#endif
#endif

#if defined(CONFIG_KEYBOARD_GPIO) || defined(CONFIG_KEYBOARD_GPIO_MODULE)
#include <linux/gpio_keys.h>

static struct gpio_keys_button bfin_gpio_keys_table[] = {
	{BTN_0, GPIO_PH7, 1, "gpio-keys: BTN0"},
};

static struct gpio_keys_platform_data bfin_gpio_keys_data = {
	.buttons        = bfin_gpio_keys_table,
	.nbuttons       = ARRAY_SIZE(bfin_gpio_keys_table),
};

static struct platform_device bfin_device_gpiokeys = {
	.name      = "gpio-keys",
	.dev = {
		.platform_data = &bfin_gpio_keys_data,
	},
};
#endif

static const unsigned int cclk_vlev_datasheet[] =
{
/*
 * Internal VLEV BF54XSBBC1533
 ****temporarily using these values until data sheet is updated
 */
	VRPAIR(VLEV_085, 150000000),
	VRPAIR(VLEV_090, 250000000),
	VRPAIR(VLEV_110, 276000000),
	VRPAIR(VLEV_115, 301000000),
	VRPAIR(VLEV_120, 525000000),
	VRPAIR(VLEV_125, 550000000),
	VRPAIR(VLEV_130, 600000000),
};

static struct bfin_dpmc_platform_data bfin_dmpc_vreg_data = {
	.tuple_tab = cclk_vlev_datasheet,
	.tabsize = ARRAY_SIZE(cclk_vlev_datasheet),
	.vr_settling_time = 25 /* us */,
};

static struct platform_device bfin_dpmc = {
	.name = "bfin dpmc",
	.dev = {
		.platform_data = &bfin_dmpc_vreg_data,
	},
};

static struct platform_device *cm_bf548_devices[] __initdata = {

	&bfin_dpmc,

#if defined(CONFIG_RTC_DRV_BFIN) || defined(CONFIG_RTC_DRV_BFIN_MODULE)
	&rtc_device,
#endif

#if defined(CONFIG_SERIAL_BFIN) || defined(CONFIG_SERIAL_BFIN_MODULE)
	&bfin_uart_device,
#endif

#if defined(CONFIG_BFIN_SIR) || defined(CONFIG_BFIN_SIR_MODULE)
#ifdef CONFIG_BFIN_SIR0
	&bfin_sir0_device,
#endif
#ifdef CONFIG_BFIN_SIR1
	&bfin_sir1_device,
#endif
#ifdef CONFIG_BFIN_SIR2
	&bfin_sir2_device,
#endif
#ifdef CONFIG_BFIN_SIR3
	&bfin_sir3_device,
#endif
#endif

#if defined(CONFIG_FB_BF54X_LQ043) || defined(CONFIG_FB_BF54X_LQ043_MODULE)
	&bf54x_lq043_device,
#endif

#if defined(CONFIG_SMSC911X) || defined(CONFIG_SMSC911X_MODULE)
	&smsc911x_device,
#endif

#if defined(CONFIG_USB_MUSB_HDRC) || defined(CONFIG_USB_MUSB_HDRC_MODULE)
	&musb_device,
#endif

#if defined(CONFIG_PATA_BF54X) || defined(CONFIG_PATA_BF54X_MODULE)
	&bfin_atapi_device,
#endif

#if defined(CONFIG_MTD_NAND_BF5XX) || defined(CONFIG_MTD_NAND_BF5XX_MODULE)
	&bf5xx_nand_device,
#endif

#if defined(CONFIG_SDH_BFIN) || defined(CONFIG_SDH_BFIN_MODULE)
	&bf54x_sdh_device,
#endif

#if defined(CONFIG_SPI_BFIN) || defined(CONFIG_SPI_BFIN_MODULE)
	&bf54x_spi_master0,
	&bf54x_spi_master1,
#endif

#if defined(CONFIG_KEYBOARD_BFIN) || defined(CONFIG_KEYBOARD_BFIN_MODULE)
	&bf54x_kpad_device,
#endif

#if defined(CONFIG_I2C_BLACKFIN_TWI) || defined(CONFIG_I2C_BLACKFIN_TWI_MODULE)
	&i2c_bfin_twi0_device,
#if !defined(CONFIG_BF542)
	&i2c_bfin_twi1_device,
#endif
#endif

#if defined(CONFIG_KEYBOARD_GPIO) || defined(CONFIG_KEYBOARD_GPIO_MODULE)
	&bfin_device_gpiokeys,
#endif

#if defined(CONFIG_MTD_PHYSMAP) || defined(CONFIG_MTD_PHYSMAP_MODULE)
	&para_flash_device,
#endif

	&bfin_gpios_device,
};

static int __init cm_bf548_init(void)
{
	printk(KERN_INFO "%s(): registering device resources\n", __func__);
	platform_add_devices(cm_bf548_devices, ARRAY_SIZE(cm_bf548_devices));

#if defined(CONFIG_SPI_BFIN) || defined(CONFIG_SPI_BFIN_MODULE)
	spi_register_board_info(bf54x_spi_board_info,
			ARRAY_SIZE(bf54x_spi_board_info));
#endif

	return 0;
}

arch_initcall(cm_bf548_init);
