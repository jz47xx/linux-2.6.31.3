/*
 * Linux/sound/oss/jz4760_dlv.h
 *
 * Copyright (c) Ingenic Semiconductor Co., Ltd.
 */

#ifndef __JZ4770_DLV_H__
#define __JZ4770_DLV_H__

/* Power setting */
#define POWER_ON		0
#define POWER_OFF		1

/* File ops mode W & R */
#define REPLAY			1
#define RECORD			2

#define DLC_STATUS		0x0
#define DLC_SR_PON_ACK	(1 << 7)
#define DLC_SR_IRQ_ACK	(1 << 6)
#define DLC_SR_JACK_PRESENT	(1 << 5)

#define DLC_AICR_DAC		0x1
#define DLC_DAC_ADWL_LSB	6
#define DLC_DAC_ADWL_MASK	(0x3 << 6)
#define DLC_DAC_ADWL_16BIT	(0x0 << 6)
#define DLC_DAC_ADWL_18BIT	(0x1 << 6)
#define DLC_DAC_ADWL_20BIT	(0x2 << 6)
/* default */
#define DLC_DAC_ADWL_24BIT	(0x3 << 6)

#define __dlv_dac_16bit_sample() dlv_update_reg(DLC_AICR_DAC, DLC_DAC_ADWL_16BIT, DLC_DAC_ADWL_MASK)
#define __dlv_dac_18bit_sample() dlv_update_reg(DLC_AICR_DAC, DLC_DAC_ADWL_18BIT, DLC_DAC_ADWL_MASK)
#define __dlv_dac_20bit_sample() dlv_update_reg(DLC_AICR_DAC, DLC_DAC_ADWL_20BIT, DLC_DAC_ADWL_MASK)
#define __dlv_dac_24bit_sample() dlv_update_reg(DLC_AICR_DAC, DLC_DAC_ADWL_24BIT, DLC_DAC_ADWL_MASK)

/* Note: should be set to 1 */
#define DLC_DAC_SERIAL	(1 << 1)
/* Note: should be set to 1 */
#define DLC_DAC_I2S_MODE	(1 << 0)

#define __dlv_set_dac_serial() dlv_write_reg_bit(DLC_AICR_DAC, 1, DLC_DAC_SERIAL)
#define __dlv_dac_i2s_mode() dlv_write_reg_bit(DLC_AICR_DAC, 1, DLC_DAC_I2S_MODE)


#define DLC_AICR_ADC		0x2
#define DLC_ADC_ADWL_LSB	6
#define DLC_ADC_ADWL_MASK	(0x3 << 6)
#define DLC_ADC_ADWL_16BIT	(0x0 << 6)
#define DLC_ADC_ADWL_18BIT	(0x1 << 6)
#define DLC_ADC_ADWL_20BIT	(0x2 << 6)
/* default */
#define DLC_ADC_ADWL_24BIT	(0x3 << 6)

#define __dlv_adc_16bit_sample() dlv_update_reg(DLC_AICR_ADC, DLC_ADC_ADWL_16BIT, DLC_ADC_ADWL_MASK)
#define __dlv_adc_18bit_sample() dlv_update_reg(DLC_AICR_ADC, DLC_ADC_ADWL_18BIT, DLC_ADC_ADWL_MASK)
#define __dlv_adc_20bit_sample() dlv_update_reg(DLC_AICR_ADC, DLC_ADC_ADWL_20BIT, DLC_ADC_ADWL_MASK)
#define __dlv_adc_24bit_sample() dlv_update_reg(DLC_AICR_ADC, DLC_ADC_ADWL_24BIT, DLC_ADC_ADWL_MASK)

/* Note: should be set to 1 */
#define DLC_ADC_SERIAL	(1 << 1)
/* Note: should be set to 1 */
#define DLC_ADC_I2S_MODE	(1 << 0)

#define __dlv_set_adc_serial() dlv_write_reg_bit(DLC_AICR_ADC, 1, DLC_ADC_SERIAL)
#define __dlv_adc_i2s_mode() dlv_write_reg_bit(DLC_AICR_ADC, 1, DLC_ADC_I2S_MODE)

#define DLC_CR_LO		0x3
#define DLC_LO_MUTE		(1 << 7)
#define DLC_SB_LO		(1 << 4)
#define DLC_LO_SEL_LSB	0
#define DLC_LO_SEL_MASK	(0x3 << 0)
#define DLC_LO_SEL_MIC1	(0x0 << 0)
#define DLC_LO_SEL_MIC2	(0x1 << 0)
#define DLC_LO_SEL_BYPASS	(0x2 << 0)
#define DLC_LO_SEL_DAC	(0X3 << 0)
/* if MICSTEREO */
#define DLC_LO_SEL_MIC12	(0x0 << 0)

#define __dlv_get_sb_line_out()		((dlv_read_reg(DLC_CR_LO) & DLC_SB_LO) ? POWER_OFF : POWER_ON)

#define __dlv_switch_sb_line_out(pwrstat)				\
	do {								\
		if (__dlv_get_sb_line_out() != pwrstat) {		\
			dlv_write_reg_bit(DLC_CR_LO, pwrstat, DLC_SB_LO); \
		}							\
	} while (0)

#define __dlv_enable_lineout_mute()					\
	do {								\
		dlv_write_reg_bit(DLC_CR_LO, 1, DLC_LO_MUTE);		\
	} while (0)

#define __dlv_disable_lineout_mute()					\
	do {								\
		dlv_write_reg_bit(DLC_CR_LO, 0, DLC_LO_MUTE);		\
	} while (0)

#define __dlv_lineout_from_mic1()	dlv_update_reg(DLC_CR_LO, DLC_LO_SEL_MIC1, DLC_LO_SEL_MASK)
#define __dlv_lineout_from_mic2()	dlv_update_reg(DLC_CR_LO, DLC_LO_SEL_MIC2, DLC_LO_SEL_MASK)
#define __dlv_lineout_from_bypass()	dlv_update_reg(DLC_CR_LO, DLC_LO_SEL_BYPASS, DLC_LO_SEL_MASK)
#define __dlv_lineout_from_dac()	dlv_update_reg(DLC_CR_LO, DLC_LO_SEL_DAC, DLC_LO_SEL_MASK)
#define __dlv_lineout_from_mic12()	dlv_update_reg(DLC_CR_LO, DLC_LO_SEL_MIC12, DLC_LO_SEL_MASK)

#define DLC_CR_HP		0x4
#define DLC_HP_MUTE		(1 << 7)
#define DLC_HP_LOAD		(1 << 6)
#define DLC_SB_HP		(1 << 4)
#define DLC_SB_HPCM		(1 << 3)

#define __dlv_get_sb_hp()		((dlv_read_reg(DLC_CR_HP) & DLC_SB_HP) ? POWER_OFF : POWER_ON)

