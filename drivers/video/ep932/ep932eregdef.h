#ifndef EP932EREGDEF_H
#define EP932EREGDEF_H

#define EP932E_VENDOR_ID						0x0000
#define EP932E_DEVICE_ID						0x0002
#define EP932E_FIRMWARE_REVISION__MAJOR					0x0004
#define EP932E_FIRMWARE_REVISION__MINOR					0x0005

#define EP932E_INTERRUPT_FLAGS						0x0100
#define EP932E_INTERRUPT_FLAGS__EDID_CHG				0x80
#define EP932E_INTERRUPT_FLAGS__VIDEO_CHG				0x40
#define EP932E_INTERRUPT_FLAGS__AUDIO_CHG				0x20
#define EP932E_INTERRUPT_FLAGS__VS_ALIGN_FAILED				0x10
#define EP932E_INTERRUPT_FLAGS__VS_ALIGN_DONE				0x08

#define EP932E_SYSTEM_STATUS						0x0200
#define EP932E_SYSTEM_STATUS__RSEN					0x80
#define EP932E_SYSTEM_STATUS__HTPLG					0x40
#define EP932E_SYSTEM_STATUS__KEY_FAIL					0x02
#define EP932E_SYSTEM_STATUS__DEF_KEY					0x01

#define EP932E_HDCP_STATUS						0x0300
#define EP932E_HDCP_STATUS__BKSV					0x80
#define EP932E_HDCP_STATUS__AKSV					0x40
#define EP932E_HDCP_STATUS__R0						0x20
#define EP932E_HDCP_STATUS__RI						0x10
#define EP932E_HDCP_STATUS__REPEATERRDY					0x08
#define EP932E_HDCP_STATUS__REPEATERSHA					0x04
#define EP932E_HDCP_STATUS__RSEN					0x02
#define EP932E_HDCP_STATUS__REVOKE					0x01

#define EP932E_HDCP_STATE						0x0301

#define EP932E_HDCP_AKSV						0x0302
 
#define EP932E_HDCP_BKSV						0x0307 

#define EP932E_HDCP_BCAPS						0x030C

#define EP932E_HDCP_BSTATUS						0x030D

#define EP932E_HDCP_KSV_FIFO						0x030F

#define EP932E_EDID_STATUS						0x0400
#define EP932E_EDID_STATUS__HDMI					0x10
#define EP932E_EDID_STATUS__DDC_STATUS					0x0F

typedef enum {
	EDID_DDC_SUCCESS = 0x00,
	EDID_DDC_PENDING,
	EDID_DDC_NOACT = 0x02,
	EDID_DDC_TIMEOUT,
	EDID_DDC_ARBITRATIONLOSS = 0x04,
	EDID_DDC_BLOCKNUMBER
} edid_ddc_status;

#define EP932E_EDID_STATUS_ASFREQ			0x0401
#define EP932E_EDID_STATUS_ACHANNEL			0x0402

#define EP932E_VIDEO_STATUS_VS_PERIOD			0x0500					// 2 Byte
#define EP932E_VIDEO_STATUS_H_RES			0x0502					// 2 Byte
#define EP932E_VIDEO_STATUS_V_RES			0x0504					// 2 Byte
#define EP932E_VIDEO_STATUS_RATIO_24			0x0506					// 2 Byte
#define EP932E_VIDEO_STATUS_PARAMS			0x0508					// 8 Byte

#define EP932E_AUDIO_STATUS_AS_FREQ			0x0600					// 2 Byte
#define EP932E_AUDIO_STATUS_AS_PERIOD			0x0602					// 2 Byte
#define EP932E_AUDIO_STATUS_PARAMS			0x0604					// 7 Byte

#define EP932E_EDID_DATA				0x0700					// 256 Byte

#define EP932E_ANALOG_TEST_CONTROL				0x1C00
#define EP932E_ANALOG_TEST_CONTROL__PREEMPHASIS			0x03

#define EP932E_SIP_TEST_CONTROL					0x1D00
#define EP932E_SIP_TEST_CONTROL__VS_ALIGN			0x08
#define EP932E_SIP_TEST_CONTROL__BIST				0x04
#define EP932E_SIP_TEST_CONTROL__ANA_TEST			0x02
#define EP932E_SIP_TEST_CONTROL__IIC_STOP 			0x01

#define EP932E_POWER_CONTROL					0x2000
#define EP932E_POWER_CONTROL__PD_HDMI				0x02
#define EP932E_POWER_CONTROL__PD_TOT				0x01

#define EP932E_SYSTEM_CONFIGURATION				0x2001
#define EP932E_SYSTEM_CONFIGURATION__PACKET_RDY			0x80
#define EP932E_SYSTEM_CONFIGURATION__HDCP_DIS			0x20
#define EP932E_SYSTEM_CONFIGURATION__HDMI_DIS			0x10
#define EP932E_SYSTEM_CONFIGURATION__FORCE_HDMI_CAP		0x08
#define EP932E_SYSTEM_CONFIGURATION__AUDIO_DIS			0x02
#define EP932E_SYSTEM_CONFIGURATION__VIDEO_DIS			0x01

