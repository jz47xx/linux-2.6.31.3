/*
 * Linux/sound/oss/jz_dlv.c
 *
 * DLV CODEC driver for Ingenic Jz4750 MIPS processor
 *
 * 2009-12-xx	Steven <dsqiu@ingenic.cn>
 * 2010-01-xx	Jason <xwang@ingenic.cn>
 *
 * Copyright (c) Ingenic Semiconductor Co., Ltd.
 */

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/workqueue.h>

#include <linux/sound.h>
#include <linux/slab.h>
#include <sound/core.h>
#include <sound/initval.h>
#include <linux/proc_fs.h>
#include <linux/soundcard.h>
#include <linux/dma-mapping.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <asm/hardirq.h>
#include <asm/jzsoc.h>

#include "sound_config.h"

#include "jz_codec.h"
#include "jz4760_dlv.h"
#include "jz_i2s_dbg.h"

#define HP_SENSE_DETECT		0
#define REPLAY			1
#define RECORD			2

#define POWER_ON	0
#define POWER_OFF	1

#define switch_SB_DAC(pwrstat)					\
do {								\
	dlv_write_reg_bit(DLV_REG_PMR2, pwrstat, PMR2_SB_DAC);	\
	dlv_write_reg_bit(DLV_REG_PMR2, pwrstat, PMR2_SB_ADC);	\
} while (0)

#define switch_SB_LINE_OUT(pwrstat)				\
do {								\
	dlv_write_reg_bit(DLV_REG_PMR2, pwrstat, PMR2_SB_LOUT);	\
} while (0)

#define switch_SB_OUT(pwrstat)					\
do {								\
	dlv_write_reg_bit(DLV_REG_PMR2, pwrstat, PMR2_SB_HP);	\
	dlv_write_reg_bit(DLV_REG_PMR2, pwrstat, PMR2_SB_LOUT);	\
	dlv_write_reg_bit(DLV_REG_PMR2, pwrstat, PMR2_SB_BTL);	\
} while (0)

#define switch_SB_MIX(pwrstat)					\
do {								\
	/*	dlv_write_reg_bit(5, pwrstat, 5);*/		\
} while (0)

#define switch_SB_ADC(pwrstat)					\
do {								\
	dlv_write_reg_bit(DLV_REG_PMR2, pwrstat, PMR2_SB_ADC);	\
} while (0)

#define switch_SB_MIC1(pwrstat)					\
do {								\
	dlv_write_reg_bit(DLV_REG_PMR1, pwrstat, PMR1_SB_MIC1);	\
	dlv_write_reg_bit(DLV_REG_PMR1, pwrstat, PMR1_SB_MIC2);	\
	dlv_write_reg_bit(DLV_REG_PMR1, pwrstat, PMR1_SB_MICBIAS);\
								\
} while (0)

#define switch_SB_LINE_IN(pwrstat)					\
do {								\
	dlv_write_reg_bit(DLV_REG_PMR1, pwrstat, PMR1_SB_LINE);	\
	dlv_write_reg_bit(DLV_REG_PMR1, pwrstat, PMR1_SB_MICBIAS);\
								\
} while (0)

/*
#define ENTER()			printk("Enter: %s, %s:%i\n", __FUNCTION__, __FILE__, __LINE__)
#define LEAVE()			printk("Leave: %s, %s:%i\n", __FUNCTION__, __FILE__, __LINE__)
*/