#define __dlv_switch_sb_hp(pwrstat)					\
	do {								\
		if (__dlv_get_sb_hp() != pwrstat) {			\
			dlv_write_reg_bit(DLC_CR_HP, pwrstat, DLC_SB_HP); \
		}							\
									\
	} while (0)

#define __dlv_enable_hp_mute()					\
	do {							\
		dlv_write_reg_bit(DLC_CR_HP, 1, DLC_HP_MUTE);	\
								\
	} while (0)

#define __dlv_disable_hp_mute()					\
	do {							\
		dlv_write_reg_bit(DLC_CR_HP, 0, DLC_HP_MUTE);	\
								\
	} while (0)

#define __dlv_set_16ohm_load()					\
	do {							\
		dlv_write_reg_bit(DLC_CR_HP, 0, DLC_HP_LOAD);	\
	} while (0)

#define __dlv_set_10kohm_load()					\
	do {							\
		dlv_write_reg_bit(DLC_CR_HP, 1, DLC_HP_LOAD);	\
	} while (0)

#define __dlv_set_cap_less() dlv_write_reg_bit(DLC_CR_HP, 0, DLC_SB_HPCM)
#define __dlv_set_cap_couple() dlv_write_reg_bit(DLC_CR_HP, 1, DLC_SB_HPCM)

#define DLC_HP_SEL_LSB	0
#define DLC_HP_SEL_MASK	0x3
#define DLC_HP_MIC1_TO_LR	0x0
#define DLC_HP_MIC2_TO_LR	0x1
#define DLC_HP_BYPASS		0x2
#define DLC_HP_DAC		0x3
/* if MICSTEREO */
#define DLC_HP_MIC1L_MIC2R	0x0
#define DLC_HP_MIC1R_MIC2L	0x1

#define __dlv_hp_mic1_to_lr() dlv_update_reg(DLC_CR_HP, DLC_HP_MIC1_TO_LR, DLC_HP_SEL_MASK)
#define __dlv_hp_mic2_to_lr() dlv_update_reg(DLC_CR_HP, DLC_HP_MIC2_TO_LR, DLC_HP_SEL_MASK)
#define __dlv_hp_linein_to_lr() dlv_update_reg(DLC_CR_HP, DLC_HP_BYPASS, DLC_HP_SEL_MASK)
#define __dlv_hp_dac_to_lr() dlv_update_reg(DLC_CR_HP, DLC_HP_DAC, DLC_HP_SEL_MASK)

#define __dlv_hp_mic1L_mic2R() dlv_update_reg(DLC_CR_HP, DLC_HP_MIC1L_MIC2R, DLC_HP_SEL_MASK)
#define __dlv_hp_mic1R_mic2L() dlv_update_reg(DLC_CR_HP, DLC_HP_MIC1R_MIC2L, DLC_HP_SEL_MASK)


#define DLC_CR_DAC		0x6
#define DLC_CR_DAC_MUTE	(1 << 7)
#define DLC_CR_DAC_MONO	(1 << 6)
#define DLC_CR_DAC_LEFT_ONLY	(1 << 5)
#define DLC_CR_SB_DAC		(1 << 4)
/* Note: LRSWAP should be set to 1 */
#define DLC_CR_DAC_LRSWAP	(1 << 3)

#define __dlv_set_dac_lrswap() dlv_write_reg_bit(DLC_CR_DAC, 1, DLC_CR_DAC_LRSWAP);

#define __dlv_get_dac_mute()        ((dlv_read_reg(DLC_CR_DAC) & DLC_CR_DAC_MUTE) ? 1 : 0)

#define __dlv_enable_dac_mute()						\
	do {								\
		dlv_write_reg_bit(DLC_CR_DAC, 1, DLC_CR_DAC_MUTE);	\
									\
	} while (0)

#define __dlv_disable_dac_mute()					\
	do {								\
		dlv_write_reg_bit(DLC_CR_DAC, 0, DLC_CR_DAC_MUTE);	\
									\
	} while (0)

#define __dlv_get_sb_dac()		((dlv_read_reg(DLC_CR_DAC) & DLC_CR_SB_DAC) ? POWER_OFF : POWER_ON)

#define __dlv_switch_sb_dac(pwrstat)					\
	do {								\
		if (__dlv_get_sb_dac() != pwrstat) {			\
			dlv_write_reg_bit(DLC_CR_DAC, pwrstat, DLC_CR_SB_DAC); \
		}							\
									\
	} while (0)

#define __dlv_enable_dac_mono()						\
	do {								\
		dlv_write_reg_bit(DLC_CR_DAC, 1, DLC_CR_DAC_MONO);	\
	} while (0)

#define __dlv_disable_dac_mono()						\
	do {								\
		dlv_write_reg_bit(DLC_CR_DAC, 0, DLC_CR_DAC_MONO);	\
	} while (0)

#define __dlv_enable_dac_left_only() dlv_write_reg_bit(DLC_CR_DAC, 1, DLC_CR_DAC_LEFT_ONLY)
#define __dlv_disable_dac_left_only() dlv_write_reg_bit(DLC_CR_DAC, 0, DLC_CR_DAC_LEFT_ONLY)


#define DLC_CR_MIC		0x7
#define DLC_CR_MIC_STEREO	(1 << 7)
#define DLC_CR_MICDIFF	(1 << 6)
#define DLC_CR_SB_MIC2	(1 << 5)
#define DLC_CR_SB_MIC1	(1 << 4)
#define DLC_CR_MICBIAS_V0	(1 << 1)
#define DLC_CR_SB_MICBIAS	(1 << 0)

#define __dlv_get_sb_micbias()		((dlv_read_reg(DLC_CR_MIC) & DLC_CR_SB_MICBIAS) ? POWER_OFF : POWER_ON)

#define __dlv_switch_sb_micbias(pwrstat)				\
	do {								\
		if (__dlv_get_sb_micbias() != pwrstat) {		\
			dlv_write_reg_bit(DLC_CR_MIC, pwrstat, DLC_CR_SB_MICBIAS); \
		}							\
									\
	} while (0)

#define __dlv_enable_mic_stereo()	dlv_write_reg_bit(DLC_CR_MIC, 1, DLC_CR_MIC_STEREO)
#define __dlv_disable_mic_stereo()	dlv_write_reg_bit(DLC_CR_MIC, 0, DLC_CR_MIC_STEREO)

#define __dlv_enable_micdiff() dlv_write_reg_bit(DLC_CR_MIC, 1, DLC_CR_MICDIFF)
#define __dlv_disable_micdiff() dlv_write_reg_bit(DLC_CR_MIC, 0, DLC_CR_MICDIFF)