#define EP932E_INTERRUPT_ENABLE					0x2100
#define EP932E_INTERRUPT_ENABLE__EDID_CHG			0x80
#define EP932E_INTERRUPT_ENABLE__VIDEO_CHG			0x40
#define EP932E_INTERRUPT_ENABLE__AUDIO_CHG			0x20
#define EP932E_INTERRUPT_ENABLE__VS_ALIGN_DONE			0x08

#define EP932E_VIDEO_INTERFACE_SETTING_0				0x2200
#define EP932E_VIDEO_INTERFACE_SETTING_0__DK				0xE0
#define EP932E_VIDEO_INTERFACE_SETTING_0__DKEN				0x10
#define EP932E_VIDEO_INTERFACE_SETTING_0__DSEL				0x08
#define EP932E_VIDEO_INTERFACE_SETTING_0__BSEL				0x04
#define EP932E_VIDEO_INTERFACE_SETTING_0__EDGE				0x02
#define EP932E_VIDEO_INTERFACE_SETTING_0__FMT12				0x01

#define EP932E_VIDEO_INTERFACE_SETTING_1				0x2201
#define EP932E_VIDEO_INTERFACE_SETTING_1__COLOR				0x30
#define EP932E_VIDEO_INTERFACE_SETTING_1__COLOR__AUTO			0x00
#define EP932E_VIDEO_INTERFACE_SETTING_1__COLOR__601			0x10
#define EP932E_VIDEO_INTERFACE_SETTING_1__COLOR__709			0x20
#define EP932E_VIDEO_INTERFACE_SETTING_1__XVYCC_EN			0x80
#define EP932E_VIDEO_INTERFACE_SETTING_1__SYNC				0x0C
#define EP932E_VIDEO_INTERFACE_SETTING_1__SYNC__HVDE			0x00
#define EP932E_VIDEO_INTERFACE_SETTING_1__SYNC__HV			0x04
#define EP932E_VIDEO_INTERFACE_SETTING_1__SYNC__Embeded			0x08
#define EP932E_VIDEO_INTERFACE_SETTING_1__VIN_FMT			0x03
#define EP932E_VIDEO_INTERFACE_SETTING_1__VIN_FMT__RGB			0x00
#define EP932E_VIDEO_INTERFACE_SETTING_1__VIN_FMT__YCC444		0x01
#define EP932E_VIDEO_INTERFACE_SETTING_1__VIN_FMT__YCC422		0x02 

#define EP932E_AUDIO_INTERFACE_SETTING					0x2300
#define EP932E_AUDIO_INTERFACE_SETTING__CHANNEL				0xF0
#define EP932E_AUDIO_INTERFACE_SETTING__IIS				0x08
#define EP932E_AUDIO_INTERFACE_SETTING__WS_M				0x04
#define EP932E_AUDIO_INTERFACE_SETTING__WS_POL				0x02
#define EP932E_AUDIO_INTERFACE_SETTING__SCK_POL				0x01

#define EP932E_VIDEO_INPUT_FORMAT_VIC					0x2400
#define EP932E_VIDEO_INPUT_FORMAT_1					0x2401
#define EP932E_VIDEO_INPUT_FORMAT_1__AFAR				0x30
#define EP932E_VIDEO_INPUT_FORMAT_1__AFAR__AUTO				0x00
#define EP932E_VIDEO_INPUT_FORMAT_1__AFAR__4_3				0x10
#define EP932E_VIDEO_INPUT_FORMAT_1__AFAR__16_9				0x20
#define EP932E_VIDEO_INPUT_FORMAT_1__AFAR__14_9				0x30
#define EP932E_VIDEO_INPUT_FORMAT_1__VIF				0x01

#define EP932E_AUDIO_INPUT_FORMAT					0x2500
#define EP932E_AUDIO_INPUT_FORMAT__NOCOPYRIGHT				0x10
#define EP932E_AUDIO_INPUT_FORMAT__ADO_FREQ				0x07
#define EP932E_AUDIO_INPUT_FORMAT__ADO_FREQ__AUTO			0x00
#define EP932E_AUDIO_INPUT_FORMAT__ADO_FREQ__32000HZ			0x01
#define EP932E_AUDIO_INPUT_FORMAT__ADO_FREQ__44100HZ			0x02
#define EP932E_AUDIO_INPUT_FORMAT__ADO_FREQ__48000HZ			0x03
#define EP932E_AUDIO_INPUT_FORMAT__ADO_FREQ__88200HZ			0x04
#define EP932E_AUDIO_INPUT_FORMAT__ADO_FREQ__96000HZ			0x05
#define EP932E_AUDIO_INPUT_FORMAT__ADO_FREQ__176400HZ			0x06
#define EP932E_AUDIO_INPUT_FORMAT__ADO_FREQ__192000HZ			0x07

#define EP932E_KSV_REVOCATION_LIST				0x2600

#define EP932E_GAMUT_PACKET_HEADER				0x2700
#define EP932E_GAMUT_PACKET_DATA				0x2703

#define EP932E_SELECT_PACKET_HEADER				0x2800
#define EP932E_SELECT_PACKET_DATA				0x2803
#endif
