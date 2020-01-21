/*
 * lepus.c  --  SoC audio for LEPUS
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <asm/jzsoc.h>

#include "../codecs/jz4760_icdc.h"
#include "jz4760-pcm.h"
#include "jz4760-i2s.h"

#if 0
/* currently we have nothing to do when startup & powerdown */
static int lepus_startup(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec *codec = rtd->socdev->card->codec;

	return 0;
}

static void lepus_shutdown(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec *codec = rtd->socdev->card->codec;

	return;
}
#endif

static int lepus_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	int ret = 0;

	/* set codec DAI configuration */
#if 0
	ret = snd_soc_dai_set_fmt(codec_dai,
				  SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0)
		return ret;
#endif

	/* set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai,
				  SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0)
		return ret;

	/* set the codec system clock for DAC and ADC */
	ret = snd_soc_dai_set_sysclk(codec_dai, 0, JZ4760_I2S_SYSCLK,
				     SND_SOC_CLOCK_IN);
	if (ret < 0)
		return ret;
	/* we do not have to set clkdiv for internal codec */

#if 0
	/* set codec DAI configuration */
#if 0
	ret = codec_dai->ops->set_fmt(codec_dai, SND_SOC_DAIFMT_I2S |
		SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0)
		return ret;
#endif

	/* set cpu DAI configuration */
	ret = cpu_dai->ops->set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S |
		SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0)
		return ret;

	/* set the codec system clock for DAC and ADC */
	ret = codec_dai->ops->set_sysclk(codec_dai, 0, 111,
		SND_SOC_CLOCK_IN);
	if (ret < 0)
		return ret;

	/* set the I2S system clock as input (unused) */
	ret = cpu_dai->ops->set_sysclk(cpu_dai, JZ4760_I2S_SYSCLK, 0,
		SND_SOC_CLOCK_IN);
	if (ret < 0)
		return ret;
#endif

	return 0;
}

static struct snd_soc_ops lepus_ops = {
	//.startup = lepus_startup,
	.hw_params = lepus_hw_params,
	//.shutdown = lepus_shutdown,
};

/* define the scenarios */
#define LEPUS_AUDIO_OFF			0
#define LEPUS_STEREO_TO_HEADPONES	1
#define LEPUS_STEREO_TO_SPEAKERS	2
#define LEPUS_CAPTURE_MIC1		3
#define LEPUS_CAPTURE_MIC2		4

static const char *lepus_scenarios[] = {
	"Off",
	"Headphone",
	"Speakers",
	"Mic1",
	"Mic2",
};

static const struct soc_enum lepus_scenario_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(lepus_scenarios), lepus_scenarios),
};

static int lepus_scenario;

static int lepus_get_scenario(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = lepus_scenario;
	return 0;
}

static int set_scenario_endpoints(struct snd_soc_codec *codec, int scenario)
{
	switch (lepus_scenario) {
	case LEPUS_AUDIO_OFF:
		snd_soc_dapm_disable_pin(codec, "Headphone Jack");
		snd_soc_dapm_disable_pin(codec, "Mic1 Jack");
		snd_soc_dapm_disable_pin(codec, "Mic2 Jack");
		snd_soc_dapm_disable_pin(codec, "Ext Spk");
		break;
	case LEPUS_STEREO_TO_HEADPONES:
		snd_soc_dapm_enable_pin(codec, "Headphone Jack");
		snd_soc_dapm_disable_pin(codec, "Mic1 Jack");
		snd_soc_dapm_disable_pin(codec, "Mic2 Jack");
		snd_soc_dapm_disable_pin(codec, "Ext Spk");
		break;
	case  LEPUS_STEREO_TO_SPEAKERS:
		snd_soc_dapm_disable_pin(codec, "Headphone Jack");
		snd_soc_dapm_disable_pin(codec, "Mic1 Jack");
		snd_soc_dapm_disable_pin(codec, "Mic2 Jack");
		snd_soc_dapm_enable_pin(codec, "Ext Spk");
		break;
	case LEPUS_CAPTURE_MIC1:
		snd_soc_dapm_disable_pin(codec, "Headphone Jack");
		snd_soc_dapm_enable_pin(codec, "Mic1 Jack");
		snd_soc_dapm_disable_pin(codec, "Mic2 Jack");
		snd_soc_dapm_disable_pin(codec, "Ext Spk");
		break;
	case LEPUS_CAPTURE_MIC2:
		snd_soc_dapm_disable_pin(codec, "Headphone Jack");
		snd_soc_dapm_disable_pin(codec, "Mic1 Jack");
		snd_soc_dapm_enable_pin(codec, "Mic2 Jack");
		snd_soc_dapm_disable_pin(codec, "Ext Spk");
		break;

	default:
		snd_soc_dapm_disable_pin(codec, "Headphone Jack");
		snd_soc_dapm_disable_pin(codec, "Mic1 Jack");
		snd_soc_dapm_disable_pin(codec, "Mic2 Jack");
		snd_soc_dapm_disable_pin(codec, "Ext Spk");
		break;
	}

	snd_soc_dapm_sync(codec);

	return 0;
}