#define __dlv_enable_mic1() dlv_write_reg_bit(DLC_CR_MIC, 0, DLC_CR_SB_MIC1)
#define __dlv_disable_mic1() dlv_write_reg_bit(DLC_CR_MIC, 1, DLC_CR_SB_MIC1)

#define __dlv_enable_mic2() dlv_write_reg_bit(DLC_CR_MIC, 0, DLC_CR_SB_MIC2)
#define __dlv_disable_mic2() dlv_write_reg_bit(DLC_CR_MIC, 1, DLC_CR_SB_MIC2)

#define DLC_CR_LI		0x8
#define DLC_CR_LI_SB_LIBY	(1 << 4)
#define DLC_CR_LI_SB_LIN	(1 << 0)

#define __dlv_enable_li_for_bypass() dlv_write_reg_bit(DLC_CR_LI, 0, DLC_CR_LI_SB_LIBY)
#define __dlv_disable_li_for_bypass() dlv_write_reg_bit(DLC_CR_LI, 1, DLC_CR_LI_SB_LIBY)

#define __dlv_enable_li_for_adc() dlv_write_reg_bit(DLC_CR_LI, 0, DLC_CR_LI_SB_LIN)
#define __dlv_disable_li_for_adc() dlv_write_reg_bit(DLC_CR_LI, 1, DLC_CR_LI_SB_LIN)

#define DLC_CR_ADC		0x9
#define DLC_CR_ADC_DMIC_SEL	(1 << 7)
#define DLC_CR_ADC_MONO	(1 << 6)
#define DLC_CR_ADC_LEFT_ONLY	(1 << 5)
#define DLC_CR_SB_ADC		(1 << 4)
/* NOTE: LRSWAP should be set to 1 */
#define DLC_CR_ADC_LRSWAP	(1 << 3)

#define __dlv_enable_adc() dlv_write_reg_bit(DLC_CR_ADC, 0, DLC_CR_SB_ADC)
#define __dlv_disable_adc() dlv_write_reg_bit(DLC_CR_ADC, 1, DLC_CR_SB_ADC)

#define __dlv_enable_adc_left_only()	dlv_write_reg_bit(DLC_CR_ADC, 1, DLC_CR_ADC_LEFT_ONLY)
#define __dlv_disable_adc_left_only()	dlv_write_reg_bit(DLC_CR_ADC, 0, DLC_CR_ADC_LEFT_ONLY)

#define __dlv_enable_adc_mono()	dlv_write_reg_bit(DLC_CR_ADC, 1, DLC_CR_ADC_MONO)
#define __dlv_disable_adc_mono()	dlv_write_reg_bit(DLC_CR_ADC, 0, DLC_CR_ADC_MONO)

#define __dlv_enable_adc_lrswap()	dlv_write_reg_bit(DLC_CR_ADC, 1, DLC_CR_ADC_LRSWAP)
#define __dlv_disable_adc_lrswap()	dlv_write_reg_bit(DLC_CR_ADC, 0, DLC_CR_ADC_LRSWAP)

#define DLC_CR_IN_SEL_LSB	0
#define DLC_CR_IN_SEL_MASK	0x3
#define DLC_CR_MIC1_TO_LR	0x0
#define DLC_CR_MIC2_TO_LR	0x1
#define DLC_CR_LINE_IN	0x2
/* if MIC_STEREO */
#define DLC_CR_MIC1L_MIC2R	0x0
#define DLC_CR_MIC1R_MIC2L	0x1

#define __dlv_adc_mic1_to_lr()	dlv_update_reg(DLC_CR_ADC, DLC_CR_MIC1_TO_LR, DLC_CR_IN_SEL_MASK)
#define __dlv_adc_mic2_to_lr()	dlv_update_reg(DLC_CR_ADC, DLC_CR_MIC2_TO_LR, DLC_CR_IN_SEL_MASK)
#define __dlv_adc_linein_to_lr()	dlv_update_reg(DLC_CR_ADC, DLC_CR_LINE_IN, DLC_CR_IN_SEL_MASK)

#define __dlv_adc_mic1L_mic2R()	dlv_update_reg(DLC_CR_ADC, DLC_CR_MIC1L_MIC2R, DLC_CR_IN_SEL_MASK)
#define __dlv_adc_mic1R_mic2L()	dlv_update_reg(DLC_CR_ADC, DLC_CR_MIC2L_MIC1R, DLC_CR_IN_SEL_MASK)


#define DLC_CR_MIX		0xA
#define DLC_CR_MIX_REC_LSB	2
#define DLC_CR_MIX_REC_MASK	(0x3 << 2)
#define DLC_CR_REC_INPUT	(0x0 << 2)
#define DLC_CR_REC_IN_A_DAC	(0x1 << 2)


#define DLC_CR_DAC_MIX_LSB	0
#define DLC_CR_DAC_MIX_MASK	0x3
#define DLC_CR_PLAY_DAC	0x0
#define DLC_CR_PLAY_DAC_ADC	0x1


#define DLC_CR_VIC		0xB
#define DLC_VIC_SB_SLEEP	(1 << 1)
#define DLC_VIC_SB		(1 << 0)

#define __dlv_get_sb()			((dlv_read_reg(DLC_CR_VIC) & DLC_VIC_SB) ? POWER_OFF : POWER_ON)

#define __dlv_switch_sb(pwrstat)					\
	do {								\
		if (__dlv_get_sb() != pwrstat) {			\
			dlv_write_reg_bit(DLC_CR_VIC, pwrstat, DLC_VIC_SB); \
		}							\
	} while (0)

#define __dlv_get_sb_sleep()		((dlv_read_reg(DLC_CR_VIC) & DLC_VIC_SB_SLEEP) ? POWER_OFF : POWER_ON)

#define __dlv_switch_sb_sleep(pwrstat)					\
	do {								\
		if (__dlv_get_sb_sleep() != pwrstat) {			\
			dlv_write_reg_bit(DLC_CR_VIC, pwrstat, DLC_VIC_SB_SLEEP); \
		}							\
									\
	} while (0)

#define DLC_CCR		0xC
#define DLC_CCR_DMIC_CLKON	(1 << 7)
#define DLC_CCR_CRYSTAL_LSB	0
#define DLC_CCR_CRYSTAL_MASK	0xf
/* default to 12MHZ(Note: must be 12MHZ) */
#define DLC_CCR_CRYSTAL_12M	0x0
#define DLC_CCR_CRYSTAL_13M	0x1

#define __dlv_set_12m_crystal()						\
	do {								\
		dlv_update_reg(DLC_CCR, DLC_CCR_CRYSTAL_12M, DLC_CCR_CRYSTAL_MASK); \
	} while (0)

#define DLC_FCR_DAC		0xD
#define DLC_DAC_FREQ_LSB	0
#define DLC_DAC_FREQ_MASK	0xf