#ifdef IOC_DEBUG
static dlv_print_ioc_cmd(int cmd)
{
	char *dlv_ioc_cmd[] = {
		"CODEC_SET_MODE",		"CODEC_CLEAR_MODE",		"CODEC_SET_GPIO_PIN",
		"CODEC_EACH_TIME_INIT",		"CODEC_SET_STARTUP_PARAM",	"CODEC_SET_VOLUME_TABLE",
		"CODEC_SET_RECORD",		"CODEC_SET_REPLAY",		"CODEC_SET_REPLAY_RECORD",
		"CODEC_TURN_ON",		"CODEC_TURN_OFF",		"CODEC_SET_REPLAY_SPEED",
		"CODEC_RESET",			"CODEC_GET_MIXER_OLD_INFO",	"CODEC_GET_MIXER_INFO",
		"CODEC_SET_BASS",		"CODEC_SET_VOLUME",		"CODEC_SET_MIC",
		"CODEC_SET_LINE",		"CODEC_I2S_RESUME",		"CODEC_I2S_SUSPEND",
		"CODEC_PIN_INIT",		"CODEC_SET_SOME_FUNC",		"CODEC_CLEAR_RECORD",
		"CODEC_CLEAR_REPLAY",		"CODEC_SET_REPLAY_HP_OR_SPKR",	"CODEC_SET_DIRECT_MODE",
		"CODEC_CLEAR_DIRECT_MODE",	"CODEC_SET_LINEIN2HP",		"CODEC_CLEAR_LINEIN2HP",
		"CODEC_ANTI_POP",		"CODEC_TURN_REPLAY",		"CODEC_SET_REPLAY_CHANNEL",
		"CODEC_SET_REPLAY_FORMAT",	"CODEC_SET_RECORD_CHANNEL",	"CODEC_SET_RECORD_FORMAT",
		"CODEC_SET_RECORD_SPEED",	"CODEC_DAC_MUTE"
	};

	if (cmd >= (sizeof(dlv_ioc_cmd) / sizeof(dlv_ioc_cmd[0]))) {
		printk("%s: Unkown command !\n", __FUNCTION__);
	} else {
		printk("IOC CMD NAME = %s\n", dlv_ioc_cmd[cmd - 1]);
	}
}
#endif

/**
 * CODEC read register
 *
 * addr:	address of register
 * return:	value of register
 */
static inline int dlv_read_reg(int addr)
{
	volatile int reg;
	while (__icdc_rgwr_ready()) {
		;//nothing...
	}
	__icdc_set_addr(addr);
	reg = __icdc_get_value();
	reg = __icdc_get_value();
	reg = __icdc_get_value();
	reg = __icdc_get_value();
	reg = __icdc_get_value();
	return __icdc_get_value();
}

/**
 * CODEC write register
 *
 * addr:	address of register
 * val:		value to set
 */
void dlv_write_reg(int addr, int val)
{
	volatile int reg;
	while (__icdc_rgwr_ready()) {
		;//nothing...
	}
	REG_ICDC_RGADW = ((addr << ICDC_RGADW_RGADDR_BIT) | val);
	__icdc_set_rgwr();
	reg = __icdc_rgwr_ready();
	reg = __icdc_rgwr_ready();
	reg = __icdc_rgwr_ready();
	reg = __icdc_rgwr_ready();
	reg = __icdc_rgwr_ready();
	reg = __icdc_rgwr_ready();
	while (__icdc_rgwr_ready()) {
		;//nothing...
	}
}

/**
 * CODEC write a bit of a register
 *
 * addr:	address of register
 * bitval:	bit value to modifiy
 * mask_bit:	indicate which bit will be modifiy
 */
static int dlv_write_reg_bit(int addr, int bitval, int mask_bit)
{
	int val = dlv_read_reg(addr);

	if (bitval)
		val |= (1 << mask_bit);
	else
		val &= ~(1 << mask_bit);
	dlv_write_reg(addr, val);

	return 1;
}

/*
 * DLV CODEC operations routines
 */

static void dlv_each_time_init(void)
{
	ENTER();
	__i2s_disable();
	__i2s_as_slave();
	__aic_internal_codec();
	//__i2s_set_oss_sample_size(16);
	//__i2s_set_iss_sample_size(16);
	LEAVE();
}

static void dlv_set_mode(void)
{
	ENTER();

	//dlv_write_reg(8, 0x2f);
	//dlv_write_reg(9, 0xff);
	dlv_write_reg(DLV_REG_ICR, 0x0f);
	dlv_write_reg(DLV_REG_IFR, 0xff); // ???

	schedule_timeout(2);

	dlv_write_reg_bit(6, 0, 1);//PMR2.SB->0
	dlv_write_reg_bit(6, 0, 0);//PMR2.SB->0

//	dlv_write_reg_bit(1, 0, 3);//PMR2.SB->0

	dlv_write_reg_bit(5, 0, 4);//SB_ADC->1
//	set_record_mic_input_audio_with_audio_data_replay();
//	reset_dlv_codec();
	LEAVE();
}

