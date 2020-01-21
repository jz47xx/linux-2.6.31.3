
#include "ep932settingsdata.h"

vdo_settings ep932_vdo_settings[] = {
	//                   HVRes_Type,             DE_Gen,                     E_Sync, AR_PR,       Pix_Freq_Type,
	{  0,{       0,   0,   0,    0},{  0,   0,  0,   0},{0x00,  0,  0,  0,  0,   0},  0x00,                  1}, // 0:
	// HDMI Mode
	{  1,{VNEGHNEG, 800, 525,16666},{ 48, 640, 34, 480},{0x00, 12, 96, 10,  2,   0},  0x10,  PIX_FREQ_25175KHZ}, // 1:    640 x  480p
	{  2,{VNEGHNEG, 858, 525,16666},{ 60, 720, 31, 480},{0x00, 12, 62,  9,  6,   0},  0x10,  PIX_FREQ_27000KHZ}, // 2:    720 x  480p  4:3
	{  3,{VNEGHNEG, 858, 525,16666},{ 60, 720, 31, 480},{0x00, 12, 62,  9,  6,   0},  0x20,  PIX_FREQ_27000KHZ}, // 3:    720 x  480p 16:9
	{  4,{VPOSHPOS,1650, 750,16666},{220,1280, 21, 720},{0x00,106, 40,  5,  5,   0},  0x20,  PIX_FREQ_74176KHZ}, // 4:   1280 x  720p
	{  5,{VPOSHPOS,2200, 563,16666},{148,1920, 16, 540},{0x09, 84, 44,  2,  5,1100},  0x20,  PIX_FREQ_74176KHZ}, // 5:   1920 x 1080i
	{  6,{VNEGHNEG, 858, 262,16666},{ 57, 720, 16, 240},{0x09, 15, 62,  4,  3, 429},  0x15,  PIX_FREQ_27000KHZ}, // 6:    720 x  480i, pix repl
	{  7,{VNEGHNEG, 858, 262,16666},{ 57, 720, 16, 240},{0x09, 15, 62,  4,  3, 429},  0x25,  PIX_FREQ_27000KHZ}, // 7:    720 x  480i, pix repl
	{  8,{VNEGHNEG, 858, 262,16666},{ 57, 720, 16, 240},{0x00, 15, 62,  4,  3,   0},  0x15,  PIX_FREQ_27000KHZ}, // 8:    720 x  240p, pix repl
	{  9,{VNEGHNEG, 858, 262,16666},{ 57, 720, 16, 240},{0x00, 15, 62,  4,  3,   0},  0x25,  PIX_FREQ_27000KHZ}, // 9:    720 x  240p, pix repl
	{ 10,{VNEGHNEG,3432, 262,16666},{228,2880, 16, 240},{0x09, 72,248,  4,  3,1716},  0x10,  PIX_FREQ_54000KHZ}, // 10:  2880 x  480i
	{ 11,{VNEGHNEG,3432, 262,16666},{228,2880, 16, 240},{0x09, 72,248,  4,  3,1716},  0x20,  PIX_FREQ_54000KHZ}, // 11:  2880 x  480i
	{ 12,{VNEGHNEG,3432, 262,16666},{228,2880, 16, 240},{0x00, 72,248,  4,  3,   0},  0x10,  PIX_FREQ_54000KHZ}, // 12:  2880 x  240p
	{ 13,{VNEGHNEG,3432, 262,16666},{228,2880, 16, 240},{0x00, 72,248,  4,  3,   0},  0x20,  PIX_FREQ_54000KHZ}, // 13:  2880 x  240p
	{ 14,{VNEGHNEG,1716, 525,16666},{120,1440, 31, 480},{0x00, 28,124,  9,  6,   0},  0x10,  PIX_FREQ_54000KHZ}, // 14:  1440 x  480p
	{ 15,{VNEGHNEG,1716, 525,16666},{120,1440, 31, 480},{0x00, 28,124,  9,  6,   0},  0x20,  PIX_FREQ_54000KHZ}, // 15:  1440 x  480p
	{ 16,{VPOSHPOS,2200,1125,16666},{148,1920, 37,1080},{0x00, 84, 44,  4,  5,   0},  0x20, PIX_FREQ_148352KHZ}, // 16:  1920 x 1080p
	{ 17,{VNEGHNEG, 864, 625,20000},{ 68, 720, 40, 576},{0x00,  8, 64,  5,  5,   0},  0x10,  PIX_FREQ_27000KHZ}, // 17:   720 x  576p
	{ 18,{VNEGHNEG, 864, 625,20000},{ 68, 720, 40, 576},{0x00,  8, 64,  5,  5,   0},  0x20,  PIX_FREQ_27000KHZ}, // 18:   720 x  576p
	{ 19,{VPOSHPOS,1980, 750,20000},{220,1280, 21, 720},{0x00,436, 40,  5,  5,   0},  0x20,  PIX_FREQ_74250KHZ}, // 19:  1280 x  720p, 50 Hz
	{ 20,{VPOSHPOS,2640, 563,20000},{148,1920, 16, 540},{0x09,524, 44,  2,  5,1320},  0x20,  PIX_FREQ_74250KHZ}, // 20:  1920 x 1080i, 50 Hz
	{ 21,{VNEGHNEG, 864, 313,20000},{ 69, 720, 20, 288},{0x09,  8, 63,  2,  3, 432},  0x15,  PIX_FREQ_27000KHZ}, // 21:   720 x  576i, pix repl
	{ 22,{VNEGHNEG, 864, 313,20000},{ 69, 720, 20, 288},{0x09,  8, 63,  2,  3, 432},  0x25,  PIX_FREQ_27000KHZ}, // 22:   720 x  576i, pix repl
	{ 23,{VNEGHNEG, 864, 313,20000},{ 69, 720, 20, 288},{0x00,  8, 63,  3,  3,   0},  0x15,  PIX_FREQ_27000KHZ}, // 23:   720 x  288p, pix repl
	{ 24,{VNEGHNEG, 864, 313,20000},{ 69, 720, 20, 288},{0x00,  8, 63,  3,  3,   0},  0x25,  PIX_FREQ_27000KHZ}, // 24:   720 x  288p, pix repl
	{ 25,{VNEGHNEG,3456, 313,20000},{276,2880, 20, 288},{0x09, 44,252,  2,  3,1728},  0x10,  PIX_FREQ_54000KHZ}, // 25:  2880 x  576i
	{ 26,{VNEGHNEG,3456, 313,20000},{276,2880, 20, 288},{0x09, 44,252,  2,  3,1728},  0x20,  PIX_FREQ_54000KHZ}, // 26:  2880 x  576i
	{ 27,{VNEGHNEG,3456, 313,20000},{276,2880, 20, 288},{0x00, 44,252,  3,  3,   0},  0x10,  PIX_FREQ_54000KHZ}, // 27:  2880 x  288p
	{ 28,{VNEGHNEG,3456, 313,20000},{276,2880, 20, 288},{0x00, 44,252,  3,  3,   0},  0x20,  PIX_FREQ_54000KHZ}, // 28:  2880 x  288p
	{ 29,{VPOSHNEG,1728, 625,20000},{136,1440, 40, 576},{0x00, 20,128,  5,  5,   0},  0x10,  PIX_FREQ_54000KHZ}, // 29:  1440 x  576p
	{ 30,{VPOSHNEG,1728, 625,20000},{136,1440, 40, 576},{0x00, 20,128,  5,  5,   0},  0x20,  PIX_FREQ_54000KHZ}, // 30:  1440 x  576p
	{ 31,{VPOSHPOS,2640,1125,20000},{148,1920, 37,1080},{0x00,524, 44,  4,  5,   0},  0x20, PIX_FREQ_148500KHZ}, // 31:  1920 x 1080p, 50 Hz
	{ 32,{VPOSHPOS,2750,1125,41666},{148,1920, 37,1080},{0x00,634, 44,  4,  5,   0},  0x20,  PIX_FREQ_74176KHZ}, // 32:  1920 x 1080p
	{ 33,{VPOSHPOS,2640,1125,40000},{148,1920, 37,1080},{0x00,524, 44,  4,  5,   0},  0x20,  PIX_FREQ_74250KHZ}, // 33:  1920 x 1080p, 25 Hz
	{ 34,{VPOSHPOS,2200,1125,33333},{148,1920, 37,1080},{0x00, 84, 44,  4,  5,   0},  0x20,  PIX_FREQ_74176KHZ}, // 34:  1920 x 1080p

	{ 35,{VNEGHNEG,3432, 525,16666},{240,2880, 31, 480},{0x00, 92,248,  9,  6,   0},  0x10, PIX_FREQ_108000KHZ}, // 35:  2880 x  480p
	{ 36,{VNEGHNEG,3432, 525,16666},{240,2880, 31, 480},{0x00, 92,248,  9,  6,   0},  0x20, PIX_FREQ_108000KHZ}, // 36:  2880 x  480p
	{ 37,{VNEGHNEG,3456, 625,20000},{272,2880, 40, 576},{0x00, 44,256,  5,  5,   0},  0x10, PIX_FREQ_108000KHZ}, // 37:  2880 x  576p @ 50Hz
	{ 38,{VNEGHNEG,3456, 625,20000},{272,2880, 40, 576},{0x00, 44,256,  5,  5,   0},  0x20, PIX_FREQ_108000KHZ}, // 38:  2880 x  576p @ 50Hz
	{ 39,{VPOSHNEG,2304, 625,20000},{184,1920, 58, 540},{0x09, 28,168,  2,  5,1152},  0x20,  PIX_FREQ_72000KHZ}, // 39:  1920 x 1080i @ 50Hz
	{ 40,{VPOSHPOS,2640, 563,10000},{148,1920, 16, 540},{0x09,524, 44,  2,  5,1320},  0x20, PIX_FREQ_148500KHZ}, // 40:  1920 x 1080i @ 100Hz
	{ 41,{VPOSHPOS,1980, 750,10000},{220,1280, 21, 720},{0x00,436, 40,  5,  5,   0},  0x20, PIX_FREQ_148500KHZ}, // 41:  1280 x  720p @ 100Hz
	{ 42,{VNEGHNEG, 864, 625,10000},{ 68, 720, 40, 576},{0x00,  8, 64,  5,  5,   0},  0x10,  PIX_FREQ_54000KHZ}, // 42:   720 x  576p @ 100Hz
	{ 43,{VNEGHNEG, 864, 625,10000},{ 68, 720, 40, 576},{0x00,  8, 64,  5,  5,   0},  0x20,  PIX_FREQ_54000KHZ}, // 43:   720 x  576p @ 100Hz
	{ 44,{VNEGHNEG, 864, 313,10000},{ 69, 720, 20, 288},{0x09,  8, 63,  2,  3, 432},  0x15,  PIX_FREQ_54000KHZ}, // 44:   720 x  576i @ 100Hz, pix repl
	{ 45,{VNEGHNEG, 864, 313,10000},{ 69, 720, 20, 288},{0x09,  8, 63,  2,  3, 432},  0x25,  PIX_FREQ_54000KHZ}, // 45:   720 x  576i @ 100Hz, pix repl
	{ 46,{VPOSHPOS,2200, 563, 8333},{148,1920, 16, 540},{0x09, 84, 44,  2,  5,2200},  0x20, PIX_FREQ_148352KHZ}, // 46:  1920 x 1080i @ 119.88/120Hz
	{ 47,{VPOSHPOS,1650, 750, 8333},{220,1280, 21, 720},{0x00,106, 40,  5,  5,   0},  0x20, PIX_FREQ_148352KHZ}, // 47:  1280 x  720p @ 119.88/120Hz
	{ 48,{VNEGHNEG, 858, 525, 8333},{ 60, 720, 31, 480},{0x00, 12, 62,  9,  6,   0},  0x10,  PIX_FREQ_54000KHZ}, // 48:   720 x  480p @ 119.88/120Hz
	{ 49,{VNEGHNEG, 858, 525, 8333},{ 60, 720, 31, 480},{0x00, 12, 62,  9,  6,   0},  0x20,  PIX_FREQ_54000KHZ}, // 49:   720 x  480p @ 119.88/120Hz
	{ 50,{VNEGHNEG, 858, 262, 8333},{ 57, 720, 16, 240},{0x09, 15, 62,  4,  3, 429},  0x15,  PIX_FREQ_54000KHZ}, // 50:   720 x  480i @ 119.88/120Hz, pix repl
	{ 51,{VNEGHNEG, 858, 262, 8333},{ 57, 720, 16, 240},{0x09, 15, 62,  4,  3, 429},  0x25,  PIX_FREQ_54000KHZ}, // 51:   720 x  480i @ 119.88/120Hz, pix repl
	{ 52,{VNEGHNEG, 864, 625, 5000},{ 68, 720, 40, 576},{0x00,  8, 64,  5,  5,   0},  0x10, PIX_FREQ_108000KHZ}, // 52:   720 x  576p @ 200Hz
	{ 53,{VNEGHNEG, 864, 625, 5000},{ 68, 720, 40, 576},{0x00,  8, 64,  5,  5,   0},  0x20, PIX_FREQ_108000KHZ}, // 53:   720 x  576p @ 200Hz
	{ 54,{VNEGHNEG, 864, 313, 5000},{ 69, 720, 20, 288},{0x09,  8, 63,  2,  3, 432},  0x15, PIX_FREQ_108000KHZ}, // 54:   720 x  576i @ 200Hz, pix repl
	{ 55,{VNEGHNEG, 864, 313, 5000},{ 69, 720, 20, 288},{0x09,  8, 63,  2,  3, 432},  0x25, PIX_FREQ_108000KHZ}, // 55:   720 x  576i @ 200Hz, pix repl
	{ 56,{VNEGHNEG, 858, 525, 4166},{ 60, 720, 31, 480},{0x00, 12, 62,  9,  6,   0},  0x10, PIX_FREQ_108000KHZ}, // 56:   720 x  480p @ 239.76/240Hz
	{ 57,{VNEGHNEG, 858, 525, 4166},{ 60, 720, 31, 480},{0x00, 12, 62,  9,  6,   0},  0x20, PIX_FREQ_108000KHZ}, // 57:   720 x  480p @ 239.76/240Hz
	{ 58,{VNEGHNEG, 858, 263, 4166},{ 57, 720, 16, 240},{0x09, 15, 62,  4,  3, 429},  0x15, PIX_FREQ_108000KHZ}, // 58:   720 x  480i @ 239.76/240Hz, pix repl
	{ 59,{VNEGHNEG, 858, 263, 4166},{ 57, 720, 16, 240},{0x09, 15, 62,  4,  3, 429},  0x25, PIX_FREQ_108000KHZ}, // 59:   720 x  480i @ 239.76/240Hz, pix repl

	// Special
	{  6,{VNEGHNEG,1716, 262,16666},{114,1440, 16, 240},{0x09, 30,124,  4,  3, 858},  0x14,  PIX_FREQ_27000KHZ}, // 60:   720 x  480i, pix repl
	{  7,{VNEGHNEG,1716, 262,16666},{114,1440, 16, 240},{0x09, 30,124,  4,  3, 858},  0x24,  PIX_FREQ_27000KHZ}, // 61:   720 x  480i, pix repl
	{  8,{VNEGHNEG,1716, 262,16666},{114,1440, 16, 240},{0x00, 30,124,  4,  3,   0},  0x14,  PIX_FREQ_27000KHZ}, // 62:   720 x  240p, pix repl
	{  9,{VNEGHNEG,1716, 262,16666},{114,1440, 16, 240},{0x00, 30,124,  4,  3,   0},  0x24,  PIX_FREQ_27000KHZ}, // 63:   720 x  240p, pix repl

	{ 21,{VNEGHNEG,1728, 313,20000},{192,1440, 20, 288},{0x09, 20,128,  2,  3, 864},  0x14,  PIX_FREQ_27000KHZ}, // 64:   720 x  576i, pix repl
	{ 22,{VNEGHNEG,1728, 313,20000},{192,1440, 20, 288},{0x09, 20,128,  2,  3, 864},  0x24,  PIX_FREQ_27000KHZ}, // 65:   720 x  576i, pix repl
	{ 23,{VNEGHNEG,1728, 313,20000},{192,1440, 20, 288},{0x00, 20,128,  3,  3,   0},  0x14,  PIX_FREQ_27000KHZ}, // 66:   720 x  288p, pix repl
	{ 24,{VNEGHNEG,1728, 313,20000},{192,1440, 20, 288},{0x00, 20,128,  3,  3,   0},  0x24,  PIX_FREQ_27000KHZ}, // 67:   720 x  288p, pix repl

	{ 44,{VNEGHNEG,1728, 313,10000},{192,1440, 20, 288},{0x09, 20,128,  2,  3, 864},  0x14,  PIX_FREQ_54000KHZ}, // 68:   720 x  576i @ 100Hz, pix repl
	{ 45,{VNEGHNEG,1728, 313,10000},{192,1440, 20, 288},{0x09, 20,128,  2,  3, 864},  0x24,  PIX_FREQ_54000KHZ}, // 69:   720 x  576i @ 100Hz, pix repl

	{ 50,{VNEGHNEG,1716, 262, 8333},{114,1440, 16, 240},{0x09, 30,124,  4,  3, 858},  0x14,  PIX_FREQ_54000KHZ}, // 70:   720 x  480i @ 119.88/120Hz, pix repl
	{ 51,{VNEGHNEG,1716, 262, 8333},{114,1440, 16, 240},{0x09, 30,124,  4,  3, 858},  0x24,  PIX_FREQ_54000KHZ}, // 71:   720 x  480i @ 119.88/120Hz, pix repl

	{ 54,{VNEGHNEG,1728, 313, 5000},{192,1440, 20, 288},{0x09, 20,128,  2,  3, 864},  0x14, PIX_FREQ_108000KHZ}, // 72:   720 x  576i @ 200Hz, pix repl
	{ 55,{VNEGHNEG,1728, 313, 5000},{192,1440, 20, 288},{0x09, 20,128,  2,  3, 864},  0x24, PIX_FREQ_108000KHZ}, // 73:   720 x  576i @ 200Hz, pix repl

	{ 58,{VNEGHNEG,1716, 262, 4166},{114,1440, 16, 240},{0x09, 30,124,  4,  3, 858},  0x14, PIX_FREQ_108000KHZ}, // 74:   720 x  480i @ 239.76/240Hz, pix repl
	{ 59,{VNEGHNEG,1716, 262, 4166},{114,1440, 16, 240},{0x09, 30,124,  4,  3, 858},  0x24, PIX_FREQ_108000KHZ}, // 75:   720 x  480i @ 239.76/240Hz, pix repl

	// PC Mode (start form 76)
	{128,{VNEGHPOS, 832, 445,16683},{ 96, 640, 61, 350},{0x00, 28, 64, 32,  3,   0},  0x00,        PIX_FREQ_PC}, // 128:  640 x  350p @ 60HZ d
	{129,{VNEGHPOS, 832, 445,14285},{ 96, 640, 61, 350},{0x00, 28, 64, 32,  3,   0},  0x00,        PIX_FREQ_PC}, // 129:  640 x  350p @ 70HZ d
	{130,{VNEGHPOS, 832, 445,13333},{ 96, 640, 61, 350},{0x00, 28, 64, 32,  3,   0},  0x00,        PIX_FREQ_PC}, // 130:  640 x  350p @ 75HZ d
	{131,{VNEGHPOS, 832, 445,11753},{ 96, 640, 61, 350},{0x00, 28, 64, 32,  3,   0},  0x00,        PIX_FREQ_PC}, // 131:  640 x  350p @ 85HZ

	{132,{VPOSHNEG, 832, 445,16683},{ 96, 640, 42, 400},{0x00, 28, 64,  1,  3,   0},  0x00,        PIX_FREQ_PC}, // 132:  640 x  400p @ 60HZ d
	{133,{VPOSHNEG, 832, 445,14285},{ 96, 640, 42, 400},{0x00, 28, 64,  1,  3,   0},  0x00,        PIX_FREQ_PC}, // 133:  640 x  400p @ 70HZ d
	{134,{VPOSHNEG, 832, 445,13333},{ 96, 640, 42, 400},{0x00, 28, 64,  1,  3,   0},  0x00,        PIX_FREQ_PC}, // 134:  640 x  400p @ 75HZ d
	{135,{VPOSHNEG, 832, 445,11753},{ 96, 640, 42, 400},{0x00, 28, 64,  1,  3,   0},  0x00,        PIX_FREQ_PC}, // 135:  640 x  400p @ 85HZ

	{136,{VPOSHNEG, 900, 449,16683},{108, 720, 43, 400},{0x00, 14,108, 13,  2,   0},  0x00,        PIX_FREQ_PC}, // 136:  720 x  400p @ 60HZ d
	{137,{VPOSHNEG, 900, 449,14285},{108, 720, 43, 400},{0x00, 14,108, 13,  2,   0},  0x00,        PIX_FREQ_PC}, // 137:  720 x  400p @ 70HZ d
	{138,{VPOSHNEG, 900, 449,13333},{108, 720, 43, 400},{0x00, 14,108, 13,  2,   0},  0x00,        PIX_FREQ_PC}, // 138:  720 x  400p @ 75HZ d
	{139,{VPOSHNEG, 936, 446,11759},{108, 720, 43, 400},{0x00, 32, 72,  1,  3,   0},  0x00,        PIX_FREQ_PC}, // 139:  720 x  400p @ 85HZ

	// VGA
	{140,{VNEGHNEG, 800, 525,16683},{ 48, 640, 34, 480},{0x00,  8,100,  8,  4,   0},  0x10,  PIX_FREQ_25175KHZ}, // 140:  640 x  480p @ 60Hz
	{141,{VNEGHNEG, 832, 520,13734},{128, 640, 29, 480},{0x00, 20, 40,  9,  3,   0},  0x10,        PIX_FREQ_PC}, // 141:  640 x  480p @ 72HZ
	{142,{VNEGHNEG, 840, 500,13333},{120, 640, 17, 480},{0x00, 12, 64,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 142:  640 x  480p @ 75HZ
	{143,{VNEGHNEG, 832, 509,11763},{ 80, 640, 26, 480},{0x00, 52, 56,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 143:  640 x  480p @ 85HZ

	// SVGA
	{144,{VPOSHPOS,1056, 628,16579},{ 88, 800, 24, 600},{0x00, 36,128,  1,  4,   0},  0x10,        PIX_FREQ_PC}, // 144:  800 x  600p @ 60HZ
	{145,{VPOSHPOS,1040, 666,13852},{ 64, 800, 24, 600},{0x00, 52,120, 37,  6,   0},  0x10,        PIX_FREQ_PC}, // 145:  800 x  600p @ 72HZ
	{146,{VPOSHPOS,1056, 625,13333},{160, 800, 22, 600},{0x00, 12, 80,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 146:  800 x  600p @ 75HZ
	{147,{VPOSHPOS,1048, 631,11756},{152, 800, 28, 600},{0x00, 28, 64,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 147:  800 x  600p @ 85HZ

	{148,{VPOSHPOS,1088, 517,16666},{112, 848, 24, 480},{0x00, 12,112,  6,  8,   0},  0x20,        PIX_FREQ_PC}, // 148:  848 x  480p @ 60HZ
	{149,{VPOSHPOS,1088, 517,14285},{112, 848, 24, 480},{0x00, 12,112,  6,  8,   0},  0x20,        PIX_FREQ_PC}, // 149:  848 x  480p @ 70HZ d
	{150,{VPOSHPOS,1088, 517,13333},{112, 848, 24, 480},{0x00, 12,112,  6,  8,   0},  0x20,        PIX_FREQ_PC}, // 150:  848 x  480p @ 75HZ d
	{151,{VPOSHPOS,1088, 517,11764},{112, 848, 24, 480},{0x00, 12,112,  6,  8,   0},  0x20,        PIX_FREQ_PC}, // 151:  848 x  480p @ 85HZ d

	// XGA
	{152,{VNEGHNEG,1344, 806,16665},{160,1024, 30, 768},{0x00, 20,136,  3,  6,   0},  0x10,        PIX_FREQ_PC}, // 152: 1024 x  768p @ 60HZ
	{153,{VNEGHNEG,1328, 806,14271},{144,1024, 30, 768},{0x00, 20,136,  3,  6,   0},  0x10,        PIX_FREQ_PC}, // 153: 1024 x  768p @ 70HZ
	{154,{VPOSHPOS,1312, 800,13328},{176,1024, 29, 768},{0x00, 12, 96,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 154: 1024 x  768p @ 75HZ
	{155,{VPOSHPOS,1376, 808,11765},{208,1024, 37, 768},{0x00, 44, 96,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 155: 1024 x  768p @ 85HZ

	{156,{VPOSHPOS,1600, 900,16666},{256,1152, 33, 864},{0x00, 60,128,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 156: 1152 x  864p @ 60HZ d
	{157,{VPOSHPOS,1600, 900,14285},{256,1152, 33, 864},{0x00, 60,128,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 157: 1152 x  864p @ 70HZ d
	{158,{VPOSHPOS,1600, 900,13333},{256,1152, 33, 864},{0x00, 60,128,  1,  3,   0},  0x10, PIX_FREQ_108000KHZ}, // 158: 1152 x  864p @ 75Hz
	{159,{VPOSHPOS,1600, 900,11764},{256,1152, 33, 864},{0x00, 60,128,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 159: 1152 x  864p @ 85HZ d

	{160,{VPOSHNEG,1664, 798,16702},{192,1280, 21, 768},{0x00, 60,128,  3,  7,   0},  0x20,        PIX_FREQ_PC}, // 160: 1280 x  768p @ 60HZ
	{161,{VPOSHNEG,1696, 805,14285},{208,1280, 28, 768},{0x00, 76,128,  3,  7,   0},  0x20,        PIX_FREQ_PC}, // 161: 1280 x  768p @ 70HZ d
	{162,{VPOSHNEG,1696, 805,13352},{208,1280, 28, 768},{0x00, 76,128,  3,  7,   0},  0x20,        PIX_FREQ_PC}, // 162: 1280 x  768p @ 75HZ
	{163,{VPOSHNEG,1712, 809,11787},{216,1280, 32, 768},{0x00, 76,136,  3,  7,   0},  0x20,        PIX_FREQ_PC}, // 163: 1280 x  768p @ 85HZ

	{164,{VPOSHPOS,1800,1000,16666},{312,1280, 37, 960},{0x00, 92,112,  1,  3,   0},  0x10, PIX_FREQ_108000KHZ}, // 164: 1280 x  960p @ 60Hz
	{165,{VPOSHPOS,1728,1011,14285},{224,1280, 48, 960},{0x00, 60,160,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 165: 1280 x  960p @ 70HZ d
	{166,{VPOSHPOS,1728,1011,13333},{224,1280, 48, 960},{0x00, 60,160,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 166: 1280 x  960p @ 75HZ d
	{167,{VPOSHPOS,1728,1011,11764},{224,1280, 48, 960},{0x00, 60,160,  1,  3,   0},  0x10, PIX_FREQ_148500KHZ}, // 167: 1280 x  960p @ 85Hz

	// SXGA
	{168,{VPOSHPOS,1688,1066,16661},{248,1280, 39,1024},{0x00, 44,112,  1,  3,   0},  0x10, PIX_FREQ_108000KHZ}, // 168: 1280 x 1024p @ 60Hz
	{169,{VPOSHPOS,1688,1066,14285},{248,1280, 39,1024},{0x00, 12,144,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 169: 1280 x 1024p @ 70HZ d
	{170,{VPOSHPOS,1688,1066,13328},{248,1280, 39,1024},{0x00, 12,144,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 170: 1280 x 1024p @ 75HZ
	{171,{VPOSHPOS,1728,1072,11761},{224,1280, 45,1024},{0x00, 60,160,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 171: 1280 x 1024p @ 85HZ

	{172,{VPOSHPOS,1792, 795,16662},{256,1360, 19, 768},{0x00, 60,112,  3,  6,   0},  0x20,        PIX_FREQ_PC}, // 172: 1360 x  768p @ 60HZ
	{173,{VPOSHPOS,1792, 795,14285},{256,1360, 19, 768},{0x00, 60,112,  3,  6,   0},  0x20,        PIX_FREQ_PC}, // 173: 1360 x  768p @ 70HZ d
	{174,{VPOSHPOS,1792, 795,13333},{256,1360, 19, 768},{0x00, 60,112,  3,  6,   0},  0x20,        PIX_FREQ_PC}, // 174: 1360 x  768p @ 75HZ d
	{175,{VPOSHPOS,1792, 795,11764},{256,1360, 19, 768},{0x00, 60,112,  3,  6,   0},  0x20,        PIX_FREQ_PC}, // 175: 1360 x  768p @ 85HZ d

	{176,{VPOSHNEG,1864,1089,16672},{232,1400, 33,1050},{0x00, 84,144,  3,  4,   0},  0x10,        PIX_FREQ_PC}, // 176: 1400 x 1050p @ 60HZ
	{177,{VPOSHNEG,1896,1099,14285},{248,1400, 43,1050},{0x00,100,144,  3,  4,   0},  0x10,        PIX_FREQ_PC}, // 177: 1400 x 1050p @ 70HZ d
	{178,{VPOSHNEG,1896,1099,13357},{248,1400, 43,1050},{0x00,100,144,  3,  4,   0},  0x10,        PIX_FREQ_PC}, // 178: 1400 x 1050p @ 75HZ
	{179,{VPOSHNEG,1912,1105,11770},{256,1400, 49,1050},{0x00,100,152,  3,  4,   0},  0x10,        PIX_FREQ_PC}, // 179: 1400 x 1050p @ 85HZ

	{180,{VPOSHNEG,1904, 934,16698},{232,1440, 26, 900},{0x00, 76,152,  3,  6,   0},  0x00,        PIX_FREQ_PC}, // 180: 1440 x  900p @ 60HZ
	{181,{VPOSHNEG,1936, 942,14285},{248,1440, 34, 900},{0x00, 92,152,  3,  6,   0},  0x00,        PIX_FREQ_PC}, // 181: 1440 x  900p @ 70HZ d
	{182,{VPOSHNEG,1936, 942,13336},{248,1440, 34, 900},{0x00, 92,152,  3,  6,   0},  0x00,        PIX_FREQ_PC}, // 182: 1440 x  900p @ 75HZ
	{183,{VPOSHNEG,1952, 948,11786},{256,1440, 40, 900},{0x00,100,152,  3,  6,   0},  0x00,        PIX_FREQ_PC}, // 183: 1440 x  900p @ 85HZ

	// UXGA
	{184,{VPOSHPOS,2160,1250,16666},{304,1600, 47,1200},{0x00, 60,192,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 184: 1600 x 1200p @ 60HZ
	{185,{VPOSHPOS,2160,1250,14285},{304,1600, 47,1200},{0x00, 60,192,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 185: 1600 x 1200p @ 70HZ
	{186,{VPOSHPOS,2160,1250,13333},{304,1600, 47,1200},{0x00, 60,192,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 186: 1600 x 1200p @ 75HZ
	{187,{VPOSHPOS,2160,1250,11764},{304,1600, 47,1200},{0x00, 60,192,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 186: 1600 x 1200p @ 85HZ

	{188,{VPOSHNEG,2240,1089,16679},{280,1680, 31,1050},{0x00,100,176,  3,  6,   0},  0x10,        PIX_FREQ_PC}, // 188: 1680 x 1050p @ 60HZ
	{189,{VPOSHNEG,2272,1099,14285},{296,1680, 41,1050},{0x00,116,176,  3,  6,   0},  0x10,        PIX_FREQ_PC}, // 189: 1680 x 1050p @ 70HZ d
	{190,{VPOSHNEG,2272,1099,13352},{296,1680, 41,1050},{0x00,116,176,  3,  6,   0},  0x10,        PIX_FREQ_PC}, // 190: 1680 x 1050p @ 75HZ
	{191,{VPOSHNEG,2288,1105,11772},{304,1680, 47,1050},{0x00,124,176,  3,  6,   0},  0x10,        PIX_FREQ_PC}, // 191: 1680 x 1050p @ 85HZ

	{192,{VPOSHNEG,2448,1394,16666},{328,1792, 47,1344},{0x00,124,200,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 192: 1792 x 1344p @ 60HZ
	{193,{VPOSHNEG,2456,1417,14285},{352,1792, 70,1344},{0x00, 92,216,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 193: 1792 x 1344p @ 70HZ d
	{194,{VPOSHNEG,2456,1417,13333},{352,1792, 70,1344},{0x00, 92,216,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 194: 1792 x 1344p @ 75HZ
	{195,{VPOSHNEG,2456,1417,11764},{352,1792, 70,1344},{0x00, 92,216,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 195: 1792 x 1344p @ 85HZ d

	{196,{VPOSHNEG,2528,1439,16668},{352,1856, 44,1392},{0x00, 92,224,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 196: 1856 x 1392p @ 60HZ
	{197,{VPOSHNEG,2560,1500,14285},{352,1856,105,1392},{0x00,124,224,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 197: 1856 x 1392p @ 70HZ d
	{198,{VPOSHNEG,2560,1500,13333},{352,1856,105,1392},{0x00,124,224,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 198: 1856 x 1392p @ 75HZ
	{199,{VPOSHNEG,2560,1500,11764},{352,1856,105,1392},{0x00,124,224,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 199: 1856 x 1392p @ 85HZ d

	{200,{VPOSHNEG,2592,1245,16698},{336,1920, 37,1200},{0x00,132,200,  3,  6,   0},  0x10,        PIX_FREQ_PC}, // 200: 1920 x 1200p @ 60HZ
	{201,{VPOSHNEG,2608,1255,14285},{344,1920, 47,1200},{0x00,132,208,  3,  6,   0},  0x10,        PIX_FREQ_PC}, // 201: 1920 x 1200p @ 70HZ d
	{202,{VPOSHNEG,2608,1255,13345},{344,1920, 47,1200},{0x00,132,208,  3,  6,   0},  0x10,        PIX_FREQ_PC}, // 202: 1920 x 1200p @ 75HZ
	{203,{VPOSHNEG,2624,1262,11774},{352,1920, 54,1200},{0x00,140,208,  3,  6,   0},  0x10,        PIX_FREQ_PC}, // 203: 1920 x 1200p @ 85HZ

	{204,{VPOSHNEG,2600,1500,16666},{344,1920, 57,1440},{0x00,124,208,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 204: 1920 x 1440p @ 60HZ
	{205,{VPOSHNEG,2640,1500,14285},{352,1920, 57,1440},{0x00,140,224,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 205: 1920 x 1440p @ 70HZ d
	{206,{VPOSHNEG,2640,1500,13333},{352,1920, 57,1440},{0x00,140,224,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 206: 1920 x 1440p @ 75HZ
	{207,{VPOSHNEG,2640,1500,11774},{352,1920, 57,1440},{0x00,140,224,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 207: 1920 x 1440p @ 85HZ d

	// Special
	{144,{VPOSHPOS,1024, 625,17777},{128, 800, 23, 600},{0x00, 20, 72,  1,  2,   0},  0x10,        PIX_FREQ_PC}, // 144:  800 x  600p @ 56HZ *
	{160,{VNEGHPOS,1440, 790,16668},{ 80,1280, 13, 768},{0x00, 44, 32,  3,  7,   0},  0x10,        PIX_FREQ_PC}, // 160: 1280 x  768p @ 60HZ * (Reduced Blanking)
	{176,{VNEGHPOS,1560,1080,16681},{ 80,1400, 24,1050},{0x00, 44, 32,  3,  4,   0},  0x10,        PIX_FREQ_PC}, // 176: 1400 x 1050p @ 60HZ * (Reduced Blanking)
	{180,{VNEGHPOS,1600, 926,16694},{ 80,1440, 18, 900},{0x00, 44, 32,  3,  6,   0},  0x10,        PIX_FREQ_PC}, // 180: 1440 x  900p @ 60HZ * (Reduced Blanking)
	{184,{VPOSHPOS,2160,1250,15484},{304,1600, 47,1200},{0x00, 60,192,  1,  3,   0},  0x10,        PIX_FREQ_PC}, // 184: 1600 x 1200p @ 65HZ *
	{188,{VNEGHPOS,1840,1080,16699},{ 80,1680, 22,1050},{0x00, 44, 32,  3,  6,   0},  0x10,        PIX_FREQ_PC}, // 188: 1680 x 1050p @ 60HZ * (Reduced Blanking)
	{200,{VNEGHPOS,2080,1235,16680},{ 80,1920, 27,1200},{0x00, 44, 32,  3,  6,   0},  0x10,        PIX_FREQ_PC}, // 200: 1920 x 1200p @ 60HZ * (Reduced Blanking)
};

unsigned char ep932_vdo_settings_max = (sizeof(ep932_vdo_settings)/sizeof(ep932_vdo_settings[0]));

// Index = [Channel Number]
ado_settings ep932_ado_settings[] = {
	// SpeakerMapping, Flat
	{0x00, 0xF0}, // 0.0 - Flat All
	{0x00, 0x00}, // 2.0 - SD0
	{0x04, 0xA0}, // 3.0 - SD0 +SD2
	{0x08, 0xA0}, // 4.0 - SD0 +SD1 +SD2
	{0x07, 0x80}, // 4.1 - SD0 +SD3
	{0x0B, 0x80}, // 5.1 - SD0 +SD1 +SD3
	{0x0F, 0x00}, // 6.1 - SD0 +SD2 +SD3
	{0x13, 0x00}, // 7.1 - SD0 +SD1 +SD2 +SD3
};

// Index = [Pixel Freq Type]
n_cts_settings n_cts_32k[] = {
	{ 4576, 28125},        // ( 28125,  4576),  25.20 MHZ / 1.001
	{ 4096, 25200},        // ( 25200,  4096),  25.20 MHZ

	{ 4096, 27000},        // ( 27000,  4096),  27.00 MHZ
	{ 4096, 27027},        // ( 27027,  4096),  27.00 MHZ * 1.001

	{ 4096, 54000},        // ( 54000,  4096),  54.00 MHZ
	{ 4096, 54054},        // ( 54054,  4096),  54.00 MHZ * 1.001

	{ 4096, 72000},        // ( 72000,  4096),  72.00 MHZ

	{11648,210937},        // (210937, 11648),  74.25 MHZ / 1.001
	{ 4096, 74250},        // ( 74250,  4096),  74.25 MHZ

	{ 4096,108000},        // (108000,  4096), 108.00 MHZ
	{ 4096,108108},        // (108108,  4096), 108.00 MHZ * 1.001

	{11648,421875},        // (421875, 11648), 148.50 MHZ / 1.001
	{ 4096,148500},        // (148500,  4096), 148.50 MHZ

	{ 4096,148500},        // PC
};

n_cts_settings n_cts_44k1[] = {
	{ 7007, 31250},        // ( 31250,  7007),  25.20 MHZ / 1.001
	{ 6272, 28000},        // ( 28000,  6272),  25.20 MHZ

	{ 6272, 30000},        // ( 30000,  6272),  27.00 MHZ
	{ 6272, 30030},        // ( 30030,  6272),  27.00 MHZ * 1.001

	{ 6272, 60000},        // ( 60000,  6272),  54.00 MHZ
	{ 6272, 60060},        // ( 60060,  6272),  54.00 MHZ * 1.001

	{ 6272, 80000},        // ( 80000,  6272),  72.00 MHZ

	{17836,234375},        // (234375, 17836),  74.25 MHZ / 1.001
	{ 6272, 82500},        // ( 82500,  6272),  74.25 MHZ

	{ 6272,120000},        // (120000,  6272), 108.00 MHZ
	{ 6272,120120},        // (120120,  6272), 108.00 MHZ * 1.001

	{ 8918,234375},        // (234375,  8918), 148.50 MHZ / 1.001
	{ 6272,165000},        // (165000,  6272), 148.50 MHZ

	{ 6272,165000},        // PC
};

n_cts_settings n_cts_48k[] = {
	{ 6864, 28125},        // ( 28125,  6864),  25.20 MHZ / 1.001
	{ 6144, 25200},        // ( 25200,  6144),  25.20 MHZ

	{ 6144, 27000},        // ( 27000,  6144),  27.00 MHZ
	{ 6144, 27027},        // ( 27027,  6144),  27.00 MHZ * 1.001

	{ 6144, 54000},        // ( 54000,  6144),  54.00 MHZ
	{ 6144, 54054},        // ( 54054,  6144),  54.00 MHZ * 1.001

	{ 6144, 72000},        // ( 72000,  6144),  72.00 MHZ

	{11648,140625},        // (140625, 11648),  74.25 MHZ / 1.001
	{ 6144, 74250},        // ( 74250,  6144),  74.25 MHZ

	{ 6144, 108000},        // (108000,  6144), 108.00 MHZ
	{ 6144, 108108},        // (108108,  6144), 108.00 MHZ * 1.001

	{ 5824, 140625},        // (140625,  5824), 148.50 MHZ / 1.001
	{ 6144, 148500},        // (148500,  6144), 148.50 MHZ

	{ 6144, 148500},        // PC
};