#define __dlv_set_dac_sample_rate(r)		\
	dlv_update_reg(DLC_FCR_DAC,		\
		       (r) << DLC_DAC_FREQ_LSB, \
		       DLC_DAC_FREQ_MASK)

#define DLC_FCR_ADC		0xE
#define DLC_ADC_HPF		(1 << 6)
#define DLC_ADC_FREQ_LSB	0
#define DLC_ADC_FREQ_MASK	0xF

#define __dlv_set_adc_sample_rate(r)		\
	dlv_update_reg(DLC_FCR_ADC,		\
		       (r) << DLC_ADC_FREQ_LSB, \
		       DLC_ADC_FREQ_MASK)


#define DLC_ICR		0xF
#define DLC_ICR_INT_FORM_LSB	6
#define DLC_ICR_INT_FORM_MASK	(0x3 << 6)
#define DLC_ICR_INT_HIGH	(0x0 << 6)
#define DLC_ICR_INT_LOW	(0x1 << 6)
#define DLC_ICR_INT_HPULSE_8	(0x2 << 6)
#define DLC_ICR_INT_LPULSE_8	(0x3 << 6)

#define __dlv_set_int_form(v)						\
	do {								\
		dlv_update_reg(DLC_ICR, (v), DLC_ICR_INT_FORM_MASK);	\
	} while (0)

#define DLC_IMR		0x10
#define DLC_IMR_SCLR	(1 << 6)
#define DLC_IMR_JACK	(1 << 5)
#define DLC_IMR_SCMC2	(1 << 4)
#define DLC_IMR_RUP	(1 << 3)
#define DLC_IMR_RDO	(1 << 2)
#define DLC_IMR_GUP	(1 << 1)
#define DLC_IMR_GDO	(1 << 0)

#define ICR_ALL_MASK							\
	(DLC_IMR_SCLR | DLC_IMR_JACK | DLC_IMR_SCMC2 | DLC_IMR_RUP | DLC_IMR_RDO | DLC_IMR_GUP | DLC_IMR_GDO)

#define ICR_COMMON_MASK	(ICR_ALL_MASK & (~DLC_IMR_SCLR) & (~DLC_IMR_SCMC2))

#define __dlv_set_irq_mask(mask)		\
	do {					\
		dlv_write_reg(DLC_IMR, mask);	\
						\
	} while (0)

#define DLC_IFR		0x11
#define DLC_IFR_SCLR	(1 << 6)
#define DLC_IFR_JACK	(1 << 5)
#define DLC_IFR_SCMC2	(1 << 4)

/* Important:  RUP, RDO, GUP, GDO should be reset after 4 cycles of SYS_CLK  */
#define DLC_IFR_RUP	(1 << 3)
#define DLC_IFR_RDO	(1 << 2)
#define DLC_IFR_GUP	(1 << 1)
#define DLC_IFR_GDO	(1 << 0)
#define IFR_ALL_FLAG	(DLC_IFR_SCLR | DLC_IFR_JACK | DLC_IFR_SCMC2 | DLC_IFR_RUP | DLC_IFR_RDO | DLC_IFR_GUP | DLC_IFR_GDO)

#define __dlv_set_irq_flag(flag)			\
	do {						\
		dlv_write_reg(DLC_IFR, flag);	\
							\
	} while (0)

#define __dlv_get_irq_flag()		(dlv_read_reg(DLC_IFR))

#define DLC_GCR_HPL	0x12
#define DLC_GCR_HPL_LRGO	(1 << 7)
#define DLC_GOL_LSB	0
#define DLC_GOL_MASK	0x1f

#define DLC_GCR_HPR	0x13
#define DLC_GOR_LSB	0
#define DLC_GOR_MASK	0x1f

#define __dlv_set_hp_volume(vol)					\
	do {								\
		int ___vol = ((vol) & DLC_GOL_MASK) | DLC_GCR_HPL_LRGO; \
		dlv_write_reg(DLC_GCR_HPL, ___vol);			\
		dlv_write_reg(DLC_GCR_HPR, ((vol) & DLC_GOL_MASK));	\
	} while (0)

#define __dlv_set_hp_volume_lr(lvol, rvol)		\
	do {						\
		int ___lvol = (lvol) & DLC_GOL_MASK;	\
		int ___rvol = (rvol) & DLC_GOR_MASK;	\
		dlv_write_reg(DLC_GCR_HPL, ___lvol);	\
		dlv_write_reg(DLC_GCR_HPR, ___rvol);	\
	} while(0)

#define DLC_GCR_LIBYL	0x14
#define DLC_GCR_LIBYL_LRGI	(1 << 7)
#define DLC_GIL_LSB	0
#define DLC_GIL_MASK	0x1f

#define DLC_GCR_LIBYR	0x15
#define DLC_GIR_LSB	0
#define DLC_GIR_MASK	0x1f

#define __dlv_set_line_in_bypass_volume(vol)				\
	do {								\
		int ___vol = (vol & DLC_GIL_MASK) | DLC_GCR_LIBYL_LRGI;	\
		dlv_write_reg(DLC_GCR_LIBYL, ___vol);			\
	} while (0)

#define __dlv_set_line_in_bypass_volume_rl(lvol, rvol)	\
	do {						\
		int ___lvol = (lvol) & DLC_GIL_MASK;	\
		int ___rvol = (rvol) & DLC_GIR_MASK;	\
		dlv_write_reg(DLC_GCR_LIBYL, ___lvol);	\
		dlv_write_reg(DLC_GCR_LIBYR, ___rvol);	\
	} while (0)

#define DLC_GCR_DACL	0x16
#define DLC_GCR_RLGOD	(1 << 7)
#define DLC_GODL_LSB	0
#define DLC_GODL_MASK	0x1f

#define DLC_GCR_DACR	0x17
#define DLC_GODR_LSB	0
#define DLC_GODR_MASK	0x1f

#define __dlv_set_dac_gain(g)						\
	do {								\
		int ___g = ((g) & DLC_GODL_MASK) | DLC_GCR_RLGOD;	\
		dlv_write_reg(DLC_GCR_DACL, ___g);			\
	} while(0)

#define __dlv_set_dac_gain_lr(lg, rg)			\
	do {						\
		int ___lg = (lg) & DLC_GODL_MASK;	\
		int ___rg = (rg) & DLC_GODR_MASK;	\
		dlv_write_reg(DLC_GCR_DACL, ___lg);	\
		dlv_write_reg(DLC_GCR_DACR, ___rg);	\
	} while(0)

#define DLC_GCR_MIC1	0x18
#define DLC_GIM1_LSB	0
#define DLC_GIM1_MASK	0x7

#define __dlv_set_mic1_boost(n) dlv_update_reg(DLC_GCR_MIC1, (n), DLC_GIM1_MASK)