static void dlv_reset(void)
{
	ENTER();
	/* reset DLV codec. from hibernate mode to sleep mode */
	//dlv_write_reg(0, 0xf);
	//dlv_write_reg_bit(6, 0, 0);
	//dlv_write_reg_bit(6, 0, 1);

	//2010-01-31 Jason add
	//dlv_write_reg(22, 0x40);//mic 1

	dlv_write_reg_bit(DLV_REG_AICR, 1, AICR_DAC_SERIAL);
	dlv_write_reg_bit(DLV_REG_AICR, 1, AICR_ADC_SERIAL);

	/* reset DLV codec. from hibernate mode to sleep mode */
	dlv_write_reg(DLV_REG_AICR, 0xf);
	dlv_write_reg_bit(DLV_REG_PMR1, 0, PMR1_SB);
	dlv_write_reg_bit(DLV_REG_PMR1, 0, PMR1_SB_SLEEP);

	schedule_timeout(30);



	dlv_write_reg(DLV_REG_CR3, 1 << CR3_MICSTEREO);//mic 1

	schedule_timeout(20);

	dlv_write_reg_bit(DLV_REG_PMR1, 0, PMR1_SB_AIP);

	//dlv_write_reg_bit(5, 0, 7);//PMR1.SB_DAC->0
	//dlv_write_reg_bit(5, 0, 4);//PMR1.SB_ADC->0
	dlv_write_reg_bit(DLV_REG_PMR2, 0, PMR2_SB_DAC);
	dlv_write_reg_bit(DLV_REG_PMR2, 0, PMR2_SB_ADC);


	schedule_timeout(2); ;//wait for stability
	LEAVE();
}

static int dlv_set_startup_param(void)
{
	ENTER();
	LEAVE();
//	__i2s_disable_transmit_intr();
//	__i2s_disable_receive_intr();
	return 1;
}

/**
 * Set audio replay
 */
static void dlv_set_replay(void)
{
	ENTER();

//	dump_dlv_regs("enter dlv_set_replay");

	dlv_write_reg_bit(DLV_REG_CR1, 0, CR1_HP_MUTE);
	dlv_write_reg_bit(DLV_REG_PMR1, 1, PMR1_SB_BYPASS);

	// OUTSEL <-- 11b
	dlv_write_reg_bit(DLV_REG_CR1, 1, CR1_OUTSEL0);
	dlv_write_reg_bit(DLV_REG_CR1, 1, CR1_OUTSEL1);

	// Disable mute
	dlv_write_reg_bit(DLV_REG_CR2, 0, CR2_DAC_MUTE);
	dlv_write_reg_bit(DLV_REG_CR1, 0, CR1_LINEOUT_MUTE);
	dlv_write_reg_bit(DLV_REG_CR1, 0, CR1_BTL_MUTE);

	// NOMAD
	dlv_write_reg_bit(DLV_REG_CR2, 1, CR2_NOMAD);

//	dump_dlv_regs("leave dlv_set_replay");
	LEAVE();
}

#if 0
/* set Record MIC input audio with Audio data replay (full duplex) */
static void set_record_mic_input_audio_with_audio_data_replay(void)
{
	ENTER();
	printk("when run here ?????\n");
	dlv_write_reg_bit(23, 0, 7);//AGC1.AGC_EN->0
	dlv_write_reg(9, 0xff);
	//dlv_write_reg(8, 0x30);
	dlv_write_reg(8, 0x20);
	dlv_write_reg_bit(1, 0, 4);//CR1.HP_DIS->0
	dlv_write_reg_bit(5, 1, 3);//PMR1.SB_LIN->1
	dlv_write_reg_bit(5, 1, 0);//PMR1.SB_IND->1

	dlv_write_reg_bit(22, 0, 7);//CR3.SB_MIC->0

	dlv_write_reg_bit(1, 0, 7);//CR1.SB_MICBIAS->0

	dlv_write_reg_bit(1, 1, 3);//CR1.DACSEL->1
	dlv_write_reg_bit(5, 0, 5);//PMR1.SB_MIX->0
	LEAVE();
}

