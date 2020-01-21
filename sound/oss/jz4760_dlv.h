/*
 * Linux/sound/oss/jz4760_dlv.h
 *
 * Copyright (c) Ingenic Semiconductor Co., Ltd.
 */

#ifndef __JZ4760_DLV_H__
#define __JZ4760_DLV_H__

#define MAX_RATE_COUNT	10

/* JZ4760 DLV CODEC registers number */
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

#define DLV_REG_CCR1		6

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
 #define ICR_JACK_MASK		5
 #define ICR_SCMC_MASK		4
 #define ICR_RUP_MASK		3
 #define ICR_RDO_MASK		2
 #define ICR_GUP_MASK		1
 #define ICR_GDO_MASK		0

#define DLV_REG_IFR		11
 #define IFR_JACK		6
 #define IFR_JACK_EVENT		5
 #define IFR_SCMC		4
 #define IFR_RUP		3
 #define IFR_RDO		2
 #define IFR_GUP		1
 #define IFR_GDO		0

#define DLV_REG_GCR1		12
#define DLV_REG_GCR2		13
#define DLV_REG_GCR3		14
#define DLV_REG_GCR4		15
#define DLV_REG_GCR5		16
#define DLV_REG_GCR6		17
#define DLV_REG_GCR7		18

#define DLV_REG_GCR8		19
#define DLV_REG_GCR9		20

#define DLV_REG_AGC1		21
 #define AGC1_AGCEN		7

#define DLV_REG_AGC2		22
#define DLV_REG_AGC3		23
#define DLV_REG_AGC4		24
#define DLV_REG_AGC5		25
#define DLV_REG_MIX1		26
#define DLV_REG_MIX2		27

/*
void write_codec_file(int addr, int val);
int read_codec_file(int addr);
void printk_codec_files(int aaa);
int write_codec_file_bit(int addr, int bitval, int mask_bit);
void set_audio_data_replay(void);
void unset_audio_data_replay(void);
void set_record_mic_input_audio_without_playback(void);
void unset_record_mic_input_audio_without_playback(void);
void set_record_line_input_audio_without_playback(void);
void unset_record_line_input_audio_without_playback(void);
void set_playback_line_input_audio_direct_only(void);
void unset_playback_line_input_audio_direct_only(void);
void set_record_mic_input_audio_with_direct_playback(void);
void unset_record_mic_input_audio_with_direct_playback(void);
void set_record_playing_audio_mixed_with_mic_input_audio(void);
void unset_record_playing_audio_mixed_with_mic_input_audio(void);
void set_record_mic_input_audio_with_audio_data_replay(void);
void unset_record_mic_input_audio_with_audio_data_replay(void);
void set_record_line_input_audio_with_audio_data_replay(void);
void unset_record_line_input_audio_with_audio_data_replay(void);
*/

#endif // __JZ4760_DLV_H__