#define DLC_GCR_MIC2	0x19
#define DLC_GIM2_LSB	0
#define DLC_GIM2_MASK	0x7

#define __dlv_set_mic2_boost(n) dlv_update_reg(DLC_GCR_MIC2, (n), DLC_GIM2_MASK)

#define DLC_GCR_ADCL	0x1A
#define DLC_GCR_LRGID	(1 << 7)
#define DLC_GIDL_LSB	0
#define DLC_GIDL_MASK	0x3f

#define DLC_GCR_ADCR	0x1B
#define DLC_GIDR_LSB	0
#define DLC_GIDR_MASK	0x3f

#define __dlv_set_adc_gain(g)						\
	do {								\
		int ___g = ((g) & DLC_GODL_MASK) | DLC_GCR_RLGOD;	\
		dlv_write_reg(DLC_GCR_ADCL, ___g);			\
	} while(0)

#define __dlv_set_adc_gain_lr(lg, rg)			\
	do {						\
		int ___lg = (lg) & DLC_GODL_MASK;	\
		int ___rg = (rg) & DLC_GODR_MASK;	\
		dlv_write_reg(DLC_GCR_ADCL, ___lg);	\
		dlv_write_reg(DLC_GCR_ADCR, ___rg);	\
	} while(0)

#define DLC_GCR_MIXADC	0x1D
#define DLC_GIMIX_LSB	0
#define DLC_GIMIX_MASK	0x1f

#define DLC_GCR_MIXDAC	0x1E
#define DLC_GOMIX_LSB	0
#define DLC_GOMIX_MASK	0x1f

#define DLC_AGC1	0x1F
#define DLC_AGC_EN	(1 << 7)
#define DLC_AGC_STEREO	(1 << 6)
#define DLC_AGC_TARGET_LSB 2
#define DLC_AGC_TARGET_MASK	(0xf << 2)

#define __dlv_disable_agc() dlv_write_reg_bit(DLC_AGC1, 0, DLC_AGC_EN)
#define __dlv_enable_agc() dlv_write_reg_bit(DLC_AGC1, 1, DLC_AGC_EN)

#define DLC_AGC2	0x20
#define DLC_AGC_NG_EN	(1 << 7)
#define DLC_AGC_NG_THR_LSB	4
#define DLC_AGC_NG_THR_MASK	(0x7 << 4)

#define DLC_AGC_HOLD_LSB	0
#define DLC_AGC_HOLD_MASK	(0xf << 0)

#define DLC_AGC3	0x21
#define DLC_AGC_ATK_LSB	4
#define DLC_AGC_ATK_MASK	(0xf << 4)

#define DLC_AGC_DCY_LSB 0
#define DLC_AGC_DCY_MASK	(0xf << 0)

#define DLC_AGC4	0x22
#define DLC_AGC_MAX_LSB	0
#define DLC_AGC_MAX_MASK	(0x1f << 0)

#define DLC_AGC5	0x23
#define DLC_AGC_MIN_LSB	0
#define DLC_AGC_MIN_MASK	(0x1f << 0)



#if 0

/* Enable headphone detection */
//#define HP_SENSE_DETECT		1

/* Power setting */
#define POWER_ON		0
#define POWER_OFF		1

/* Have not been used. */
#if 0

/* CODEC mode
 *
 * unsigned int:		I--- RL21 O--- -LSH
 */
#define CODEC_REPLAY		0x00008000
 #define REPLAY_HP_OUT		1 << 0
 #define REPLAY_SPEAKER		1 << 1
 #define REPLAY_LINE_OUT	1 << 2

#define CODEC_RECORD		0x80000000
 #define RECORD_MIC1_IN		1 << 8
 #define RECORD_MIC2_IN		1 << 9
 #define RECORD_LINE_IN_L	1 << 10
 #define RECORD_LINE_IN_R	1 << 11

#endif

/* File ops mode W & R */
#define REPLAY			1
#define RECORD			2

/* Channels */
#define LEFT_CHANNEL		1
#define RIGHT_CHANNEL		2

#define MAX_RATE_COUNT		10


/*
 * JZ4760 DLV CODEC registers
 */

#define DLV_REG_SR		0

#define DLV_REG_AICR		1
 #define AICR_DAC_SERIAL	3
 #define AICR_ADC_SERIAL	2
 #define AICR_DAC_I2S		1
 #define AICR_ADC_I2S		0

#define DLV_REG_CR1		2
 #define CR1_LOAD		7
 #define CR1_HP_MUTE		5
 #define CR1_LINEOUT_MUTE	4
 #define CR1_BTL_MUTE		3
 #define CR1_OUTSEL1		1
 #define CR1_OUTSEL0		0
 #define CR1_OUTSEL_MASK	0x3

#define DLV_REG_CR2		3
 #define CR2_MONO		7
 #define CR2_DAC_MUTE		5
 #define CR2_NOMAD		1
 #define CR2_DAC_RIGHT_ONLY	0

#define DLV_REG_CR3		4
 #define CR3_INSEL1		3
 #define CR3_INSEL0		2
 #define CR3_MICSTEREO		1
 #define CR3_MICDIFF		0

#define DLV_REG_CR4		5
 #define CR4_ADC_HPF		7
 #define CR4_ADC_RIGHT_ONLY	0

#define DLV_REG_CCR1		6
 #define CCR1_CRYSTAL_BIT	0
 #define CCR1_CRYSTAL_MASK	0xf

#define DLV_REG_CCR2		7

#define DLV_REG_PMR1		8
 #define PMR1_SB		7
 #define PMR1_SB_SLEEP		6
 #define PMR1_SB_AIP		5
 #define PMR1_SB_LINE		4
 #define PMR1_SB_MIC1		3
 #define PMR1_SB_MIC2		2
 #define PMR1_SB_BYPASS		1
 #define PMR1_SB_MICBIAS	0

#define DLV_REG_PMR2		9
 #define PMR2_SB_ADC		4
 #define PMR2_SB_HP		3
 #define PMR2_SB_BTL		2
 #define PMR2_SB_LOUT		1
 #define PMR2_SB_DAC		0

#define DLV_REG_ICR		10
 #define ICR_INT_FORM_MASK	0xc0
 #define ICR_INT_FORM_BIT	6
 #define ICR_INT_HIGH		0x00
 #define ICR_INT_LOW		0x01
 #define ICR_INT_HIGH_8CYCLES	0x10
 #define ICR_JACK_MASK		(1 << 5)
 #define ICR_SCMC_MASK		(1 << 4)
 #define ICR_RUP_MASK		(1 << 3)
 #define ICR_RDO_MASK		(1 << 2)
 #define ICR_GUP_MASK		(1 << 1)
 #define ICR_GDO_MASK		(1 << 0)
 #define ICR_ALL_MASK							\
	(ICR_GDO_MASK | ICR_GUP_MASK | ICR_RDO_MASK | ICR_RUP_MASK | ICR_SCMC_MASK | ICR_JACK_MASK)