/* unset Record MIC input audio with Audio data replay (full duplex) */
static void unset_record_mic_input_audio_with_audio_data_replay(void)
{
	ENTER();
	/* ADC path */
	printk("@@@ %s", __FUNCTION__);
	dlv_write_reg_bit(5, 1, 4);//SB_ADC->1
	dlv_write_reg_bit(1, 1, 7);//CR1.SB_MICBIAS->1
	//dlv_write_reg_bit(1, 1, 6);//CR1.MONO->1
	dlv_write_reg(22, 0xc0);//CR3.SB_MIC1->1
//	dlv_write_reg_bit(5, 1, 7);//SB_DAC->1
	dlv_write_reg_bit(5, 1, 5);//SB_MIX->1

	// 2009-01-20 Jason marked
//	dlv_write_reg_bit(6, 1, 0);//SB_SLEEP->1
//	dlv_write_reg_bit(6, 1, 1);//SB->1
	LEAVE();
}

/* set Record LINE input audio with Audio data replay (full duplex for linein) */
static void set_record_line_input_audio_with_audio_data_replay(void)
{
	ENTER();
	dlv_write_reg(9, 0xff);
	//dlv_write_reg(8, 0x30);
	dlv_write_reg(8, 0x20);
	dlv_write_reg_bit(1, 0, 4);//CR1.HP_DIS->0
	dlv_write_reg_bit(5, 0, 3);//PMR1.SB_LIN->0
	dlv_write_reg_bit(5, 1, 0);//PMR1.SB_IND->1
	dlv_write_reg_bit(1, 1, 7);//CR1.SB_MICBIAS->1
	//dlv_write_reg_bit(22, 1, 7);//CR3.SB_MIC->1
	dlv_write_reg_bit(1, 1, 3);//CR1.DACSEL->1
	dlv_write_reg_bit(5, 0, 5);//PMR1.SB_MIX->0

	dlv_write_reg(22, 0xc6);//line in 1
	dlv_write_reg_bit(23, 0, 7);//AGC1.AGC_EN->0
	dlv_write_reg_bit(1, 0, 2);//CR1.BYPASS->0
	dlv_write_reg_bit(5, 0, 5);//PMR1.SB_MIX->0
	LEAVE();
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
/* unset Record LINE input audio with Audio data replay (full duplex for linein) */
static void unset_record_line_input_audio_with_audio_data_replay(void)
{
	ENTER();
	/* ADC path */
	printk("@@@ %s", __FUNCTION__);

	dlv_write_reg_bit(5, 1, 4);//SB_ADC->1
	dlv_write_reg_bit(1, 1, 7);//CR1.SB_MICBIAS->1
	//dlv_write_reg_bit(1, 1, 6);//CR1.MONO->1
	dlv_write_reg(22, 0xc0);//CR3.SB_MIC1->1
//	dlv_write_reg_bit(5, 1, 7);//SB_DAC->1
	dlv_write_reg_bit(5, 1, 5);//SB_MIX->1

	// 2010-01-20 Jason masked
//	dlv_write_reg_bit(6, 1, 0);//SB_SLEEP->1
//	dlv_write_reg_bit(6, 1, 1);//SB->1
	LEAVE();
}

#endif

static int dlv_set_replay_speed(int rate)
{
	int speed = 0, val;

	int mrate[MAX_RATE_COUNT] = {
		96000, 48000, 44100, 32000, 24000,
		22050, 16000, 12000, 11025, 8000
	};

	for (val = 0; val < MAX_RATE_COUNT; val++) {
		if (rate >= mrate[val]) {
			speed = val;
			break;
		}
	}
	if (rate < mrate[MAX_RATE_COUNT - 1]) {
		speed = MAX_RATE_COUNT - 1;
	}
	val = dlv_read_reg(DLV_REG_CCR2);
	val &= 0xf;
	val = (speed << 4) | val;
	dlv_write_reg(DLV_REG_CCR2, val);
	return mrate[speed];
}

static int dlv_set_record_speed(int rate)
{
	int speed = 0, val;

	int mrate[MAX_RATE_COUNT] = {
		96000, 48000, 44100, 32000, 24000,
		22050, 16000, 12000, 11025, 8000
	};

	for (val = 0; val < MAX_RATE_COUNT; val++) {
		if (rate >= mrate[val]) {
			speed = val;
			break;
		}
	}
	if (rate < mrate[MAX_RATE_COUNT - 1]) {
		speed = MAX_RATE_COUNT - 1;
	}
	val = dlv_read_reg(DLV_REG_CCR2);
	val &= 0xf0;
	val = speed | val;
	dlv_write_reg(DLV_REG_CCR2, val);

	return mrate[speed];
}

#if 0
static void reset_jzcodec(void)
{
	;
}
#endif

static void dlv_get_mixer_old_info(mixer_info *info)
{
	strncpy(info->id, "JZDLV", sizeof(info->id));
	strncpy(info->name, "Jz internal codec dlv on jz4750", sizeof(info->name));
}

static void dlv_get_mixer_info(mixer_info *old_info)
{
	strncpy(old_info->id, "JZDLV", sizeof(old_info->id));
	strncpy(old_info->name, "Jz internal codec dlv on jz4750", sizeof(old_info->name));
}

static void dlv_set_mic(int val)
{
	int cur_vol;

	ENTER();

	printk("~~~~ dlv_set_mic\n");

	/* set GIM */

	/* set gain */
	cur_vol = 31 * val / 100;
	cur_vol = 0x80  | cur_vol;
	dlv_write_reg(DLV_REG_GCR8, cur_vol);//GIL,GIR

	LEAVE();
}

static void dlv_set_line(int val)
{
	int cur_vol;

	ENTER();
	/* set gain */
	cur_vol = 31 * val / 100;
	cur_vol &= 0x1f;
	/* ???
	dlv_write_reg(11, cur_vol);//GO1L
	dlv_write_reg(12, cur_vol);//GO1R
	*/
	LEAVE();
}

static void dlv_set_volume(int val)
{
	unsigned long cur_vol;

	ENTER();

	/* To protect circut and to avoid shutting down CODEC,
	 * valume must less then 60% of the max
	 */

	if (val > 60) {
		val = 60;
	}


	//val = 90;
	cur_vol = 31 * (100 - val) / 100;

	dlv_write_reg(DLV_REG_GCR1, 0x80 | cur_vol);

	DPRINT_CODEC("$$$$$ val = %d, DLV_REG_CGR1 = 0x%02x\n",
		     val, dlv_read_reg(DLV_REG_GCR1));

	LEAVE();
}

/*
 * Base on set_record_mic_input_audio_without_playback()
 */
static void dlv_set_record(void)
{
	ENTER();

//	dump_dlv_regs("enter dlv_set_record");

	/* ADC path for MIC IN */

	dlv_write_reg_bit(DLV_REG_AGC1, 0, AGC1_AGCEN);//AGC1.AGC_EN->0

	dlv_write_reg_bit(DLV_REG_PMR1, 0, PMR1_SB);
	dlv_write_reg_bit(DLV_REG_PMR1, 0, PMR1_SB_SLEEP);

	switch_SB_ADC(POWER_ON);

	/* MIC 1*/
	dlv_write_reg_bit(DLV_REG_PMR1, POWER_ON, PMR1_SB_MIC1);

	/* MIC 2 */
	//dlv_write_reg_bit(DLV_REG_PMR1, POWER_ON, PMR1_SB_MIC2);

	dlv_write_reg_bit(DLV_REG_PMR1, POWER_ON, PMR1_SB_MICBIAS);

	switch_SB_LINE_IN(POWER_OFF);

	schedule_timeout(2);

	// Record mono
	dlv_write_reg(DLV_REG_CR3, 0);
	dlv_write_reg_bit(DLV_REG_CR3, 1, CR3_MICDIFF);

	/* MIC 1 */
	dlv_write_reg_bit(DLV_REG_CR3, 0, CR3_INSEL1);
	dlv_write_reg_bit(DLV_REG_CR3, 0, CR3_INSEL0);

	/* MIC 2 */
	//dlv_write_reg_bit(DLV_REG_CR3, 0, CR3_INSEL1);
	//dlv_write_reg_bit(DLV_REG_CR3, 1, CR3_INSEL0);

	dlv_write_reg_bit(DLV_REG_CR4, 0, 0);

	dlv_write_reg_bit(DLV_REG_PMR1, 1, PMR1_SB_BYPASS);

	// GIM
//	dlv_write_reg(DLV_REG_GCR7, 0x3f);
	dlv_write_reg(DLV_REG_GCR7, 0x0);

	// GIDR
//	dlv_write_reg(DLV_REG_GCR8, 0xff);
	dlv_write_reg(DLV_REG_GCR8, 0x0);
	// GID
//	dlv_write_reg(DLV_REG_GCR9, 0x1f);
	dlv_write_reg(DLV_REG_GCR9, 0x0);



//	dump_dlv_regs("leave dlv_set_record");


	LEAVE();
}

static void dlv_set_replay_recode(int val)
{
	ENTER();
	if (val == USE_LINEIN) {
		/* Record LINE input audio with Audio data replay (full duplex for linein) */
		/* codec_test_line */
		printk("use line in ???\n");
//		set_record_line_input_audio_with_audio_data_replay();
	}
	if (val == USE_MIC) {
		/* Record MIC input audio with Audio data replay (full duplex) */
		/* codec_test_mic */
//		set_record_mic_input_audio_with_audio_data_replay();
	}
	LEAVE();
}

static void dlv_anti_pop(int mode)
{
//	dump_dlv_regs("++++++++++++++");
	switch(mode) {
	case CODEC_WRMODE:
		//set SB_ADC or SB_DAC
		//dlv_write_reg_bit(5, 0, 6);//PMR1.SB_OUT->0
		switch_SB_OUT(POWER_ON);

		//2010-01-31 Jason add
		//dlv_write_reg(22, 0x40);//mic 1

		//2010-01-31 Jason add
		//dlv_write_reg(1, 0x04);

		schedule_timeout(28); //280 ms
		break;
	case CODEC_RMODE:
		printk("dlv_anti_pop CODEC_WMODE\n");
		switch_SB_MIC1(POWER_ON);
		break;
	case CODEC_WMODE:
		//dlv_write_reg_bit(5, 0, 6);//PMR1.SB_OUT->0
		printk("dlv_anti_pop CODEC_WMODE\n");
		switch_SB_OUT(POWER_ON);

		schedule_timeout(28); //280 ms

		// 2010-01-31 Jason marked
		//dlv_write_reg_bit(5, 1, 4);//SB_ADC->1

		//2010-01-31 Jason add
		//dlv_write_reg(1, 0x04);

		//dlv_write_reg_bit(5, 0, 7);//PMR1.SB_OUT->0
		switch_SB_DAC(POWER_ON);

		break;
	}
//	dump_dlv_regs("--------------");
}

static void dlv_turn_replay(int mode)
{
	ENTER();
	if (mode == USE_LINEIN) {
		printk("use line in ?????????\n");
		//unset_record_line_input_audio_with_audio_data_replay();
	}
	if (mode == USE_MIC) {
		//unset_record_mic_input_audio_with_audio_data_replay();
	}
	LEAVE();
}

static void dlv_turn_off(int mode)
{
	ENTER();

	if ((mode & REPLAY) && (mode & RECORD)) {
		printk("Close DLV !!!\n");
//		dlv_write_reg_bit(1, 1, 5);//DAC_MUTE->1
		schedule_timeout(20);

		// 2010-01-31 Jason marked
		//dlv_write_reg_bit(5, 1, 6);//SB_OUT->1

		dlv_write_reg(9, 0xff);
		dlv_write_reg(8, 0x2f);
	} else if (mode & REPLAY) {
		//nothing
	} else if (mode & RECORD) {
		printk("Close RECORD\n");
//		dlv_write_reg(4, 0x20);
	}

	LEAVE();
}

static int dlv_set_channel(int ch)
{
	if(ch > 2) ch = 2;
	if(ch < 1) ch = 1;
	switch (ch) {
	case 1:
		dlv_write_reg_bit(DLV_REG_CR2, 1, CR2_MONO);// MONO->1 for Mono
		break;
	case 2:
		dlv_write_reg_bit(DLV_REG_CR2, 0, CR2_MONO);// MONO->0 for Stereo
		break;
	}
	return ch;
}

static int dlv_set_data_width(unsigned int mode, unsigned int width)
{
	unsigned char cr2 = dlv_read_reg(DLV_REG_AICR);
	unsigned char savecr2 = cr2;
	int supported_width[4] = {16, 18, 20, 24};
	int i;

	for (i = 0; i < (sizeof(supported_width) / sizeof(supported_width[0])); i++) {
		if (supported_width[i] <= width) {
			break;
		}
	}

	if (i == (sizeof(supported_width) / sizeof(supported_width[0]))) {
		// For 8 bit width mode, handle it as 16 bit
		if (width == 8) {
			i = 0;
		} else {
			return -1;
		}
	}

	//printk("mode = %d, width = %d, selected %d\n", mode, width, i);

	switch (mode) {
	case RECORD:
		cr2 &= ~(3 << 4);
		cr2 |= (i << 4);
		break;
	case REPLAY:
		cr2 &= ~(3 << 6);
		cr2 |= (i << 6);
		break;
	}

	if (cr2 != savecr2) {
		dlv_write_reg(DLV_REG_AICR, cr2);
	}

	printk("set cr2 = %x, %x\n", cr2, savecr2);

	if (width == 8) {
		return 8;
	} else {
		return supported_width[i];
	}
}

static int dlv_mute(int val)
{
	return dlv_write_reg_bit(DLV_REG_CR2, val ? 1 : 0, CR2_DAC_MUTE);
}

void dump_dlv_regs(const char * str)
{
	unsigned int i;
	unsigned char data;
	printk("codec register dump, %s:\n", str);
	for (i = 0; i < 32; i++) {
		data = dlv_read_reg(i);
		printk("address = 0x%02x, data = 0x%02x\n", i, data);
	}
}

static int jzdlv_ioctl(void *context, unsigned int cmd, unsigned long arg)
{
	ENTER();
	DUMP_CODEC_REGS(__FUNCTION__);
	DPRINT_CODEC("[dlv IOCTL]++++++++++++++++++++++++++++\n");
	DPRINT_CODEC("%s  cmd = %d, arg = %lu\n", __FUNCTION__, cmd, arg);
	DPRINT_DLV_IOC_CMD(cmd);
	DPRINT_CODEC("[dlv IOCTL]----------------------------\n");

	switch (cmd) {
	case CODEC_SET_MODE:
		dlv_set_mode();
		break;

	case CODEC_SET_STARTUP_PARAM:
		dlv_set_startup_param();
		break;

	case CODEC_SET_REPLAY:
		dlv_set_replay();
		break;

	case CODEC_SET_RECORD:
		dlv_set_record();
		break;

	case CODEC_SET_REPLAY_RECORD:
		dlv_set_replay_recode(arg);
		break;

	case CODEC_SET_VOLUME:
		dlv_set_volume(arg);
		break;

	case CODEC_SET_MIC:
		dlv_set_mic(arg);
		break;

	case CODEC_SET_LINE:
		dlv_set_line(arg);
		break;

	case CODEC_EACH_TIME_INIT:
		dlv_each_time_init();
		break;

	case CODEC_RESET:
		dlv_reset();
		break;

	case CODEC_ANTI_POP:
		dlv_anti_pop(arg);
		break;

	case CODEC_TURN_REPLAY:
		dlv_turn_replay(arg);
		break;

	case CODEC_TURN_OFF:
		dlv_turn_off(arg);
		break;

	case CODEC_GET_MIXER_INFO:
		dlv_get_mixer_info((mixer_info *)arg);
		break;

	case CODEC_GET_MIXER_OLD_INFO:
		dlv_get_mixer_old_info((mixer_info *)arg);
		break;

	case CODEC_SET_REPLAY_SPEED:
		return dlv_set_replay_speed(arg);

	case CODEC_SET_RECORD_SPEED:
		return dlv_set_record_speed(arg);

	case CODEC_SET_RECORD_CHANNEL:
		return arg;

	case CODEC_SET_REPLAY_CHANNEL:
		return dlv_set_channel(arg);

	case CODEC_SET_RECORD_DATA_WIDTH:
		return dlv_set_data_width(RECORD, arg);

	case CODEC_SET_REPLAY_DATA_WIDTH:
		return dlv_set_data_width(REPLAY, arg);

	case CODEC_DAC_MUTE:
		return dlv_mute(arg);

	default:
		printk("%s:%d no support\n", __FUNCTION__, __LINE__);
		return -1;
	}

	LEAVE();
	return 0;
}

static struct work_struct dlv_work;

/*
 * work handler
 *
 * Mission:
 *	Restart CODEC after shut down by short circurt protection
 */
static void dlv_work_handle(struct work_struct *work)
{

	printk("CODEC: short circurt detected!\n");

//	dump_dlv_regs("IRQ ---------");

	/* Renable SB OUT */
	switch_SB_OUT(POWER_OFF);
	mdelay(300);

	while (!(dlv_read_reg(DLV_REG_IFR) & (1 << IFR_RDO))) {
		;/* nothing */
	}

	while (dlv_read_reg(DLV_REG_IFR) & (1 << IFR_SCMC)) {
		dlv_write_reg(DLV_REG_IFR, 1 << IFR_SCMC);
	}
	switch_SB_OUT(POWER_ON);
	mdelay(300);

	while (!(dlv_read_reg(DLV_REG_IFR) & (1 << IFR_RUP))) {
		;/* nothing */
	}

	/* Enable SCMC and JACK EVENT interrupt ... */
	dlv_write_reg(DLV_REG_ICR, 0x0f);
}

static spinlock_t dlv_irq_lock;

static irqreturn_t dlv_codec_irq(int irq, void *dev_id)
{
//	unsigned char dlv_icr;
	unsigned char dlv_ifr;

	spin_lock(dlv_irq_lock);

	/* Clear interrupt flag */
	dlv_ifr = dlv_read_reg(DLV_REG_IFR);
	dlv_write_reg(DLV_REG_IFR, dlv_ifr);

	/* Mask SCMC and JACK EVENT temporarily */
	dlv_write_reg(DLV_REG_ICR, 0x3f);

	REG_AIC_SR = 0x78; //???

	/*
	 * Start handler when output short circuit has been detected
	 */
	if (dlv_ifr & (1 << IFR_SCMC)) {
		schedule_work(&dlv_work);
	} else {
		printk("Jack event ???\n");
//		dump_dlv_regs("irq -- maybe jack event");
	}

#if HP_SENSE_DETECT
	/* Jack event detected */
	if (dlv_ifr & (1 << IFR_JACK_EVENT)) {

		printk("Jack event ! dlv_ifr = 0x%08x\n", dlv_ifr);

		/* IFR_JACK indicate the status of jack:
		 * 	1. The jack is pluged: plug event
		 *	2. The jack is not pluged: unplug event
		 */
		switch_set_state(&data->sdev, dlv_ifr & (1 << IFR_JACK));
	}
#endif
	spin_unlock(dlv_irq_lock);
	return IRQ_HANDLED;
}

#if HP_SENSE_DETECT
/*
 * HP_SENSE switch
 */
#define gpio_to_irq(gpio)	(IRQ_GPIO_0 + (gpio))

struct hp_switch_data {
	struct switch_dev	sdev;
	unsigned int		gpio;
	const char		*name_on;
	const char		*name_off;
	const char		*state_on;
	const char		*state_off;
};

static int __devexit hp_switch_remove(struct platform_device *pdev)
{
	struct gpio_switch_data *switch_data = platform_get_drvdata(pdev);

	switch_dev_unregister(&switch_data->sdev);
	kfree(switch_data);

	return 0;
}

static struct platform_driver hp_switch_driver = {
	.probe		= hp_switch_probe,
	.remove		= __devexit_p(hp_switch_remove),
	.driver		= {
		.name	= "switch-gpio",
		.owner	= THIS_MODULE,
	},
};
/* HP_SENSE switch end */
#endif

static int __init init_dlv(void)
{
	int retval;

	cpm_start_clock(CGM_AIC);

	spin_lock_init(&dlv_irq_lock);
	INIT_WORK(&dlv_work, dlv_work_handle);
	register_jz_codecs((void *)jzdlv_ioctl);

#if HP_SENSE_DETECT
	retval = platform_driver_register(&hp_switch_driver);
	if (retval) {
		printk("Could net register headphone sense switch\n");
		return retval;
	}
#endif

	dlv_reset();

	retval = request_irq(IRQ_AIC, dlv_codec_irq, IRQF_DISABLED, "dlv_codec_irq", NULL);
	if (retval) {
		printk("Could not get aic codec irq %d\n", IRQ_AIC);
		return retval;
	}

	return 0;
}

static void __exit cleanup_dlv(void)
{
	free_irq(IRQ_AIC, NULL);
#if HP_SENSE_DETECT
	platform_driver_unregister(&hp_switch_driver);
#endif
}

module_init(init_dlv);
module_exit(cleanup_dlv);