static int lepus_set_scenario(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);

	if (lepus_scenario == ucontrol->value.integer.value[0])
		return 0;

	lepus_scenario = ucontrol->value.integer.value[0];
	set_scenario_endpoints(codec, lepus_scenario);
	return 1;
}

/* lepus machine dapm widgets */
static const struct snd_soc_dapm_widget jz_icdc_dapm_widgets[] = {
	SND_SOC_DAPM_HP("Headphone Jack", NULL),
	SND_SOC_DAPM_MIC("Mic1 Jack", NULL),
	SND_SOC_DAPM_MIC("Mic2 Jack", NULL),
	SND_SOC_DAPM_SPK("Ext Spk", NULL),
};

/* lepus machine audio map (connections to the codec pins) */
static const struct snd_soc_dapm_route audio_map [] = {
	/* Destination Widget  <=== Path Name <=== Source Widget */

	/* headphone connected to LHPOUT/RHPOUT */
	{"Headphone Jack", NULL, "LHPOUT"},
	{"Headphone Jack", NULL, "RHPOUT"},

	/* speaker connected to BTLP/BTLN */
	{"Ext Spk", NULL, "BTLP"},
	{"Ext Spk", NULL, "BTLN"},

	/* mic is connected to MICIN (via right channel of headphone jack) */
	{"MIC1P", NULL, "Mic Bias"},
	{"MIC1N", NULL, "Mic Bias"}, /* no such connection, but not harm */
	{"Mic Bias", NULL, "Mic1 Jack"},

	{"MIC2P", NULL, "Mic Bias"},
	{"MIC2N", NULL, "Mic Bias"},
	{"Mic Bias", NULL, "Mic2 Jack"},
};

/* If you have any amplifiers, you can add its controls here */
static const struct snd_kcontrol_new jz_icdc_lepus_controls[] = {
	SOC_ENUM_EXT("Lepus Mode", lepus_scenario_enum[0],
		     lepus_get_scenario, lepus_set_scenario),
};

/*
 * Lepus for a jz_icdc as connected on jz4760 Device
 */
static int lepus_jz_icdc_init(struct snd_soc_codec *codec)
{
	int err;

	/* set up codec pins not used */
	/* on Lepus board, linein pin are not connected */
	snd_soc_dapm_nc_pin(codec, "LLINEIN");
	snd_soc_dapm_nc_pin(codec, "RLINEIN");

	/* Add lepus specific controls */
	err = snd_soc_add_controls(codec, jz_icdc_lepus_controls,
				   ARRAY_SIZE(jz_icdc_lepus_controls));
	if (err < 0)
		return err;

	/* Add lepus specific widgets */
	snd_soc_dapm_new_controls(codec, jz_icdc_dapm_widgets, ARRAY_SIZE(jz_icdc_dapm_widgets));

	snd_soc_dapm_add_routes(codec, audio_map, ARRAY_SIZE(audio_map));

	snd_soc_dapm_sync(codec);

	/* set endpoints to default mode */
	set_scenario_endpoints(codec, LEPUS_AUDIO_OFF);

	return 0;
}

/* lepus digital audio interface glue - connects codec <--> CPU */
static struct snd_soc_dai_link lepus_dai = {
	.name = "JZ_ICDC",
	.stream_name = "JZ_ICDC",
	.cpu_dai = &jz4760_i2s_dai,
	.codec_dai = &jz_icdc_dai, /* defined in sound/soc/codecs/jz4760-icdc.c */
	.init = lepus_jz_icdc_init,
	.ops = &lepus_ops,
};

/* lepus audio machine driver */
static struct snd_soc_card snd_soc_card_lepus = {
	.name = "Lepus",      /* used by state file, use aplay -l or arecord -l will see this name */
	.dai_link = &lepus_dai,
	.platform = &jz4760_soc_platform,/* defined in sound/soc/jz4760/jz4760-pcm.c */
	.num_links = 1,	      /* ARRAY_SIZE(lepus_dai), though lepus_dai is not an array *n* */
};

/* lepus audio subsystem */
static struct snd_soc_device lepus_snd_devdata = {
	.card = &snd_soc_card_lepus,
	.codec_dev = &jz_icdc_soc_codec_dev, /* defined in sound/soc/codecs/jz4760-icdc.c */
	//.codec_data = your private data here,
};

static struct platform_device *lepus_snd_device;

static int __init lepus_init(void)
{
	int ret;

	lepus_snd_device = platform_device_alloc("soc-audio", -1);

	if (!lepus_snd_device)
		return -ENOMEM;

	platform_set_drvdata(lepus_snd_device, &lepus_snd_devdata);
	lepus_snd_devdata.dev = &lepus_snd_device->dev;
	ret = platform_device_add(lepus_snd_device);

	if (ret)
		platform_device_put(lepus_snd_device);

	return ret;
}

static void __exit lepus_exit(void)
{
	platform_device_unregister(lepus_snd_device);
}

module_init(lepus_init);
module_exit(lepus_exit);

/* Module information */
MODULE_AUTHOR("Lutts Wolf <slcao@ingenic.cn>");
MODULE_DESCRIPTION("ALSA SoC Internel Codec Lepus");
MODULE_LICENSE("GPL");