//#define ICR_COMMON_MASK	(ICR_GDO_MASK | ICR_GUP_MASK | ICR_RDO_MASK | ICR_RUP_MASK)
 #define ICR_COMMON_MASK	(ICR_GDO_MASK | ICR_GUP_MASK | ICR_RDO_MASK | ICR_RUP_MASK | ICR_JACK_MASK)

#define DLV_REG_IFR		11
 #define IFR_JACK		6
 #define IFR_JACKE		5
 #define IFR_SCMC		4
 #define IFR_RUP		3
 #define IFR_RDO		2
 #define IFR_GUP		1
 #define IFR_GDO		0

#define DLV_REG_GCR1		12
#define DLV_REG_GCR2		13
#define DLV_REG_GCR3		14
 #define GCR3_RLGI		7
 #define GCR3_GIR_BIT		0
 #define GCR3_GIR_MASK		0x1f

#define DLV_REG_GCR4		15
 #define GCR4_GIL_BIT		0
 #define GCR3_GIR_MASK		0x1f

#define DLV_REG_GCR5		16
#define DLV_REG_GCR6		17
#define DLV_REG_GCR7		18

#define DLV_REG_GCR8		19
 #define GCR8_GIDR_MASK		0x1f
 #define GCR8_GIDR_BIT		0

#define DLV_REG_GCR9		20
 #define GCR9_GIDL_MASK		0x1f
 #define GCR9_GIDL_BIT		0

#define DLV_REG_AGC1		21
 #define AGC1_AGCEN		7

#define DLV_REG_AGC2		22
#define DLV_REG_AGC3		23
#define DLV_REG_AGC4		24
#define DLV_REG_AGC5		25
#define DLV_REG_MIX1		26
 #define MIX1_REC_BIT		6
 #define MIX1_REC_MASK		0x3
 #define MIX1_GIMIX_BIT		0
 #define MIX1_GIMIX_MASK	0x1f

#define DLV_REG_MIX2		27

//#define HP_SENSE_DETECT

/*
 * Ops
 */

/* SB switch */

#define __dlv_get_sb_aip()		((dlv_read_reg(DLV_REG_PMR1) & (1 << PMR1_SB_AIP)) ? POWER_OFF : POWER_ON)

#define __dlv_switch_sb_aip(pwrstat)					\
do {									\
	if (__dlv_get_sb_aip() != pwrstat) {				\
		dlv_write_reg_bit(DLV_REG_PMR1, pwrstat, PMR1_SB_AIP);	\
	}								\
									\
} while (0)


#define __dlv_get_sb()			((dlv_read_reg(DLV_REG_PMR1) & (1 << PMR1_SB)) ? POWER_OFF : POWER_ON)

#define __dlv_switch_sb(pwrstat)					\
do {									\
	if (__dlv_get_sb() != pwrstat) {				\
		dlv_write_reg_bit(DLV_REG_PMR1, pwrstat, PMR1_SB);	\
	}								\
									\
} while (0)

#define __dlv_get_sb_sleep()		((dlv_read_reg(DLV_REG_PMR1) & (1 << PMR1_SB_SLEEP)) ? POWER_OFF : POWER_ON)

#define __dlv_switch_sb_sleep(pwrstat)					\
do {									\
	if (__dlv_get_sb_sleep() != pwrstat) {				\
		dlv_write_reg_bit(DLV_REG_PMR1, pwrstat, PMR1_SB_SLEEP);\
	}								\
									\
} while (0)

#define __dlv_get_sb_dac()		((dlv_read_reg(DLV_REG_PMR2) & (1 << PMR2_SB_DAC)) ? POWER_OFF : POWER_ON)

#define __dlv_switch_sb_dac(pwrstat)					\
do {									\
	if (__dlv_get_sb_dac() != pwrstat) {				\
		dlv_write_reg_bit(DLV_REG_PMR2, pwrstat, PMR2_SB_DAC);	\
	}								\
									\
} while (0)

#define __dlv_get_sb_line_out()		((dlv_read_reg(DLV_REG_PMR2) & (1 << PMR2_SB_LOUT)) ? POWER_OFF : POWER_ON)

#define __dlv_switch_sb_line_out(pwrstat)				\
do {									\
	if (__dlv_get_sb_line_out() != pwrstat) {			\
		dlv_write_reg_bit(DLV_REG_PMR2, pwrstat, PMR2_SB_LOUT);	\
	}								\
									\
} while (0)

#define __dlv_get_sb_hp()		((dlv_read_reg(DLV_REG_PMR2) & (1 << PMR2_SB_HP)) ? POWER_OFF : POWER_ON)

#define __dlv_switch_sb_hp(pwrstat)					\
do {									\
	if (__dlv_get_sb_hp() != pwrstat) {				\
		dlv_write_reg_bit(DLV_REG_PMR2, pwrstat, PMR2_SB_HP);	\
	}								\
									\
} while (0)

#define __dlv_get_sb_btl()		((dlv_read_reg(DLV_REG_PMR2) & (1 << PMR2_SB_BTL)) ? POWER_OFF : POWER_ON)

#define __dlv_switch_sb_btl(pwrstat)					\
do {									\
	if (__dlv_get_sb_btl() != pwrstat) {				\
		dlv_write_reg_bit(DLV_REG_PMR2, pwrstat, PMR2_SB_BTL);	\
	}								\
									\
} while (0)

#define __dlv_switch_sb_out(pwrstat)					\
do {									\
	/*__dlv_switch_sb_btl(pwrstat);*/				\
	__dlv_switch_sb_hp(pwrstat);					\
	/*__dlv_switch_sb_line_out(pwrstat);*/				\
} while (0)

#define __dlv_get_sb_adc()		((dlv_read_reg(DLV_REG_PMR2) & (1 << PMR2_SB_ADC)) ? POWER_OFF : POWER_ON)

#define __dlv_switch_sb_adc(pwrstat)					\
do {									\
	if (__dlv_get_sb_adc() != pwrstat) {				\
		dlv_write_reg_bit(DLV_REG_PMR2, pwrstat, PMR2_SB_ADC);	\
	}								\
									\
} while (0)

#define __dlv_get_sb_mic1()		((dlv_read_reg(DLV_REG_PMR1) & (1 << PMR1_SB_MIC1)) ? POWER_OFF : POWER_ON)

#define __dlv_switch_sb_mic1(pwrstat)					\
do {									\
	if (__dlv_get_sb_mic1() != pwrstat) {				\
		dlv_write_reg_bit(DLV_REG_PMR1, pwrstat, PMR1_SB_MIC1);	\
	}								\
									\
} while (0)

#define __dlv_get_sb_mic2()		((dlv_read_reg(DLV_REG_PMR1) & (1 << PMR1_SB_MIC2)) ? POWER_OFF : POWER_ON)

#define __dlv_switch_sb_mic2(pwrstat)					\
do {									\
	if (__dlv_get_sb_mic2() != pwrstat) {				\
		dlv_write_reg_bit(DLV_REG_PMR1, pwrstat, PMR1_SB_MIC2);	\
	}								\
									\
} while (0)

#define __dlv_get_sb_micbias()		((dlv_read_reg(DLV_REG_PMR1) & (1 << PMR1_SB_MICBIAS)) ? POWER_OFF : POWER_ON)

#define __dlv_switch_sb_micbias(pwrstat)				\
do {									\
	if (__dlv_get_sb_micbias() != pwrstat) {			\
		dlv_write_reg_bit(DLV_REG_PMR1, pwrstat, PMR1_SB_MICBIAS);\
	}								\
									\
} while (0)

#define __dlv_get_sb_line_in()		((dlv_read_reg(DLV_REG_PMR1) & (1 << PMR1_SB_LINE)) ? POWER_OFF : POWER_ON)

#define __dlv_switch_sb_line_in(pwrstat)				\
do {									\
	if (__dlv_get_sb_line_in() != pwrstat) {			\
		dlv_write_reg_bit(DLV_REG_PMR1, pwrstat, PMR1_SB_LINE);	\
	}								\
									\
} while (0)

#define __dlv_get_sb_bypass()		((dlv_read_reg(DLV_REG_PMR1) & (1 << PMR1_SB_BYPASS)) ? POWER_OFF : POWER_ON)

#define __dlv_switch_sb_bypass(pwrstat)					\
do {									\
	if (__dlv_get_sb_bypass() != pwrstat) {				\
		dlv_write_reg_bit(DLV_REG_PMR1, pwrstat, PMR1_SB_BYPASS);\
	}								\
									\
} while (0)

#define __dlv_set_int_form(v)						\
do {									\
	dlv_write_reg_bit(DLV_REG_ICR, ((v) >> 1) & 1, 7);		\
	dlv_write_reg_bit(DLV_REG_ICR, (v) & 1, 6);			\
} while (0)

#define __dlv_set_irq_mask(mask)					\
do {									\
	/*unsigned int int_form = dlv_read_reg(DLV_REG_ICR) & 0xc0;*/	\
	/*dlv_write_reg(DLV_REG_ICR, ((mask) & 0x3f) | int_form);*/	\
	dlv_write_reg(DLV_REG_ICR, mask);				\
									\
} while (0)

#define __dlv_set_irq_flag(flag)					\
do {									\
	dlv_write_reg(DLV_REG_IFR, flag);				\
									\
} while (0)

#define __dlv_get_irq_flag()		(dlv_read_reg(DLV_REG_IFR))
#define __dlv_get_irq_mask()		(dlv_read_reg(DLV_REG_ICR) & 0x3f)

#define __dlv_set_16ohm_load()						\
do {									\
	dlv_write_reg_bit(DLV_REG_CR1, 0, CR1_LOAD);			\
									\
} while (0)

#define __dlv_set_10kohm_load()						\
do {									\
	dlv_write_reg_bit(DLV_REG_CR1, 1, CR1_LOAD);			\
									\
} while (0)

#define __dlv_enable_hp_mute()						\
do {									\
	dlv_write_reg_bit(DLV_REG_CR1, 1, CR1_HP_MUTE);			\
									\
} while (0)

#define __dlv_disable_hp_mute()						\
do {									\
	dlv_write_reg_bit(DLV_REG_CR1, 0, CR1_HP_MUTE);			\
									\
} while (0)

#define __dlv_enable_lineout_mute()					\
do {									\
	dlv_write_reg_bit(DLV_REG_CR1, 1, CR1_LINEOUT_MUTE);		\
									\
} while (0)

#define __dlv_disable_lineout_mute()					\
do {									\
	dlv_write_reg_bit(DLV_REG_CR1, 0, CR1_LINEOUT_MUTE);		\
									\
} while (0)

#define __dlv_enable_btl_mute()						\
do {									\
	dlv_write_reg_bit(DLV_REG_CR1, 1, CR1_BTL_MUTE);		\
									\
} while (0)

#define __dlv_disable_btl_mute()					\
do {									\
	dlv_write_reg_bit(DLV_REG_CR1, 0, CR1_BTL_MUTE);		\
									\
} while (0)

#define MIC1_TO_LR		0 //00b
#define MIC2_TO_LR		1 //01b
#define MIC1_TO_R_MIC2_TO_L	0 //00b
#define MIC2_TO_R_MIC1_TO_L	1 //01b
#define BYPASS_PATH		2 //10b
#define DAC_OUTPUT		3 //11b

#define __dlv_set_outsel(opt)						\
do {									\
	dlv_write_reg_bit(DLV_REG_CR1, (opt) & 0x1, CR1_OUTSEL0);	\
	dlv_write_reg_bit(DLV_REG_CR1, (opt >> 1) & 0x1, CR1_OUTSEL1);	\
} while (0)

#define __dlv_enable_dac_m2s()						\
do {									\
	dlv_write_reg_bit(DLV_REG_CR2, 1, CR2_MONO);			\
									\
} while (0)

#define __dlv_disable_dac_m2s()						\
do {									\
	dlv_write_reg_bit(DLV_REG_CR2, 0, CR2_MONO);			\
									\
} while (0)

#define __dlv_get_dac_mute()        ((dlv_read_reg(DLV_REG_CR2) & (1 << CR2_DAC_MUTE)) ? 1 : 0)

#define __dlv_enable_dac_mute()						\
do {									\
	dlv_write_reg_bit(DLV_REG_CR2, 1, CR2_DAC_MUTE);		\
									\
} while (0)

#define __dlv_disable_dac_mute()					\
do {									\
	dlv_write_reg_bit(DLV_REG_CR2, 0, CR2_DAC_MUTE);		\
									\
} while (0)

#define __dlv_enable_nomad()						\
do {									\
	dlv_write_reg_bit(DLV_REG_CR2, 1, CR2_NOMAD);			\
									\
} while (0)

#define __dlv_disable_nomad()						\
do {									\
	dlv_write_reg_bit(DLV_REG_CR2, 0, CR2_NOMAD);			\
									\
} while (0)


#define __dlv_enable_dac_right_only()					\
do {									\
	dlv_write_reg_bit(DLV_REG_CR2, 1, CR2_DAC_RIGHT_ONLY);		\
									\
} while (0)

#define __dlv_disable_dac_right_only()					\
do {									\
	dlv_write_reg_bit(DLV_REG_CR2, 0, CR2_DAC_RIGHT_ONLY);		\
									\
} while (0)

/* Following macros are the arguments for __dlv_set_insel.
 * And they have already defined.
 *
#define MIC1_TO_LR		0 //00b
#define MIC2_TO_LR		1 //01b
#define MIC1_TO_R_MIC2_TO_L	0 //00b
#define MIC2_TO_R_MIC1_TO_L	1 //01b
 *
 */
#define LINE_INPUT		2 //10b

#define __dlv_set_insel(opt)						\
do {									\
	dlv_write_reg_bit(DLV_REG_CR3, (opt) & 1, CR3_INSEL0);		\
	dlv_write_reg_bit(DLV_REG_CR3, (opt >> 1) & 1, CR3_INSEL1);	\
} while (0)

#define __dlv_set_mic_stereo()						\
do {									\
	dlv_write_reg_bit(DLV_REG_CR3, 1, CR3_MICSTEREO);		\
									\
} while (0)

#define __dlv_set_mic_mono()						\
do {									\
	dlv_write_reg_bit(DLV_REG_CR3, 0, CR3_MICSTEREO);		\
									\
} while (0)

#define __dlv_enable_mic_diff()					\
do {								\
	dlv_write_reg_bit(DLV_REG_CR3, 1, CR3_MICDIFF);		\
								\
} while (0)

#define __dlv_disable_mic_diff()				\
do {								\
	dlv_write_reg_bit(DLV_REG_CR3, 0, CR3_MICDIFF);		\
								\
} while (0)

#define __dlv_enable_adc_high_pass()				\
do {								\
	dlv_write_reg_bit(DLV_REG_CR4, 1, CR4_ADC_HPF);		\
								\
} while (0)

#define __dlv_disable_adc_high_pass()				\
do {								\
	dlv_write_reg_bit(DLV_REG_CR4, 0, CR4_ADC_HPF);		\
								\
} while (0)

#define __dlv_enable_adc_right_only()				\
do {								\
	dlv_write_reg_bit(DLV_REG_CR4, 1, CR4_ADC_RIGHT_ONLY);	\
								\
} while (0)

#define __dlv_disable_adc_right_only()				\
do {								\
	dlv_write_reg_bit(DLV_REG_CR4, 0, CR4_ADC_RIGHT_ONLY);	\
								\
} while (0)

#define __dlv_set_hp_volume(vol)				\
do {								\
	dlv_write_reg(DLV_REG_GCR1, vol |= 0x80);		\
								\
} while (0)

#define __dlv_set_line_in_bypass_volume(vol)			\
do {								\
	vol = (vol & GCR3_GIR_MASK) | (1 << GCR3_RLGI);		\
	dlv_write_reg(DLV_REG_GCR3, vol);			\
								\
} while (0)

#define __dlv_set_line_in_bypass_volume_rl(lvol, rvol)		\
do {								\
	rvol = rvol & GCR3_GIR_MASK;				\
	dlv_write_reg(DLV_REG_GCR3, rvol);			\
	dlv_write_reg(DLV_REG_GCR4, lvol);			\
								\
} while (0)

#define __dlv_set_mic_1_volume(vol)				\
do {								\
	dlv_write_reg(DLV_REG_GCR8, vol | 0x80);		\
								\
} while (0)

#define __dlv_set_gim(value)					\
do {								\
	dlv_write_reg(DLV_REG_GCR7, (value));			\
								\
} while (0)

#define __dlv_set_gidr(value)					\
do {								\
	unsigned int r = dlv_read_reg(DLV_REG_GCR8);		\
	r = (r & ~GCR8_GIDR_MASK) | ((value) << GCR8_GIDR_BIT);	\
	dlv_write_reg(DLV_REG_GCR8, r << GCR8_GIDR_BIT);	\
								\
} while (0)

#define __dlv_set_gidl(value)					\
do {								\
	unsigned int r = dlv_read_reg(DLV_REG_GCR9);		\
	r = (r & ~GCR9_GIDL_MASK) | ((value) << GCR9_GIDL_BIT);	\
	dlv_write_reg(DLV_REG_GCR9, r << GCR9_GIDL_BIT);	\
								\
} while (0)

#define __dlv_set_godr(value)					\
do {								\
	dlv_write_reg(DLV_REG_GCR5, value);	                \
	                                                        \
} while (0)

#define __dlv_set_godl(value)					\
do {								\
	dlv_write_reg(DLV_REG_GCR6, value);			\
								\
} while (0)

#define __dlv_enable_agc()					\
do {								\
	dlv_write_reg_bit(DLV_REG_AGC1, 1, AGC1_AGCEN);		\
								\
} while (0)

#define __dlv_disable_agc()					\
do {								\
	dlv_write_reg_bit(DLV_REG_AGC1, 0, AGC1_AGCEN);		\
								\
} while (0)

#define __dlv_set_12m_crystal()					\
do {								\
	dlv_write_reg_bit(DLV_REG_CCR1, 0, 1);			\
	dlv_write_reg_bit(DLV_REG_CCR1, 0, 0);			\
								\
} while (0)

#define __dlv_set_mix_rec_2_dac()				\
do {								\
	dlv_write_reg_bit(DLV_REG_MIX1, 1, 6);			\
	dlv_write_reg_bit(DLV_REG_CCR1, 0, 7);			\
								\
} while (0)

#define __dlv_set_mix_rec_only()				\
do {								\
	dlv_write_reg_bit(DLV_REG_MIX1, 0, 6);			\
	dlv_write_reg_bit(DLV_REG_CCR1, 0, 7);			\
								\
} while (0)

#define __dlv_reset_rup()                                       \
do {                                                            \
	dlv_write_reg_bit(DLV_REG_IFR, 1, IFR_RUP);             \
	                                                        \
} while (0)

#define __dlv_reset_gup()                                       \
do {                                                            \
	dlv_write_reg_bit(DLV_REG_IFR, 1, IFR_GUP);             \
                                                                \
} while (0)

#define __dlv_reset_gdo()                                       \
do {                                                            \
	dlv_write_reg_bit(DLV_REG_IFR, 1, IFR_GDO);             \
                                                                \
} while (0)

#define __dlv_reset_rdo()                                       \
do {                                                            \
	dlv_write_reg_bit(DLV_REG_IFR, 1, IFR_RDO);             \
                                                                \
} while (0)

#ifdef CONFIG_HP_SENSE_DETECT
/*
 * HP_SENSE switch
 */
typedef struct {
	struct switch_dev	sdev;
	const char		*name;
	const char		*name_on;
	const char		*name_off;
	const char		*state_on;
	const char		*state_off;

} jz_hp_switch_data_t ;

typedef struct {
	const char		*name;
	const char		*name_on;
	const char		*name_off;
	const char		*state_on;
	const char		*state_off;

} jz_hp_switch_platform_data_t ;

#endif
#endif

#endif // __JZ4770_DLV_H__
