/*
 * linux/drivers/video/jzepd.c -- Ingenic AUO-EPD frame buffer device
 *
 * This program is used to support Electronic Paper Display;
 * you can redistribute it and/or modify it according to your own needs.
 *
 * This driver supports 4/8level waveform.
 * The first writen by Cynthia <zhzhao@ingenic.cn>
 */

#include "jz4760_epd.h"

//#define EPD_DEBUG

#ifdef EPD_DEBUG
#define D(fmt, arg...)			printk(fmt, ##arg)
#else
#define D(fmt, arg...)
#endif

#define INVALID_TEMP -1
#define VALID_TEMP    1

#define MOD_INVALID  -2
#define MOD_VALID     2

#define EPD_SUCCESS   0

#define TEMP_NUM  4
#define MOD_NUM   4

int  totally_time; // this is used to record multiple of 16 frames

extern unsigned char *lcd_palette;

enum epd_temperature_level
{
	TEMP_LOW_LEVEL = 0,  //temp0
	TEMP_HIGH_LEVEL ,    //temp1
	TEMP_HIGHER_LEVEL ,  //temp2
	TEMP_HIGHEST_LEVEL   //temp3
};
enum epd_temperature_level epd_temp_level;


enum epd_mod_level
{
	EPD_MOD_INIT = 0,
	EPD_MOD_DU, // white/black level
	EPD_MOD_GU, //gray level
	EPD_MOD_GC  //update all display with gray level
};
enum epd_mod_level epd_mod_level;

enum epd_gray_level
{
	GRAY_LEVEL_4 = 4,
	GRAY_LEVEL_8 = 8,
	GRAY_LEVEL_16 = 16
};
enum epd_gray_level epd_gray_level;


// waveform when temp = low level
unsigned int waveform_temp0_init[]={
};

unsigned int waveform_temp0_du[]={
};

unsigned int waveform_temp0_gu[]={
};

unsigned int waveform_temp0_gc[]={
};
// waveform when temp = high level
unsigned int waveform_temp1_init[]={
};

unsigned int waveform_temp1_du[]={
};

unsigned int waveform_temp1_gu[]={
};

unsigned int waveform_temp1_gc[]={
};
// waveform when temp = higher level
unsigned int waveform_temp2_init[]={
	0x55555554, 0xAAAAAAA8, 0x01555555, 0x500AAAAA, 0xAA801555, 0x555500AA, 0xAAAAA855, 0x55555402,
	0xAAAAAAA0,
};

unsigned int waveform_temp2_du[]={
	0x01000080, 0x01000080, 0x05000080, 0x05000080, 0x05000080, 0x05000080, 0x15000080, 0x15000080,
	0x15000080, 0x150000A0, 0x150000A0, 0x150000A0, 0x150000A0, 0x150000A8, 0x150000A8, 0x00000000,
};

unsigned int waveform_temp2_gu[]={
	0x01010100, 0x01010100, 0x05050104, 0x05050104, 0x05050104, 0x05050104, 0x15051114, 0x15051114,
	0x15051114, 0x15051114, 0x15051114, 0x15051114, 0x15051114, 0x15051114, 0x15051114, 0,
        0x2A8AA2A8, 0x2A8AA2A8, 0x2A8AA2A8, 0x2A8AA2A8, 0x2A8AA2A8, 0x2A8AA2A8, 0x2A8AA2A8, 0x2A8AA2A8,
	0x2A8AA2A8, 0x2A8AA2A8, 0x2A8AA2A8, 0x2A8AA2A8, 0x2A8AA2A8, 0x2A8AA2A8, 0x2A8AA2A8, 0,
	0x15455100, 0x15455100, 0x15450000, 0x15450000, 0x15450000, 0x15450000, 0x15000000, 0x15000000,
	0x15000000, 0x15000000, 0x15000000, 0x15000000, 0x15000000, 0x15000000, 0x15000000, 0,
};

unsigned int waveform_temp2_gc[]={
};

// waveform when temp = highest level
unsigned int waveform_temp3_init[]={
	0x5555554A, 0xAAAAA855, 0x5555402A, 0xAAAAA005, 0x555554AA, 0xAAAA8555, 0x555402AA, 0xAAAA0000,
};

unsigned int waveform_temp3_du[]={
	0x01000080,0x01000080,0x05000080,0x05000080,0x05000080,0x15000080,0x15000080,0x15000080,
	0x150000A0,0x150000A0,0x150000A0,0x150000A8,0x150000A8,0x0,

};

unsigned int waveform_temp3_gu[]={
	0x01010100, 0x01010100, 0x05050104, 0x05050104, 0x05050104, 0x15051114, 0x15051114, 0x15051114,
	0x15051114, 0x15051114, 0x15051114, 0x15051114, 0x15051114, 0,          0x2A8AA2A8, 0x2A8AA2A8,
	0x2A8AA2A8, 0x2A8AA2A8, 0x2A8AA2A8, 0x2A8AA2A8, 0x2A8AA2A8, 0x2A8AA2A8, 0x2A8AA2A8, 0x2A8AA2A8,
	0x2A8AA2A8, 0x2A8AA2A8, 0x2A8AA2A8, 0,        	0x15455100, 0x15455100, 0x15450000, 0x15450000,
	0x15450000, 0x15000000, 0x15000000, 0x15000000, 0x15000000, 0x15000000, 0x15000000, 0x15000000,
	0x15000000, 0,

};

unsigned int waveform_temp3_gc[]={
	0x01010101,0x01010101,0x05050505,0x05050505,0x05050505,0x15151515,0x15151515,0x15151515,
	0x15151515,0x15151515,0x15151515,0x15151515,0x15151515,0x00000000,0xAAAAAAAA,0xAAAAAAAA,
	0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,
	0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,0x00000000,0x55555500,0x55555500,0x55550000,0x55550000,
	0x55550000,0x55000000,0x55000000,0x55000000,0x55000000,0x55000000,0x55000000,0x55000000,
	0x55000000,0x00000000,
};


unsigned int waveform_handwriting[]={
	0x01000080,0x01000080,
};


/*========8 gray level waveform =======*/

unsigned int waveform8_temp0_init[]={
};

unsigned int waveform8_temp0_du[]={
};

unsigned int waveform8_temp0_gu[]={
};

unsigned int waveform8_temp0_gc[]={
};
// waveform when temp = high level
unsigned int waveform8_temp1_init[]={
};

unsigned int waveform8_temp1_du[]={
};

unsigned int waveform8_temp1_gu[]={
};

unsigned int waveform8_temp1_gc[]={
};
// waveform when temp = higher level
unsigned int waveform8_temp2_init[]={
};

unsigned int waveform8_temp2_du[]={
};

unsigned int waveform8_temp2_gu[]={
};

unsigned int waveform8_temp2_gc[]={
};
// waveform when temp = highest level
unsigned int waveform8_temp3_init[]={
	0x5555554A, 0xAAAAA955, 0x5555002A, 0xAAAAA001, 0x5555552A, 0xAAAAA555, 0x555400AA, 0xAAAA8000,
};

unsigned int waveform8_temp3_du[]={
	0x00100000,0x00000000,0x00000000,0x00008000,0x00050000,0x00000000,0x00000000,0x0000A000,
	0x00050000,0x00000000,0x00000000,0x0000A000,0x00150000,0x00000000,0x00000000,0x0000A800,
	0x00150000,0x00000000,0x00000000,0x0000A800,0x00550000,0x00000000,0x00000000,0x0000AA00,
	0x00550000,0x00000000,0x00000000,0x0000AA00,0x01550000,0x00000000,0x00000000,0x0000AA00,
	0x01550000,0x00000000,0x00000000,0x0000AA80,0x05550000,0x00000000,0x00000000,0x0000AAA0,
	0x05550000,0x00000000,0x00000000,0x0000AAA0,0x15550000,0x00000000,0x00000000,0x0000AAA8,
	0x15550000,0x00000000,0x00000000,0x0000AAA8,0,         0,         0,         0,

};

unsigned int waveform8_temp3_gu[]={

	0x00010001, 0x00010001, 0x00010001, 0x00010000, 0x00050005, 0x00050005, 0x00050005, 0x00010004,
	0x00050005, 0x00050005, 0x00050005, 0x00010004, 0x00150015, 0x00150015, 0x00150005, 0x00110014,
	0x00150015, 0x00150015, 0x00150005, 0x00110014, 0x00550055, 0x00550055, 0x00150045, 0x00510054,
	0x00550055, 0x00550055, 0x00150045, 0x00510054, 0x01550155, 0x01550055, 0x01150145, 0x01510154,
	0x01550155, 0x01550055, 0x01150145, 0x01510154, 0x05550555, 0x01550455, 0x05150545, 0x05510554,
	0x05550555, 0x01550455, 0x05150545, 0x05510554, 0x15550555, 0x11551455, 0x15151545, 0x15511554,
	0x15550555, 0x11551455, 0x15151545, 0x15511554, 0,          0,          0,          0,
	0x2AAA8AAA, 0xa2aaa8aa, 0xaa2a2a8a, 0xaaa2aaa8,  0x2AAA8AAA, 0xa2aaa8aa, 0xaa2a2a8a, 0xaaa2aaa8,
	0x2AAA8AAA, 0xa2aaa8aa, 0xaa2a2a8a, 0xaaa2aaa8, 0x2AAA8AAA, 0xa2aaa8aa, 0xaa2a2a8a, 0xaaa2aaa8,
	0x2AAA8AAA, 0xa2aaa8aa, 0xaa2a2a8a, 0xaaa2aaa8, 0x2AAA8AAA, 0xa2aaa8aa, 0xaa2a2a8a, 0xaaa2aaa8,
	0x2AAA8AAA, 0xa2aaa8aa, 0xaa2a2a8a, 0xaaa2aaa8, 0x2AAA8AAA, 0xa2aaa8aa, 0xaa2a2a8a, 0xaaa2aaa8,
	0x2AAA8AAA, 0xa2aaa8aa, 0xaa2a2a8a, 0xaaa2aaa8, 0x2AAA8AAA, 0xa2aaa8aa, 0xaa2a2a8a, 0xaaa2aaa8,
	0x2AAA8AAA, 0xa2aaa8aa, 0xaa2a2a8a, 0xaaa2aaa8, 0x2AAA8AAA, 0xa2aaa8aa, 0xaa2a2a8a, 0xaaa2aaa8,
	0x2AAA8AAA, 0xa2aaa8aa, 0xaa2a2a8a, 0xaaa2aaa8, 0x15550000, 0,          0,          0,
	0x15554555, 0x51555455, 0x55155545, 0x55510000, 0x15554555, 0x51555455, 0x55155545, 0,
	0x15554555, 0x51555455, 0x55155545, 0,          0x15554555, 0x51555455, 0x55150000, 0,
	0x15554555, 0x51555455, 0x55150000, 0,          0x15554555, 0x51555455, 0,          0,
	0x15554555, 0x51555455, 0x00000000, 0,          0x15554555, 0x51550000, 0,          0,
	0x15554555, 0x51550000, 0x00000000, 0,          0x15554555, 0,          0,          0,
	0x15554555, 0,          0,          0,          0x15550000, 0,          0,          0,
	0,          0,          0,          0,          0,          0,          0,          0,
};

unsigned int waveform8_temp3_gc[]={

	0x00010001, 0x00010001, 0x00010001, 0x00010001, 0x00050005, 0x00050005, 0x00050005, 0x00050005,
	0x00050005, 0x00050005, 0x00050005, 0x00050005, 0x00150015, 0x00150015, 0x00150015, 0x00150015,
	0x00150015, 0x00150015, 0x00150015, 0x00150015, 0x00550055, 0x00550055, 0x00550055, 0x00550055,
	0x00550055, 0x00550055, 0x00550055, 0x00550055, 0x01550155, 0x01550155, 0x01550155, 0x01550155,
	0x01550155, 0x01550155, 0x01550155, 0x01550155, 0x05550555, 0x05550555, 0x05550555, 0x05550555,
	0x05550555, 0x05550555, 0x05550555, 0x05550555, 0x15551555, 0x15551555, 0x15551555, 0x15551555,
	0x15551555, 0x15551555, 0x15551555, 0x15551555, 0,          0,          0,          0,
	0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa,
	0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa,
	0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa,
	0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa,
	0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa,
	0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa,
	0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0xaaaaaaaa, 0x55550000, 0,          0,          0,
	0x55555555, 0x55555555, 0x55555555, 0x55550000,	0x55555555, 0x55555555, 0x55555555, 0,
	0x55555555, 0x55555555, 0x55555555, 0,          0x55555555, 0x55555555, 0x55550000, 0,
	0x55555555, 0x55555555, 0x55550000, 0,          0x55555555, 0x55555555, 0,          0,
	0x55555555, 0x55555555, 0x00000000, 0,          0x55555555, 0x55550000, 0x00000000, 0,
	0x55555555, 0x55550000, 0x00000000, 0,          0x55555555, 0,          0,          0,
	0x55555555, 0,          0,          0,          0x55550000, 0,          0,          0,
	0,          0,          0,          0,          0,          0,          0,          0,

};

/*========16 gray level waveform =======*/
unsigned int waveform16_temp0_init[]={
};

unsigned int waveform16_temp0_du[]={
};

unsigned int waveform16_temp0_gu[]={
};

unsigned int waveform16_temp0_gc[]={
};
// waveform when temp = high level
unsigned int waveform16_temp1_init[]={
};

unsigned int waveform16_temp1_du[]={
};

unsigned int waveform16_temp1_gu[]={
};

unsigned int waveform16_temp1_gc[]={
};
// waveform when temp = higher level
unsigned int waveform16_temp2_init[]={
};

unsigned int waveform16_temp2_du[]={
};

unsigned int waveform16_temp2_gu[]={
};

unsigned int waveform16_temp2_gc[]={
};

// waveform when temp = highest level
unsigned int waveform16_temp3_init[]={
};

unsigned int waveform16_temp3_du[]={
};

unsigned int waveform16_temp3_gu[]={
};

unsigned int waveform16_temp3_gc[]={
};



void printpalette(void *palette, int len)
{
	unsigned short *pal = (unsigned short *)palette;
	int i;
	printk("palette:\n");
	for (i = 0; i < len; i++) {
		if ((i*8)%256 == 0)
			printk("\nfrm%d:\t", i*8/256);
		if (i % 16 == 0)
			printk("\n\t");
		printk("%04x ", pal[i]);
	}
	printk("\n");
}


int get_temp_sensor(void)
{
	return 20;
}

/*identify different temperature zone*/
int get_temp(void)
{
	int temp =0 ;

	temp = get_temp_sensor();

	if (temp <=5 && temp >= 0)
	{
		epd_temp_level = TEMP_LOW_LEVEL;
	}
	else if (temp <=12 && temp >= 6)
		epd_temp_level = TEMP_HIGH_LEVEL;

	else if (temp <=17 && temp >= 13)
		epd_temp_level = TEMP_HIGHER_LEVEL;

	else if (temp <=50 && temp >= 18)
		epd_temp_level = TEMP_HIGHEST_LEVEL;

	else
		return INVALID_TEMP;

	return VALID_TEMP;
}


void fill_init_palette(void)
{
	int i, j, offset, bit2;
	int max_frm ;
	int index_per_frame = (1 << 8);
	unsigned int *ptr = (unsigned int *)lcd_palette;
	int count=0, shift=0;
	unsigned int *p;

	memset(lcd_palette, 0x0, 4096*2);

	get_temp();
	D("%s:  temp(%d) gray level=%d. \n",__func__,epd_temp_level,epd_gray_level);

	if(epd_gray_level == GRAY_LEVEL_4){

		if (epd_temp_level ==  TEMP_LOW_LEVEL)
		{

			max_frm = 179;
			// record the totally multiple of 16frames
			if( max_frm%16 != 0)
				totally_time = (1+max_frm/16);
			else
				totally_time = max_frm/16;

			p = (unsigned int *)&waveform_temp0_init; // give waveform_temp2_init to p
			for (j = 0; j < max_frm; j++) {

				for(i=0;i<index_per_frame;i++){
					/*
					  (j * index_per_frame + i)*2(bit) = offset
					  but it alligned by word, so offset = offset/32;
					*/
					offset = (j * index_per_frame + i)*2/32;
					/*
					  store by word so shif 2 bit per 2 bit voltage until 32bit(a word)
					  from Lsb to Msb bit2 = 0,2,4,6,8,10...30
					*/
					bit2 = ((j * index_per_frame + i)%16)*2;

					ptr[offset] |= ((*(p+count) >> (30-shift*2)) & 0x3) << bit2;

				}
				if (shift <15 )
					shift ++;
				else {
					shift = 0;
					count ++;
				}

			}
		}
		else if (epd_temp_level ==  TEMP_HIGH_LEVEL)
		{

			max_frm = 143;

			// record the totally multiple of 16frames
			if( max_frm%16 != 0)
				totally_time = (1+max_frm/16);
			else
				totally_time = max_frm/16;

			p = (unsigned int *)&waveform_temp1_init; // give waveform_temp2_init to p
			for (j = 0; j < max_frm; j++) {

				for(i=0;i<index_per_frame;i++){
					offset = (j * index_per_frame + i)*2/32;
					bit2 = ((j * index_per_frame + i)%16)*2;

					ptr[offset] |= ((*(p+count) >> (30-shift*2)) & 0x3) << bit2;

				}
				if (shift <15 )
					shift ++;
				else {
					count ++;
					shift = 0;

				}

			}

		}
		else if (epd_temp_level ==  TEMP_HIGHER_LEVEL)
		{

			max_frm = 143;
			// record the totally multiple of 16frames
			if( max_frm%16 != 0)
				totally_time = (1+max_frm/16);
			else
				totally_time = max_frm/16;

			p =  (unsigned int *)&waveform_temp2_init; // give waveform_temp2_init to p
			for (j = 0; j < max_frm; j++) {

				for(i=0;i<index_per_frame;i++){
					offset = (j * index_per_frame + i)*2/32;
				bit2 = ((j * index_per_frame + i)%16)*2;

				ptr[offset] |= ((*(p+count) >> (30-shift*2)) & 0x3) << bit2;

				}
				if (shift <15 ) // when shift(0~14),shift++, so shift tatally is 16
					shift ++;
				else {
				count ++;
				shift = 0;
				}

			}
		}
		else if (epd_temp_level ==  TEMP_HIGHEST_LEVEL)
		{

			max_frm = 121;
			// record the totally multiple of 16frames
			if( max_frm%16 != 0)
				totally_time = (1+max_frm/16);
			else
				totally_time = max_frm/16;

			p =  (unsigned int *)&waveform_temp3_init; // give waveform_temp2_init to p
			for (j = 0; j < max_frm; j++) {

				for(i=0;i<index_per_frame;i++){

					offset = (j * index_per_frame + i)*2/32;
					bit2 = ((j * index_per_frame + i)%16)*2;

					ptr[offset] |= ((*(p+count) >> (30-shift*2)) & 0x3) << bit2;

				}

				if (shift < 15 )
					shift ++;// when shift(0~14),shift++, so shift tatally is 16
				else {
					count ++;
					shift = 0;
				}

			}
		}
		else{
			printk("Invalid temperature level. \n");
			return;
		}
	}
	if(epd_gray_level == GRAY_LEVEL_8){
		if (epd_temp_level ==  TEMP_LOW_LEVEL)
		{

			max_frm = 179;
			// record the totally multiple of 16frames
			if( max_frm%16 != 0)
				totally_time = (1+max_frm/16);
			else
				totally_time = max_frm/16;

			p = (unsigned int *)&waveform8_temp0_init; // give waveform_temp2_init to p
			for (j = 0; j < max_frm; j++) {

				for(i=0;i<index_per_frame;i++){
					/*
					  (j * index_per_frame + i)*2(bit) = offset
					  but it alligned by word, so offset = offset/32;
					*/
					offset = (j * index_per_frame + i)*2/32;
					/*
					  store by word so shif 2 bit per 2 bit voltage until 32bit(a word)
					  from Lsb to Msb bit2 = 0,2,4,6,8,10...30
					*/
					bit2 = ((j * index_per_frame + i)%16)*2;

					ptr[offset] |= ((*(p+count) >> (30-shift*2)) & 0x3) << bit2;

				}
				if (shift <15 )
					shift ++;
				else {
					shift = 0;
					count ++;
			}

			}
		}
		else if (epd_temp_level ==  TEMP_HIGH_LEVEL)
		{

			max_frm = 143;

			// record the totally multiple of 16frames
			if( max_frm%16 != 0)
				totally_time = (1+max_frm/16);
			else
				totally_time = max_frm/16;

			p = (unsigned int *)&waveform8_temp1_init; // give waveform_temp2_init to p
			for (j = 0; j < max_frm; j++) {

				for(i=0;i<index_per_frame;i++){
					offset = (j * index_per_frame + i)*2/32;
					bit2 = ((j * index_per_frame + i)%16)*2;

					ptr[offset] |= ((*(p+count) >> (30-shift*2)) & 0x3) << bit2;

				}
				if (shift <15 )
					shift ++;
				else {
					count ++;
					shift = 0;

				}

			}

		}
		else if (epd_temp_level ==  TEMP_HIGHER_LEVEL)
		{

			max_frm = 143;
			// record the totally multiple of 16frames
			if( max_frm%16 != 0)
				totally_time = (1+max_frm/16);
			else
				totally_time = max_frm/16;

			p =  (unsigned int *)&waveform8_temp2_init; // give waveform_temp2_init to p
			for (j = 0; j < max_frm; j++) {

				for(i=0;i<index_per_frame;i++){
					offset = (j * index_per_frame + i)*2/32;
					bit2 = ((j * index_per_frame + i)%16)*2;

					ptr[offset] |= ((*(p+count) >> (30-shift*2)) & 0x3) << bit2;

				}
				if (shift <15 ) // when shift(0~14),shift++, so shift tatally is 16
					shift ++;
				else {
					count ++;
					shift = 0;
				}

			}
		}
		else if (epd_temp_level ==  TEMP_HIGHEST_LEVEL)
		{

			max_frm = 122;
			// record the totally multiple of 16frames
			if( max_frm%16 != 0)
				totally_time = (1+max_frm/16);
			else
				totally_time = max_frm/16;

			p =  (unsigned int *)&waveform8_temp3_init; // give waveform_temp2_init to p
			for (j = 0; j < max_frm; j++) {

				for(i=0;i<index_per_frame;i++){

					offset = (j * index_per_frame + i)*2/32;
					bit2 = ((j * index_per_frame + i)%16)*2;

					ptr[offset] |= ((*(p+count) >> (30-shift*2)) & 0x3) << bit2;

				}

				if (shift < 15 )
					shift ++;// when shift(0~14),shift++, so shift tatally is 16
				else {
					count ++;
					shift = 0;
				}

			}
		}
		else{
			printk("Invalid temperature level. \n");
			return;
		}




	}
	if(epd_gray_level == GRAY_LEVEL_16){
		if (epd_temp_level ==  TEMP_LOW_LEVEL)
		{

			max_frm = 179;
			// record the totally multiple of 16frames
			if( max_frm%16 != 0)
				totally_time = (1+max_frm/16);
			else
				totally_time = max_frm/16;

			p = (unsigned int *)&waveform16_temp0_init; // give waveform_temp2_init to p
			for (j = 0; j < max_frm; j++) {

				for(i=0;i<index_per_frame;i++){
					/*
					  (j * index_per_frame + i)*2(bit) = offset
					  but it alligned by word, so offset = offset/32;
					*/
					offset = (j * index_per_frame + i)*2/32;
					/*
					  store by word so shif 2 bit per 2 bit voltage until 32bit(a word)
					  from Lsb to Msb bit2 = 0,2,4,6,8,10...30
					*/
					bit2 = ((j * index_per_frame + i)%16)*2;

					ptr[offset] |= ((*(p+count) >> (30-shift*2)) & 0x3) << bit2;

				}
				if (shift <15 )
					shift ++;
				else {
					shift = 0;
					count ++;
			}

			}
		}
		else if (epd_temp_level ==  TEMP_HIGH_LEVEL)
		{

			max_frm = 143;

			// record the totally multiple of 16frames
			if( max_frm%16 != 0)
				totally_time = (1+max_frm/16);
			else
				totally_time = max_frm/16;

			p = (unsigned int *)&waveform16_temp1_init; // give waveform_temp2_init to p
			for (j = 0; j < max_frm; j++) {

				for(i=0;i<index_per_frame;i++){
					offset = (j * index_per_frame + i)*2/32;
					bit2 = ((j * index_per_frame + i)%16)*2;

					ptr[offset] |= ((*(p+count) >> (30-shift*2)) & 0x3) << bit2;

				}
				if (shift <15 )
					shift ++;
				else {
					count ++;
					shift = 0;

				}

			}

		}
		else if (epd_temp_level ==  TEMP_HIGHER_LEVEL)
		{

			max_frm = 143;
			// record the totally multiple of 16frames
			if( max_frm%16 != 0)
				totally_time = (1+max_frm/16);
			else
				totally_time = max_frm/16;

			p =  (unsigned int *)&waveform16_temp2_init; // give waveform_temp2_init to p
			for (j = 0; j < max_frm; j++) {

				for(i=0;i<index_per_frame;i++){
					offset = (j * index_per_frame + i)*2/32;
					bit2 = ((j * index_per_frame + i)%16)*2;

					ptr[offset] |= ((*(p+count) >> (30-shift*2)) & 0x3) << bit2;

				}
				if (shift <15 ) // when shift(0~14),shift++, so shift tatally is 16
					shift ++;
				else {
					count ++;
					shift = 0;
				}

			}
		}
		else if (epd_temp_level ==  TEMP_HIGHEST_LEVEL)
		{

			max_frm = 122;
			// record the totally multiple of 16frames
			if( max_frm%16 != 0)
				totally_time = (1+max_frm/16);
			else
				totally_time = max_frm/16;

			p =  (unsigned int *)&waveform16_temp3_init; // give waveform_temp2_init to p
			for (j = 0; j < max_frm; j++) {

				for(i=0;i<index_per_frame;i++){

					offset = (j * index_per_frame + i)*2/32;
					bit2 = ((j * index_per_frame + i)%16)*2;

					ptr[offset] |= ((*(p+count) >> (30-shift*2)) & 0x3) << bit2;

				}

				if (shift < 15 )
					shift ++;// when shift(0~14),shift++, so shift tatally is 16
				else {
					count ++;
					shift = 0;
				}

			}
		}
		else{
			printk("Invalid temperature level. \n");
			return;
		}




	}

	dma_cache_wback((unsigned int)(lcd_palette), 4096);
//	printpalette((unsigned short *)((unsigned int)lcd_palette | 0xa0000000), 4096);
}

void fill_du_4level_gray(int max_frm,unsigned int *p)
{

	int i,j,old,new,offset, bit2;
	int count; // count is offset of array
	unsigned int *ptr = (unsigned int *)lcd_palette;
	int index_per_frame = (1 << 8);
	memset(lcd_palette, 0x0, 4096);
	// record the totally multiple of 16frames

	if( max_frm%16 != 0)
		totally_time = (1+max_frm/16);
	else
		totally_time = max_frm/16;


	for (j = 0; j < max_frm; j++) {
		count = j;
		for(i=0;i<index_per_frame;i++){

			old = (i >> 4) & 0xf;
			new = i & 0xf;
			offset = (j * index_per_frame + i)*2/32;
			bit2 = ((j * index_per_frame + i)%16)*2;

			if(old == 0 && new == 0x0)
				ptr[offset] |= ((*(p+count) >> (30-0*2)) & 0x3)<< bit2;
			if(old == 0xf && new == 0x0)
				ptr[offset] |= ((*(p+count) >> (30-3*2)) & 0x3)<< bit2;

			if(old == 0 && new == 0xf)
				ptr[offset] |= ((*(p+count) >> (30-2*12)) & 0x3)<< bit2;
			if(old == 0xf && new == 0xf)
				ptr[offset] |= ((*(p+count) >> (30-2*15)) & 0x3)<< bit2;

		}


	}

}

void fill_du_8level_gray(int max_frm,unsigned int *p)
{

	int i,j,old,new,offset, bit2;
	int count; // count is offset of array
	unsigned int *ptr = (unsigned int *)lcd_palette;
	int index_per_frame = (1 << 8);
	memset(lcd_palette, 0x0, 4096);
	// record the totally multiple of 16frames

	if( max_frm%16 != 0)
		totally_time = (1+max_frm/16);
	else
		totally_time = max_frm/16;


	for (j = 0; j < max_frm; j++) {
		count = 4*j;

		for(i=0;i<index_per_frame;i++){

			old = (i >> 4) & 0xf;
			new = i & 0xf;
			offset = (j * index_per_frame + i)*2/32;
			bit2 = ((j * index_per_frame + i)%16)*2;

			if(old == 0 && new == 0x0)
				ptr[offset] |= ((*(p+count) >> (30-0*2)) & 0x3)<< bit2;
			if(old == 0xf && new == 0x0)
				ptr[offset] |= ((*(p+count) >> (30-7*2)) & 0x3)<< bit2;



			if(old == 0 && new == 0xf)
				ptr[offset] |= ((*(p+count+3) >> (30-8*2)) & 0x3)<< bit2;
			if(old == 0xf && new == 0xf)
				ptr[offset] |= ((*(p+count+3) >> (30-2*15)) & 0x3)<< bit2;


		}


	}

}
void fill_du_16level_gray(int max_frm,unsigned int *p)
{

	int i,j,old,new,offset, bit2;
	int count; // count is offset of array
	unsigned int *ptr = (unsigned int *)lcd_palette;
	int index_per_frame = (1 << 8);
	memset(lcd_palette, 0x0, 4096);
	// record the totally multiple of 16frames

	if( max_frm%16 != 0)
		totally_time = (1+max_frm/16);
	else
		totally_time = max_frm/16;


	for (j = 0; j < max_frm; j++) {
		count = 16*j;

		for(i=0;i<index_per_frame;i++){

			old = (i >> 4) & 0xf;
			new = i & 0xf;
			offset = (j * index_per_frame + i)*2/32;
			bit2 = ((j * index_per_frame + i)%16)*2;

			if(old == 0 && new == 0x0)
				ptr[offset] |= ((*(p+count) >> (30-0*2)) & 0x3)<< bit2;
			if(old == 0xf && new == 0x0)
				ptr[offset] |= ((*(p+count) >> (30-15*2)) & 0x3)<< bit2;



			if(old == 0 && new == 0xf)
				ptr[offset] |= ((*(p+count+15) >> (30-0*2)) & 0x3)<< bit2;
			if(old == 0xf && new == 0xf)
				ptr[offset] |= ((*(p+count+15) >> (30-2*15)) & 0x3)<< bit2;


		}


	}

}

void fill_du_palette(void)
{

	int max_frm ;
	unsigned int *p;

	get_temp();
	D("%s:  temp(%d) gray level=%d. \n",__func__,epd_temp_level,epd_gray_level);

	if(epd_gray_level == GRAY_LEVEL_4)
	{
		if (epd_temp_level ==  TEMP_LOW_LEVEL)
		{

			max_frm = 16;
			p =  (unsigned int *)&waveform_temp0_du; // give waveform_temp2_Du to p


		}
		else if (epd_temp_level ==  TEMP_HIGH_LEVEL)
		{

			max_frm = 16;
			p =  (unsigned int *)&waveform_temp1_du; // give waveform_temp2_Du to p

		}
		else if (epd_temp_level ==  TEMP_HIGHER_LEVEL)
		{

			max_frm = 14;
			p =  (unsigned int *)&waveform_temp2_du; // give waveform_temp2_Du to p

		}

		else if (epd_temp_level ==  TEMP_HIGHEST_LEVEL)
		{

			max_frm = 14;
			p =  (unsigned int *)&waveform_temp3_du;


		}
		else{
			printk("Invalid temperature level. \n");
			return;
		}

		fill_du_4level_gray(max_frm,p);
	}

	if(epd_gray_level == GRAY_LEVEL_8)
	{


		if (epd_temp_level ==  TEMP_LOW_LEVEL)
		{

			max_frm = 16;
			p =  (unsigned int *)&waveform8_temp0_du; // give waveform_temp2_Du to p


		}
		else if (epd_temp_level ==  TEMP_HIGH_LEVEL)
		{

			max_frm = 16;
			p =  (unsigned int *)&waveform8_temp1_du; // give waveform_temp2_Du to p

		}
		else if (epd_temp_level ==  TEMP_HIGHER_LEVEL)
		{

			max_frm = 14;
			p =  (unsigned int *)&waveform8_temp2_du; // give waveform_temp2_Du to p

		}

		else if (epd_temp_level ==  TEMP_HIGHEST_LEVEL)
		{
			max_frm = 14;
			p =  (unsigned int *)&waveform8_temp3_du;
		}
		else{
			printk("Invalid temperature level. \n");
			return;
		}

		fill_du_8level_gray(max_frm,p);
	}
	if(epd_gray_level == GRAY_LEVEL_16)
	{
		// add 16level gray support


		if (epd_temp_level ==  TEMP_LOW_LEVEL)
		{

			max_frm = 16;
			p =  (unsigned int *)&waveform16_temp0_du; // give waveform_temp2_Du to p


		}
		else if (epd_temp_level ==  TEMP_HIGH_LEVEL)
		{

			max_frm = 16;
			p =  (unsigned int *)&waveform16_temp1_du; // give waveform_temp2_Du to p

		}
		else if (epd_temp_level ==  TEMP_HIGHER_LEVEL)
		{

			max_frm = 14;
			p =  (unsigned int *)&waveform16_temp2_du; // give waveform_temp2_Du to p

		}

		else if (epd_temp_level ==  TEMP_HIGHEST_LEVEL)
		{
			max_frm = 14;
			p =  (unsigned int *)&waveform16_temp3_du;
		}
		else{
			printk("Invalid temperature level. \n");
			return;
		}

		fill_du_8level_gray(max_frm,p);



	}
	dma_cache_wback((unsigned int)(lcd_palette), 4096);
//	printpalette((unsigned short *)((unsigned int)lcd_palette | 0xa0000000), 2048);

}


void fill_16level_gray(int max_frm,unsigned int *p)
{
	int i,j,old,new,offset, bit2;
	int index_per_frame = (1 << 8);
	int count; // count is offset of array
	unsigned int *ptr = (unsigned int *)lcd_palette;
	memset(lcd_palette, 0x0, 4096);

	// record the totally multiple of 16frames
	if( max_frm%16 != 0)
		totally_time = (1+max_frm/16);
	else
		totally_time = max_frm/16;

	for (j = 0; j < max_frm; j++) {
		count = 16*j;
		for(i=0;i<index_per_frame;i++){
			old = (i >> 4) & 0xf;
			new = i & 0xf;
			offset = (j * index_per_frame + i)*2/32;
			bit2 = ((j * index_per_frame + i)%16)*2;

			if(new == 0 && old == 0)
				ptr[offset] |= ((*(p+count) >> (30-0*2)) & 0x3) << bit2;
			if(new == 0 && old == 0x1)
				ptr[offset] |= ((*(p+count) >> (30-1*2)) & 0x3) << bit2;
			if(new == 0 && old == 0x2)
				ptr[offset] |= ((*(p+count) >> (30-2*2)) & 0x3) << bit2;
			if(new == 0 && old == 0x3)
				ptr[offset] |= ((*(p+count) >> (30-3*2)) & 0x3) << bit2;
			if(new == 0 && old == 0x4)
				ptr[offset] |= ((*(p+count) >> (30-4*2)) & 0x3) << bit2;
			if(new == 0 && old == 0x5)
				ptr[offset] |= ((*(p+count) >> (30-5*2)) & 0x3) << bit2;
			if(new == 0 && old == 0x6)
				ptr[offset] |= ((*(p+count) >> (30-6*2)) & 0x3) << bit2;
			if(new == 0 && old == 0x7)
				ptr[offset] |= ((*(p+count) >> (30-7*2)) & 0x3) << bit2;
			if(new == 0x0 && old == 0x8)
				ptr[offset] |= ((*(p+count) >> (30-8*2)) & 0x3) << bit2;
			if(new == 0x0 && old == 0x9)
				ptr[offset] |= ((*(p+count) >> (30-9*2)) & 0x3) << bit2;
			if(new == 0x0 && old == 0xa)
				ptr[offset] |= ((*(p+count) >> (30-10*2)) & 0x3) << bit2;
			if(new == 0x0 && old == 0xb)
				ptr[offset] |= ((*(p+count) >> (30-11*2)) & 0x3) << bit2;
			if(new == 0x0 && old == 0xc)
				ptr[offset] |= ((*(p+count) >> (30-12*2)) & 0x3) << bit2;
			if(new == 0x0 && old == 0xd)
				ptr[offset] |= ((*(p+count) >> (30-13*2)) & 0x3) << bit2;
			if(new == 0x0 && old == 0xe)
				ptr[offset] |= ((*(p+count) >> (30-14*2)) & 0x3) << bit2;
			if(new == 0x0 && old == 0xf)
				ptr[offset] |= ((*(p+count) >> (30-15*2)) & 0x3) << bit2;


			if(new == 0x1 && old == 0)
				ptr[offset] |= ((*(p+count+1) >> (30-0*2)) & 0x3) << bit2;
			if(new == 0x1 && old == 0x1)
				ptr[offset] |= ((*(p+count+1) >> (30-1*2)) & 0x3) << bit2;
			if(new == 0x1 && old == 0x2)
				ptr[offset] |= ((*(p+count+1) >> (30-2*2)) & 0x3) << bit2;
			if(new == 0x1 && old == 0x3)
				ptr[offset] |= ((*(p+count+1) >> (30-3*2)) & 0x3) << bit2;
			if(new == 0x1 && old == 0x4)
				ptr[offset] |= ((*(p+count+1) >> (30-4*2)) & 0x3) << bit2;
			if(new == 0x1 && old == 0x5)
				ptr[offset] |= ((*(p+count+1) >> (30-5*2)) & 0x3) << bit2;
			if(new == 0x1 && old == 0x6)
				ptr[offset] |= ((*(p+count+1) >> (30-6*2)) & 0x3) << bit2;
			if(new == 0x1 && old == 0x7)
				ptr[offset] |= ((*(p+count+1) >> (30-7*2)) & 0x3) << bit2;
			if(new == 0x1 && old == 0x8)
				ptr[offset] |= ((*(p+count+1) >> (30-8*2)) & 0x3) << bit2;
			if(new == 0x1 && old == 0x9)
				ptr[offset] |= ((*(p+count+1) >> (30-9*2)) & 0x3) << bit2;
			if(new == 0x1 && old == 0xa)
				ptr[offset] |= ((*(p+count+1) >> (30-10*2)) & 0x3) << bit2;
			if(new == 0x1 && old == 0xb)
				ptr[offset] |= ((*(p+count+1) >> (30-11*2)) & 0x3) << bit2;
			if(new == 0x1 && old == 0xc)
				ptr[offset] |= ((*(p+count+1) >> (30-12*2)) & 0x3) << bit2;
			if(new == 0x1 && old == 0xd)
				ptr[offset] |= ((*(p+count+1) >> (30-13*2)) & 0x3) << bit2;
			if(new == 0x1 && old == 0xe)
				ptr[offset] |= ((*(p+count+1) >> (30-14*2)) & 0x3) << bit2;
			if(new == 0x1 && old == 0xf)
				ptr[offset] |= ((*(p+count+1) >> (30-15*2)) & 0x3) << bit2;


			if(new == 0x2 && old == 0)
				ptr[offset] |= ((*(p+count+2) >> (30-0*2)) & 0x3) << bit2;
			if(new == 0x2 && old == 0x1)
				ptr[offset] |= ((*(p+count+2) >> (30-1*2)) & 0x3) << bit2;
			if(new == 0x2 && old == 0x2)
				ptr[offset] |= ((*(p+count+2) >> (30-2*2)) & 0x3) << bit2;
			if(new == 0x2 && old == 0x3)
				ptr[offset] |= ((*(p+count+2) >> (30-3*2)) & 0x3) << bit2;
			if(new == 0x2 && old == 0x4)
				ptr[offset] |= ((*(p+count+2) >> (30-4*2)) & 0x3) << bit2;
			if(new == 0x2 && old == 0x5)
				ptr[offset] |= ((*(p+count+2) >> (30-5*2)) & 0x3) << bit2;
			if(new == 0x2 && old == 0x6)
				ptr[offset] |= ((*(p+count+2) >> (30-6*2)) & 0x3) << bit2;
			if(new == 0x2 && old == 0x7)
				ptr[offset] |= ((*(p+count+2) >> (30-7*2)) & 0x3) << bit2;
			if(new == 0x2 && old == 0x8)
				ptr[offset] |= ((*(p+count+2) >> (30-8*2)) & 0x3) << bit2;
			if(new == 0x2 && old == 0x9)
				ptr[offset] |= ((*(p+count+2) >> (30-9*2)) & 0x3) << bit2;
			if(new == 0x2 && old == 0xa)
				ptr[offset] |= ((*(p+count+2) >> (30-10*2)) & 0x3) << bit2;
			if(new == 0x2 && old == 0xb)
				ptr[offset] |= ((*(p+count+2) >> (30-11*2)) & 0x3) << bit2;
			if(new == 0x2 && old == 0xc)
				ptr[offset] |= ((*(p+count+2) >> (30-12*2)) & 0x3) << bit2;
			if(new == 0x2 && old == 0xd)
				ptr[offset] |= ((*(p+count+2) >> (30-13*2)) & 0x3) << bit2;
			if(new == 0x2 && old == 0xe)
				ptr[offset] |= ((*(p+count+2) >> (30-14*2)) & 0x3) << bit2;
			if(new == 0x2 && old == 0xf)
				ptr[offset] |= ((*(p+count+2) >> (30-15*2)) & 0x3) << bit2;

			if(new == 0x3 && old == 0)
				ptr[offset] |= ((*(p+count+3) >> (30-0*2)) & 0x3) << bit2;
			if(new == 0x3 && old == 0x1)
				ptr[offset] |= ((*(p+count+3) >> (30-1*2)) & 0x3) << bit2;
			if(new == 0x3 && old == 0x2)
				ptr[offset] |= ((*(p+count+3) >> (30-2*2)) & 0x3) << bit2;
			if(new == 0x3 && old == 0x3)
				ptr[offset] |= ((*(p+count+3) >> (30-3*2)) & 0x3) << bit2;
			if(new == 0x3 && old == 0x4)
				ptr[offset] |= ((*(p+count+3) >> (30-4*2)) & 0x3) << bit2;
			if(new == 0x3 && old == 0x5)
				ptr[offset] |= ((*(p+count+3) >> (30-5*2)) & 0x3) << bit2;
			if(new == 0x3 && old == 0x6)
				ptr[offset] |= ((*(p+count+3) >> (30-6*2)) & 0x3) << bit2;
			if(new == 0x3 && old == 0x7)
				ptr[offset] |= ((*(p+count+3) >> (30-7*2)) & 0x3) << bit2;
			if(new == 0x3 && old == 0x8)
				ptr[offset] |= ((*(p+count+3) >> (30-8*2)) & 0x3) << bit2;
			if(new == 0x3 && old == 0x9)
				ptr[offset] |= ((*(p+count+3) >> (30-9*2)) & 0x3) << bit2;
			if(new == 0x3 && old == 0xa)
				ptr[offset] |= ((*(p+count+3) >> (30-10*2)) & 0x3) << bit2;
			if(new == 0x3 && old == 0xb)
				ptr[offset] |= ((*(p+count+3) >> (30-11*2)) & 0x3) << bit2;
			if(new == 0x3 && old == 0xc)
				ptr[offset] |= ((*(p+count+3) >> (30-12*2)) & 0x3) << bit2;
			if(new == 0x3 && old == 0xd)
				ptr[offset] |= ((*(p+count+3) >> (30-13*2)) & 0x3) << bit2;
			if(new == 0x3 && old == 0xe)
				ptr[offset] |= ((*(p+count+3) >> (30-14*2)) & 0x3) << bit2;
			if(new == 0x3 && old == 0xf)
				ptr[offset] |= ((*(p+count+3) >> (30-15*2)) & 0x3) << bit2;


			if(new == 0x4 && old == 0)
				ptr[offset] |= ((*(p+count+4) >> (30-0*2)) & 0x3) << bit2;
			if(new == 0x4 && old == 0x1)
				ptr[offset] |= ((*(p+count+4) >> (30-1*2)) & 0x3) << bit2;
			if(new == 0x4 && old == 0x2)
				ptr[offset] |= ((*(p+count+4) >> (30-2*2)) & 0x3) << bit2;
			if(new == 0x4 && old == 0x3)
				ptr[offset] |= ((*(p+count+4) >> (30-3*2)) & 0x3) << bit2;
			if(new == 0x4 && old == 0x4)
				ptr[offset] |= ((*(p+count+4) >> (30-4*2)) & 0x3) << bit2;
			if(new == 0x4 && old == 0x5)
				ptr[offset] |= ((*(p+count+4) >> (30-5*2)) & 0x3) << bit2;
			if(new == 0x4 && old == 0x6)
				ptr[offset] |= ((*(p+count+4) >> (30-6*2)) & 0x3) << bit2;
			if(new == 0x4 && old == 0x7)
				ptr[offset] |= ((*(p+count+4) >> (30-7*2)) & 0x3) << bit2;
			if(new == 0x4 && old == 0x8)
				ptr[offset] |= ((*(p+count+4) >> (30-8*2)) & 0x3) << bit2;
			if(new == 0x4 && old == 0x9)
				ptr[offset] |= ((*(p+count+4) >> (30-9*2)) & 0x3) << bit2;
			if(new == 0x4 && old == 0xa)
				ptr[offset] |= ((*(p+count+4) >> (30-10*2)) & 0x3) << bit2;
			if(new == 0x4 && old == 0xb)
				ptr[offset] |= ((*(p+count+4) >> (30-11*2)) & 0x3) << bit2;
			if(new == 0x4 && old == 0xc)
				ptr[offset] |= ((*(p+count+4) >> (30-12*2)) & 0x3) << bit2;
			if(new == 0x4 && old == 0xd)
				ptr[offset] |= ((*(p+count+4) >> (30-13*2)) & 0x3) << bit2;
			if(new == 0x4 && old == 0xe)
				ptr[offset] |= ((*(p+count+4) >> (30-14*2)) & 0x3) << bit2;
			if(new == 0x4 && old == 0xf)
				ptr[offset] |= ((*(p+count+4) >> (30-15*2)) & 0x3) << bit2;


			if(new == 0x5 && old == 0)
				ptr[offset] |= ((*(p+count+5) >> (30-0*2)) & 0x3) << bit2;
			if(new == 0x5 && old == 0x1)
				ptr[offset] |= ((*(p+count+5) >> (30-1*2)) & 0x3) << bit2;
			if(new == 0x5 && old == 0x2)
				ptr[offset] |= ((*(p+count+5) >> (30-2*2)) & 0x3) << bit2;
			if(new == 0x5 && old == 0x3)
				ptr[offset] |= ((*(p+count+5) >> (30-3*2)) & 0x3) << bit2;
			if(new == 0x5 && old == 0x4)
				ptr[offset] |= ((*(p+count+5) >> (30-4*2)) & 0x3) << bit2;
			if(new == 0x5 && old == 0x5)
				ptr[offset] |= ((*(p+count+5) >> (30-5*2)) & 0x3) << bit2;
			if(new == 0x5 && old == 0x6)
				ptr[offset] |= ((*(p+count+5) >> (30-6*2)) & 0x3) << bit2;
			if(new == 0x5 && old == 0x7)
				ptr[offset] |= ((*(p+count+5) >> (30-7*2)) & 0x3) << bit2;
			if(new == 0x5 && old == 0x8)
				ptr[offset] |= ((*(p+count+5) >> (30-8*2)) & 0x3) << bit2;
			if(new == 0x5 && old == 0x9)
				ptr[offset] |= ((*(p+count+5) >> (30-9*2)) & 0x3) << bit2;
			if(new == 0x5 && old == 0xa)
				ptr[offset] |= ((*(p+count+5) >> (30-10*2)) & 0x3) << bit2;
			if(new == 0x5 && old == 0xb)
				ptr[offset] |= ((*(p+count+5) >> (30-11*2)) & 0x3) << bit2;
			if(new == 0x5 && old == 0xc)
				ptr[offset] |= ((*(p+count+5) >> (30-12*2)) & 0x3) << bit2;
			if(new == 0x5 && old == 0xd)
				ptr[offset] |= ((*(p+count+5) >> (30-13*2)) & 0x3) << bit2;
			if(new == 0x5 && old == 0xe)
				ptr[offset] |= ((*(p+count+5) >> (30-14*2)) & 0x3) << bit2;
			if(new == 0x5 && old == 0xf)
				ptr[offset] |= ((*(p+count+5) >> (30-15*2)) & 0x3) << bit2;

			if(new == 0x6 && old == 0)
				ptr[offset] |= ((*(p+count+6) >> (30-0*2)) & 0x3) << bit2;
			if(new == 0x6 && old == 0x1)
				ptr[offset] |= ((*(p+count+6) >> (30-1*2)) & 0x3) << bit2;
			if(new == 0x6 && old == 0x2)
				ptr[offset] |= ((*(p+count+6) >> (30-2*2)) & 0x3) << bit2;
			if(new == 0x6 && old == 0x3)
				ptr[offset] |= ((*(p+count+6) >> (30-3*2)) & 0x3) << bit2;
			if(new == 0x6 && old == 0x4)
				ptr[offset] |= ((*(p+count+6) >> (30-4*2)) & 0x3) << bit2;
			if(new == 0x6 && old == 0x5)
				ptr[offset] |= ((*(p+count+6) >> (30-5*2)) & 0x3) << bit2;
			if(new == 0x6 && old == 0x6)
				ptr[offset] |= ((*(p+count+6) >> (30-6*2)) & 0x3) << bit2;
			if(new == 0x6 && old == 0x7)
				ptr[offset] |= ((*(p+count+6) >> (30-7*2)) & 0x3) << bit2;
			if(new == 0x6 && old == 0x8)
				ptr[offset] |= ((*(p+count+6) >> (30-8*2)) & 0x3) << bit2;
			if(new == 0x6 && old == 0x9)
				ptr[offset] |= ((*(p+count+6) >> (30-9*2)) & 0x3) << bit2;
			if(new == 0x6 && old == 0xa)
				ptr[offset] |= ((*(p+count+6) >> (30-10*2)) & 0x3) << bit2;
			if(new == 0x6 && old == 0xb)
				ptr[offset] |= ((*(p+count+6) >> (30-11*2)) & 0x3) << bit2;
			if(new == 0x6 && old == 0xc)
				ptr[offset] |= ((*(p+count+6) >> (30-12*2)) & 0x3) << bit2;
			if(new == 0x6 && old == 0xd)
				ptr[offset] |= ((*(p+count+6) >> (30-13*2)) & 0x3) << bit2;
			if(new == 0x6 && old == 0xe)
				ptr[offset] |= ((*(p+count+6) >> (30-14*2)) & 0x3) << bit2;
			if(new == 0x6 && old == 0xf)
				ptr[offset] |= ((*(p+count+6) >> (30-15*2)) & 0x3) << bit2;

			if(new == 0x7 && old == 0)
				ptr[offset] |= ((*(p+count+7) >> (30-0*2)) & 0x3) << bit2;
			if(new == 0x7 && old == 0x1)
				ptr[offset] |= ((*(p+count+7) >> (30-1*2)) & 0x3) << bit2;
			if(new == 0x7 && old == 0x2)
				ptr[offset] |= ((*(p+count+7) >> (30-2*2)) & 0x3) << bit2;
			if(new == 0x7 && old == 0x3)
				ptr[offset] |= ((*(p+count+7) >> (30-3*2)) & 0x3) << bit2;
			if(new == 0x7 && old == 0x4)
				ptr[offset] |= ((*(p+count+7) >> (30-4*2)) & 0x3) << bit2;
			if(new == 0x7 && old == 0x5)
				ptr[offset] |= ((*(p+count+7) >> (30-5*2)) & 0x3) << bit2;
			if(new == 0x7 && old == 0x6)
				ptr[offset] |= ((*(p+count+7) >> (30-6*2)) & 0x3) << bit2;
			if(new == 0x7 && old == 0x7)
				ptr[offset] |= ((*(p+count+7) >> (30-7*2)) & 0x3) << bit2;
			if(new == 0x7 && old == 0x8)
				ptr[offset] |= ((*(p+count+7) >> (30-8*2)) & 0x3) << bit2;
			if(new == 0x7 && old == 0x9)
				ptr[offset] |= ((*(p+count+7) >> (30-9*2)) & 0x3) << bit2;
			if(new == 0x7 && old == 0xa)
				ptr[offset] |= ((*(p+count+7) >> (30-10*2)) & 0x3) << bit2;
			if(new == 0x7 && old == 0xb)
				ptr[offset] |= ((*(p+count+7) >> (30-11*2)) & 0x3) << bit2;
			if(new == 0x7 && old == 0xc)
				ptr[offset] |= ((*(p+count+7) >> (30-12*2)) & 0x3) << bit2;
			if(new == 0x7 && old == 0xd)
				ptr[offset] |= ((*(p+count+7) >> (30-13*2)) & 0x3) << bit2;
			if(new == 0x7 && old == 0xe)
				ptr[offset] |= ((*(p+count+7) >> (30-14*2)) & 0x3) << bit2;
			if(new == 0x7 && old == 0xf)
				ptr[offset] |= ((*(p+count+7) >> (30-15*2)) & 0x3) << bit2;

			if(new == 0x8 && old == 0)
				ptr[offset] |= ((*(p+count+8) >> (30-0*2)) & 0x3) << bit2;
			if(new == 0x8 && old == 0x1)
				ptr[offset] |= ((*(p+count+8) >> (30-1*2)) & 0x3) << bit2;
			if(new == 0x8 && old == 0x2)
				ptr[offset] |= ((*(p+count+8) >> (30-2*2)) & 0x3) << bit2;
			if(new == 0x8 && old == 0x3)
				ptr[offset] |= ((*(p+count+8) >> (30-3*2)) & 0x3) << bit2;
			if(new == 0x8 && old == 0x4)
				ptr[offset] |= ((*(p+count+8) >> (30-4*2)) & 0x3) << bit2;
			if(new == 0x8 && old == 0x5)
				ptr[offset] |= ((*(p+count+8) >> (30-5*2)) & 0x3) << bit2;
			if(new == 0x8 && old == 0x6)
				ptr[offset] |= ((*(p+count+8) >> (30-6*2)) & 0x3) << bit2;
			if(new == 0x8 && old == 0x7)
				ptr[offset] |= ((*(p+count+8) >> (30-7*2)) & 0x3) << bit2;
			if(new == 0x8 && old == 0x8)
				ptr[offset] |= ((*(p+count+8) >> (30-8*2)) & 0x3) << bit2;
			if(new == 0x8 && old == 0x9)
				ptr[offset] |= ((*(p+count+8) >> (30-9*2)) & 0x3) << bit2;
			if(new == 0x8 && old == 0xa)
				ptr[offset] |= ((*(p+count+8) >> (30-10*2)) & 0x3) << bit2;
			if(new == 0x8 && old == 0xb)
				ptr[offset] |= ((*(p+count+8) >> (30-11*2)) & 0x3) << bit2;
			if(new == 0x8 && old == 0xc)
				ptr[offset] |= ((*(p+count+8) >> (30-12*2)) & 0x3) << bit2;
			if(new == 0x8 && old == 0xd)
				ptr[offset] |= ((*(p+count+8) >> (30-13*2)) & 0x3) << bit2;
			if(new == 0x8 && old == 0xe)
				ptr[offset] |= ((*(p+count+8) >> (30-14*2)) & 0x3) << bit2;
			if(new == 0x8 && old == 0xf)
				ptr[offset] |= ((*(p+count+8) >> (30-15*2)) & 0x3) << bit2;

			if(new == 0x9 && old == 0)
				ptr[offset] |= ((*(p+count+9) >> (30-0*2)) & 0x3) << bit2;
			if(new == 0x9 && old == 0x1)
				ptr[offset] |= ((*(p+count+9) >> (30-1*2)) & 0x3) << bit2;
			if(new == 0x9 && old == 0x2)
				ptr[offset] |= ((*(p+count+9) >> (30-2*2)) & 0x3) << bit2;
			if(new == 0x9 && old == 0x3)
				ptr[offset] |= ((*(p+count+9) >> (30-3*2)) & 0x3) << bit2;
			if(new == 0x9 && old == 0x4)
				ptr[offset] |= ((*(p+count+9) >> (30-4*2)) & 0x3) << bit2;
			if(new == 0x9 && old == 0x5)
				ptr[offset] |= ((*(p+count+9) >> (30-5*2)) & 0x3) << bit2;
			if(new == 0x9 && old == 0x6)
				ptr[offset] |= ((*(p+count+9) >> (30-6*2)) & 0x3) << bit2;
			if(new == 0x9 && old == 0x7)
				ptr[offset] |= ((*(p+count+9) >> (30-7*2)) & 0x3) << bit2;
			if(new == 0x9 && old == 0x8)
				ptr[offset] |= ((*(p+count+9) >> (30-8*2)) & 0x3) << bit2;
			if(new == 0x9 && old == 0x9)
				ptr[offset] |= ((*(p+count+9) >> (30-9*2)) & 0x3) << bit2;
			if(new == 0x9 && old == 0xa)
				ptr[offset] |= ((*(p+count+9) >> (30-10*2)) & 0x3) << bit2;
			if(new == 0x9 && old == 0xb)
				ptr[offset] |= ((*(p+count+9) >> (30-11*2)) & 0x3) << bit2;
			if(new == 0x9 && old == 0xc)
				ptr[offset] |= ((*(p+count+9) >> (30-12*2)) & 0x3) << bit2;
			if(new == 0x9 && old == 0xd)
				ptr[offset] |= ((*(p+count+9) >> (30-13*2)) & 0x3) << bit2;
			if(new == 0x9 && old == 0xe)
				ptr[offset] |= ((*(p+count+9) >> (30-14*2)) & 0x3) << bit2;
			if(new == 0x9 && old == 0xf)
				ptr[offset] |= ((*(p+count+9) >> (30-15*2)) & 0x3) << bit2;

			if(new == 0xa && old == 0)
				ptr[offset] |= ((*(p+count+10) >> (30-0*2)) & 0x3) << bit2;
			if(new == 0xa && old == 0x1)
				ptr[offset] |= ((*(p+count+10) >> (30-1*2)) & 0x3) << bit2;
			if(new == 0xa && old == 0x2)
				ptr[offset] |= ((*(p+count+10) >> (30-2*2)) & 0x3) << bit2;
			if(new == 0xa && old == 0x3)
				ptr[offset] |= ((*(p+count+10) >> (30-3*2)) & 0x3) << bit2;
			if(new == 0xa && old == 0x4)
				ptr[offset] |= ((*(p+count+10) >> (30-4*2)) & 0x3) << bit2;
			if(new == 0xa && old == 0x5)
				ptr[offset] |= ((*(p+count+10) >> (30-5*2)) & 0x3) << bit2;
			if(new == 0xa && old == 0x6)
				ptr[offset] |= ((*(p+count+10) >> (30-6*2)) & 0x3) << bit2;
			if(new == 0xa && old == 0x7)
				ptr[offset] |= ((*(p+count+10) >> (30-7*2)) & 0x3) << bit2;
			if(new == 0xa && old == 0x8)
				ptr[offset] |= ((*(p+count+10) >> (30-8*2)) & 0x3) << bit2;
			if(new == 0xa && old == 0x9)
				ptr[offset] |= ((*(p+count+10) >> (30-9*2)) & 0x3) << bit2;
			if(new == 0xa && old == 0xa)
				ptr[offset] |= ((*(p+count+10) >> (30-10*2)) & 0x3) << bit2;
			if(new == 0xa && old == 0xb)
				ptr[offset] |= ((*(p+count+10) >> (30-11*2)) & 0x3) << bit2;
			if(new == 0xa && old == 0xc)
				ptr[offset] |= ((*(p+count+10) >> (30-12*2)) & 0x3) << bit2;
			if(new == 0xa && old == 0xd)
				ptr[offset] |= ((*(p+count+10) >> (30-13*2)) & 0x3) << bit2;
			if(new == 0xa && old == 0xe)
				ptr[offset] |= ((*(p+count+10) >> (30-14*2)) & 0x3) << bit2;
			if(new == 0xa && old == 0xf)
				ptr[offset] |= ((*(p+count+10) >> (30-15*2)) & 0x3) << bit2;

			if(new == 0xb && old == 0)
				ptr[offset] |= ((*(p+count+11) >> (30-0*2)) & 0x3) << bit2;
			if(new == 0xb && old == 0x1)
				ptr[offset] |= ((*(p+count+11) >> (30-1*2)) & 0x3) << bit2;
			if(new == 0xb && old == 0x2)
				ptr[offset] |= ((*(p+count+11) >> (30-2*2)) & 0x3) << bit2;
			if(new == 0xb && old == 0x3)
				ptr[offset] |= ((*(p+count+11) >> (30-3*2)) & 0x3) << bit2;
			if(new == 0xb && old == 0x4)
				ptr[offset] |= ((*(p+count+11) >> (30-4*2)) & 0x3) << bit2;
			if(new == 0xb && old == 0x5)
				ptr[offset] |= ((*(p+count+11) >> (30-5*2)) & 0x3) << bit2;
			if(new == 0xb && old == 0x6)
				ptr[offset] |= ((*(p+count+11) >> (30-6*2)) & 0x3) << bit2;
			if(new == 0xb && old == 0x7)
				ptr[offset] |= ((*(p+count+11) >> (30-7*2)) & 0x3) << bit2;
			if(new == 0xb && old == 0x8)
				ptr[offset] |= ((*(p+count+11) >> (30-8*2)) & 0x3) << bit2;
			if(new == 0xb && old == 0x9)
				ptr[offset] |= ((*(p+count+11) >> (30-9*2)) & 0x3) << bit2;
			if(new == 0xb && old == 0xa)
				ptr[offset] |= ((*(p+count+11) >> (30-10*2)) & 0x3) << bit2;
			if(new == 0xb && old == 0xb)
				ptr[offset] |= ((*(p+count+11) >> (30-11*2)) & 0x3) << bit2;
			if(new == 0xb && old == 0xc)
				ptr[offset] |= ((*(p+count+11) >> (30-12*2)) & 0x3) << bit2;
			if(new == 0xb && old == 0xd)
				ptr[offset] |= ((*(p+count+11) >> (30-13*2)) & 0x3) << bit2;
			if(new == 0xb && old == 0xe)
				ptr[offset] |= ((*(p+count+11) >> (30-14*2)) & 0x3) << bit2;
			if(new == 0xb && old == 0xf)
				ptr[offset] |= ((*(p+count+11) >> (30-15*2)) & 0x3) << bit2;

			if(new == 0xc && old == 0)
				ptr[offset] |= ((*(p+count+12) >> (30-0*2)) & 0x3) << bit2;
			if(new == 0xc && old == 0x1)
				ptr[offset] |= ((*(p+count+12) >> (30-1*2)) & 0x3) << bit2;
			if(new == 0xc && old == 0x2)
				ptr[offset] |= ((*(p+count+12) >> (30-2*2)) & 0x3) << bit2;
			if(new == 0xc && old == 0x3)
				ptr[offset] |= ((*(p+count+12) >> (30-3*2)) & 0x3) << bit2;
			if(new == 0xc && old == 0x4)
				ptr[offset] |= ((*(p+count+12) >> (30-4*2)) & 0x3) << bit2;
			if(new == 0xc && old == 0x5)
				ptr[offset] |= ((*(p+count+12) >> (30-5*2)) & 0x3) << bit2;
			if(new == 0xc && old == 0x6)
				ptr[offset] |= ((*(p+count+12) >> (30-6*2)) & 0x3) << bit2;
			if(new == 0xc && old == 0x7)
				ptr[offset] |= ((*(p+count+12) >> (30-7*2)) & 0x3) << bit2;
			if(new == 0xc && old == 0x8)
				ptr[offset] |= ((*(p+count+12) >> (30-8*2)) & 0x3) << bit2;
			if(new == 0xc && old == 0x9)
				ptr[offset] |= ((*(p+count+12) >> (30-9*2)) & 0x3) << bit2;
			if(new == 0xc && old == 0xa)
				ptr[offset] |= ((*(p+count+12) >> (30-10*2)) & 0x3) << bit2;
			if(new == 0xc && old == 0xb)
				ptr[offset] |= ((*(p+count+12) >> (30-11*2)) & 0x3) << bit2;
			if(new == 0xc && old == 0xc)
				ptr[offset] |= ((*(p+count+12) >> (30-12*2)) & 0x3) << bit2;
			if(new == 0xc && old == 0xd)
				ptr[offset] |= ((*(p+count+12) >> (30-13*2)) & 0x3) << bit2;
			if(new == 0xc && old == 0xe)
				ptr[offset] |= ((*(p+count+12) >> (30-14*2)) & 0x3) << bit2;
			if(new == 0xc && old == 0xf)
				ptr[offset] |= ((*(p+count+12) >> (30-15*2)) & 0x3) << bit2;

			if(new == 0xd && old == 0)
				ptr[offset] |= ((*(p+count+13) >> (30-0*2)) & 0x3) << bit2;
			if(new == 0xd && old == 0x1)
				ptr[offset] |= ((*(p+count+13) >> (30-1*2)) & 0x3) << bit2;
			if(new == 0xd && old == 0x2)
				ptr[offset] |= ((*(p+count+13) >> (30-2*2)) & 0x3) << bit2;
			if(new == 0xd && old == 0x3)
				ptr[offset] |= ((*(p+count+13) >> (30-3*2)) & 0x3) << bit2;
			if(new == 0xd && old == 0x4)
				ptr[offset] |= ((*(p+count+13) >> (30-4*2)) & 0x3) << bit2;
			if(new == 0xd && old == 0x5)
				ptr[offset] |= ((*(p+count+13) >> (30-5*2)) & 0x3) << bit2;
			if(new == 0xd && old == 0x6)
				ptr[offset] |= ((*(p+count+13) >> (30-6*2)) & 0x3) << bit2;
			if(new == 0xd && old == 0x7)
				ptr[offset] |= ((*(p+count+13) >> (30-7*2)) & 0x3) << bit2;
			if(new == 0xd && old == 0x8)
				ptr[offset] |= ((*(p+count+13) >> (30-8*2)) & 0x3) << bit2;
			if(new == 0xd && old == 0x9)
				ptr[offset] |= ((*(p+count+13) >> (30-9*2)) & 0x3) << bit2;
			if(new == 0xd && old == 0xa)
				ptr[offset] |= ((*(p+count+13) >> (30-10*2)) & 0x3) << bit2;
			if(new == 0xd && old == 0xb)
				ptr[offset] |= ((*(p+count+13) >> (30-11*2)) & 0x3) << bit2;
			if(new == 0xd && old == 0xc)
				ptr[offset] |= ((*(p+count+13) >> (30-12*2)) & 0x3) << bit2;
			if(new == 0xd && old == 0xd)
				ptr[offset] |= ((*(p+count+13) >> (30-13*2)) & 0x3) << bit2;
			if(new == 0xd && old == 0xe)
				ptr[offset] |= ((*(p+count+13) >> (30-14*2)) & 0x3) << bit2;
			if(new == 0xd && old == 0xf)
				ptr[offset] |= ((*(p+count+13) >> (30-15*2)) & 0x3) << bit2;

			if(new == 0xe && old == 0)
				ptr[offset] |= ((*(p+count+14) >> (30-0*2)) & 0x3) << bit2;
			if(new == 0xe && old == 0x1)
				ptr[offset] |= ((*(p+count+14) >> (30-1*2)) & 0x3) << bit2;
			if(new == 0xe && old == 0x2)
				ptr[offset] |= ((*(p+count+14) >> (30-2*2)) & 0x3) << bit2;
			if(new == 0xe && old == 0x3)
				ptr[offset] |= ((*(p+count+14) >> (30-3*2)) & 0x3) << bit2;
			if(new == 0xe && old == 0x4)
				ptr[offset] |= ((*(p+count+14) >> (30-4*2)) & 0x3) << bit2;
			if(new == 0xe && old == 0x5)
				ptr[offset] |= ((*(p+count+14) >> (30-5*2)) & 0x3) << bit2;
			if(new == 0xe && old == 0x6)
				ptr[offset] |= ((*(p+count+14) >> (30-6*2)) & 0x3) << bit2;
			if(new == 0xe && old == 0x7)
				ptr[offset] |= ((*(p+count+14) >> (30-7*2)) & 0x3) << bit2;
			if(new == 0xe && old == 0x8)
				ptr[offset] |= ((*(p+count+14) >> (30-8*2)) & 0x3) << bit2;
			if(new == 0xe && old == 0x9)
				ptr[offset] |= ((*(p+count+14) >> (30-9*2)) & 0x3) << bit2;
			if(new == 0xe && old == 0xa)
				ptr[offset] |= ((*(p+count+14) >> (30-10*2)) & 0x3) << bit2;
			if(new == 0xe && old == 0xb)
				ptr[offset] |= ((*(p+count+14) >> (30-11*2)) & 0x3) << bit2;
			if(new == 0xe && old == 0xc)
				ptr[offset] |= ((*(p+count+14) >> (30-12*2)) & 0x3) << bit2;
			if(new == 0xe && old == 0xd)
				ptr[offset] |= ((*(p+count+14) >> (30-13*2)) & 0x3) << bit2;
			if(new == 0xe && old == 0xe)
				ptr[offset] |= ((*(p+count+14) >> (30-14*2)) & 0x3) << bit2;
			if(new == 0xe && old == 0xf)
				ptr[offset] |= ((*(p+count+14) >> (30-15*2)) & 0x3) << bit2;

			if(new == 0xf && old == 0)
				ptr[offset] |= ((*(p+count+15) >> (30-0*2)) & 0x3) << bit2;
			if(new == 0xf && old == 0x1)
				ptr[offset] |= ((*(p+count+15) >> (30-1*2)) & 0x3) << bit2;
			if(new == 0xf && old == 0x2)
				ptr[offset] |= ((*(p+count+15) >> (30-2*2)) & 0x3) << bit2;
			if(new == 0xf && old == 0x3)
				ptr[offset] |= ((*(p+count+15) >> (30-3*2)) & 0x3) << bit2;
			if(new == 0xf && old == 0x4)
				ptr[offset] |= ((*(p+count+15) >> (30-4*2)) & 0x3) << bit2;
			if(new == 0xf && old == 0x5)
				ptr[offset] |= ((*(p+count+15) >> (30-5*2)) & 0x3) << bit2;
			if(new == 0xf && old == 0x6)
				ptr[offset] |= ((*(p+count+15) >> (30-6*2)) & 0x3) << bit2;
			if(new == 0xf && old == 0x7)
				ptr[offset] |= ((*(p+count+15) >> (30-7*2)) & 0x3) << bit2;
			if(new == 0xf && old == 0x8)
				ptr[offset] |= ((*(p+count+15) >> (30-8*2)) & 0x3) << bit2;
			if(new == 0xf && old == 0x9)
				ptr[offset] |= ((*(p+count+15) >> (30-9*2)) & 0x3) << bit2;
			if(new == 0xf && old == 0xa)
				ptr[offset] |= ((*(p+count+15) >> (30-10*2)) & 0x3) << bit2;
			if(new == 0xf && old == 0xb)
				ptr[offset] |= ((*(p+count+15) >> (30-11*2)) & 0x3) << bit2;
			if(new == 0xf && old == 0xc)
				ptr[offset] |= ((*(p+count+15) >> (30-12*2)) & 0x3) << bit2;
			if(new == 0xf && old == 0xd)
				ptr[offset] |= ((*(p+count+15) >> (30-13*2)) & 0x3) << bit2;
			if(new == 0xf && old == 0xe)
				ptr[offset] |= ((*(p+count+15) >> (30-14*2)) & 0x3) << bit2;
			if(new == 0xf && old == 0xf)
				ptr[offset] |= ((*(p+count+15) >> (30-15*2)) & 0x3) << bit2;

		}
	}


}


void fill_8level_gray(int max_frm,unsigned int *p)
{
	int i,j,old,new,offset, bit2;
	int index_per_frame = (1 << 8);
	int count; // count is offset of array
	unsigned int *ptr = (unsigned int *)lcd_palette;
	memset(lcd_palette, 0x0, 4096);

	// record the totally multiple of 16frames
	if( max_frm%16 != 0)
		totally_time = (1+max_frm/16);
	else
		totally_time = max_frm/16;

	for (j = 0; j < max_frm; j++) {
		count = 4*j;
		for(i=0;i<index_per_frame;i++){
			old = (i >> 4) & 0xf;
			new = i & 0xf;
			offset = (j * index_per_frame + i)*2/32;
			bit2 = ((j * index_per_frame + i)%16)*2;

			if(new == 0 && old == 0)
				ptr[offset] |= ((*(p+count) >> (30-0*2)) & 0x3) << bit2;
			if(new == 0 && old == 0x2)
				ptr[offset] |= ((*(p+count) >> (30-1*2)) & 0x3) << bit2;
			if(new == 0 && old == 0x5)
				ptr[offset] |= ((*(p+count) >> (30-2*2)) & 0x3) << bit2;
			if(new == 0 && old == 0x7)
				ptr[offset] |= ((*(p+count) >> (30-3*2)) & 0x3) << bit2;
			if(new == 0 && old == 0x8)
				ptr[offset] |= ((*(p+count) >> (30-4*2)) & 0x3) << bit2;
			if(new == 0 && old == 0xa)
				ptr[offset] |= ((*(p+count) >> (30-5*2)) & 0x3) << bit2;
			if(new == 0 && old == 0xc)
				ptr[offset] |= ((*(p+count) >> (30-6*2)) & 0x3) << bit2;
			if(new == 0 && old == 0xf)
				ptr[offset] |= ((*(p+count) >> (30-7*2)) & 0x3) << bit2;

			if(new == 0x2 && old == 0)
				ptr[offset] |= ((*(p+count) >> (30-8*2)) & 0x3) << bit2;
			if(new == 0x2 && old == 0x2)
				ptr[offset] |= ((*(p+count) >> (30-9*2)) & 0x3) << bit2;
			if(new == 0x2 && old == 0x5)
				ptr[offset] |= ((*(p+count) >> (30-10*2)) & 0x3) << bit2;
			if(new == 0x2 && old == 0x7)
				ptr[offset] |= ((*(p+count) >> (30-11*2)) & 0x3) << bit2;
			if(new == 0x2 && old == 0x8)
				ptr[offset] |= ((*(p+count) >> (30-12*2)) & 0x3) << bit2;
			if(new == 0x2 && old == 0xa)
				ptr[offset] |= ((*(p+count) >> (30-13*2)) & 0x3) << bit2;
			if(new == 0x2 && old == 0xc)
				ptr[offset] |= ((*(p+count) >> (30-14*2)) & 0x3) << bit2;
			if(new == 0x2 && old == 0xf)
				ptr[offset] |= ((*(p+count) >> (30-15*2)) & 0x3) << bit2;


			if(new == 0x5 && old == 0)
				ptr[offset] |= ((*(p+count+1) >> (30-0*2)) & 0x3) << bit2;
			if(new == 0x5 && old == 0x2)
				ptr[offset] |= ((*(p+count+1) >> (30-1*2)) & 0x3) << bit2;
			if(new == 0x5 && old == 0x5)
				ptr[offset] |= ((*(p+count+1) >> (30-2*2)) & 0x3) << bit2;
			if(new == 0x5 && old == 0x7)
				ptr[offset] |= ((*(p+count+1) >> (30-3*2)) & 0x3) << bit2;
			if(new == 0x5 && old == 0x8)
				ptr[offset] |= ((*(p+count+1) >> (30-4*2)) & 0x3) << bit2;
			if(new == 0x5 && old == 0xa)
				ptr[offset] |= ((*(p+count+1) >> (30-5*2)) & 0x3) << bit2;
			if(new == 0x5 && old == 0xc)
				ptr[offset] |= ((*(p+count+1) >> (30-6*2)) & 0x3) << bit2;
			if(new == 0x5 && old == 0xf)
				ptr[offset] |= ((*(p+count+1) >> (30-7*2)) & 0x3) << bit2;

			if(new == 0x7 && old == 0)
				ptr[offset] |= ((*(p+count+1) >> (30-8*2)) & 0x3) << bit2;
			if(new == 0x7 && old == 0x2)
				ptr[offset] |= ((*(p+count+1) >> (30-9*2)) & 0x3) << bit2;
			if(new == 0x7 && old == 0x5)
				ptr[offset] |= ((*(p+count+1) >> (30-10*2)) & 0x3) << bit2;
			if(new == 0x7 && old == 0x7)
				ptr[offset] |= ((*(p+count+1) >> (30-11*2)) & 0x3) << bit2;
			if(new == 0x7 && old == 0x8)
				ptr[offset] |= ((*(p+count+1) >> (30-12*2)) & 0x3) << bit2;
			if(new == 0x7 && old == 0xa)
				ptr[offset] |= ((*(p+count+1) >> (30-13*2)) & 0x3) << bit2;
			if(new == 0x7 && old == 0xc)
				ptr[offset] |= ((*(p+count+1) >> (30-14*2)) & 0x3) << bit2;
			if(new == 0x7 && old == 0xf)
				ptr[offset] |= ((*(p+count+1) >> (30-15*2)) & 0x3) << bit2;


			if(new == 0x8 && old == 0)
				ptr[offset] |= ((*(p+count+2) >> (30-0*2)) & 0x3) << bit2;
			if(new == 0x8 && old == 0x2)
				ptr[offset] |= ((*(p+count+2) >> (30-1*2)) & 0x3) << bit2;
			if(new == 0x8 && old == 0x5)
				ptr[offset] |= ((*(p+count+2) >> (30-2*2)) & 0x3) << bit2;
			if(new == 0x8 && old == 0x7)
				ptr[offset] |= ((*(p+count+2) >> (30-3*2)) & 0x3) << bit2;
			if(new == 0x8 && old == 0x8)
				ptr[offset] |= ((*(p+count+2) >> (30-4*2)) & 0x3) << bit2;
			if(new == 0x8 && old == 0xa)
				ptr[offset] |= ((*(p+count+2) >> (30-5*2)) & 0x3) << bit2;
			if(new == 0x8 && old == 0xc)
				ptr[offset] |= ((*(p+count+2) >> (30-6*2)) & 0x3) << bit2;
			if(new == 0x8 && old == 0xf)
				ptr[offset] |= ((*(p+count+2) >> (30-7*2)) & 0x3) << bit2;

			if(new == 0xa && old == 0)
				ptr[offset] |= ((*(p+count+2) >> (30-8*2)) & 0x3) << bit2;
			if(new == 0xa && old == 0x2)
				ptr[offset] |= ((*(p+count+2) >> (30-9*2)) & 0x3) << bit2;
			if(new == 0xa && old == 0x5)
				ptr[offset] |= ((*(p+count+2) >> (30-10*2)) & 0x3) << bit2;
			if(new == 0xa && old == 0x7)
				ptr[offset] |= ((*(p+count+2) >> (30-11*2)) & 0x3) << bit2;
			if(new == 0xa && old == 0x8)
				ptr[offset] |= ((*(p+count+2) >> (30-12*2)) & 0x3) << bit2;
			if(new == 0xa && old == 0xa)
				ptr[offset] |= ((*(p+count+2) >> (30-13*2)) & 0x3) << bit2;
			if(new == 0xa && old == 0xc)
				ptr[offset] |= ((*(p+count+2) >> (30-14*2)) & 0x3) << bit2;
			if(new == 0xa && old == 0xf)
				ptr[offset] |= ((*(p+count+2) >> (30-15*2)) & 0x3) << bit2;


			if(new == 0xc && old == 0)
				ptr[offset] |= ((*(p+count+3) >> (30-0*2)) & 0x3) << bit2;
			if(new == 0xc && old == 0x2)
				ptr[offset] |= ((*(p+count+3) >> (30-1*2)) & 0x3) << bit2;
			if(new == 0xc && old == 0x5)
				ptr[offset] |= ((*(p+count+3) >> (30-2*2)) & 0x3) << bit2;
			if(new == 0xc && old == 0x7)
				ptr[offset] |= ((*(p+count+3) >> (30-3*2)) & 0x3) << bit2;
			if(new == 0xc && old == 0x8)
				ptr[offset] |= ((*(p+count+3) >> (30-4*2)) & 0x3) << bit2;
			if(new == 0xc && old == 0xa)
				ptr[offset] |= ((*(p+count+3) >> (30-5*2)) & 0x3) << bit2;
			if(new == 0xc && old == 0xc)
				ptr[offset] |= ((*(p+count+3) >> (30-6*2)) & 0x3) << bit2;
			if(new == 0xc && old == 0xf)
				ptr[offset] |= ((*(p+count+3) >> (30-7*2)) & 0x3) << bit2;

			if(new == 0xf && old == 0)
				ptr[offset] |= ((*(p+count+3) >> (30-8*2)) & 0x3) << bit2;
			if(new == 0xf && old == 0x2)
				ptr[offset] |= ((*(p+count+3) >> (30-9*2)) & 0x3) << bit2;
			if(new == 0xf && old == 0x5)
				ptr[offset] |= ((*(p+count+3) >> (30-10*2)) & 0x3) << bit2;
			if(new == 0xf && old == 0x7)
				ptr[offset] |= ((*(p+count+3) >> (30-11*2)) & 0x3) << bit2;
			if(new == 0xf && old == 0x8)
				ptr[offset] |= ((*(p+count+3) >> (30-12*2)) & 0x3) << bit2;
			if(new == 0xf && old == 0xa)
				ptr[offset] |= ((*(p+count+3) >> (30-13*2)) & 0x3) << bit2;
			if(new == 0xf && old == 0xc)
				ptr[offset] |= ((*(p+count+3) >> (30-14*2)) & 0x3) << bit2;
			if(new == 0xf && old == 0xf)
				ptr[offset] |= ((*(p+count+3) >> (30-15*2)) & 0x3) << bit2;



		}
	}


}

void fill_4level_gray(int max_frm,unsigned int *p)
{

	int i,j,old,new,offset, bit2;
	int index_per_frame = (1 << 8);
	int count; // count is offset of array
	unsigned int *ptr = (unsigned int *)lcd_palette;
	memset(lcd_palette, 0x0, 4096);
	// record the totally multiple of 16frames

	if( max_frm%16 != 0)
		totally_time = (1+max_frm/16);
	else
		totally_time = max_frm/16;


	for (j = 0; j < max_frm; j++) {
		count = j;
		for(i=0;i<index_per_frame;i++){
			old = (i >> 4) & 0xf;
			new = i & 0xf;
			offset = (j * index_per_frame + i)*2/32;
			bit2 = ((j * index_per_frame + i)%16)*2;

			if(new == 0 && old == 0)
				ptr[offset] |= ((*(p+count) >> (30-0*2)) & 0x3) << bit2;
			if(new == 0 && old == 0x5)
				ptr[offset] |= ((*(p+count) >> (30-1*2)) & 0x3) << bit2;
			if(new == 0 && old == 0xa)
				ptr[offset] |= ((*(p+count) >> (30-2*2)) & 0x3) << bit2;
			if(new == 0 && old == 0xf)
				ptr[offset] |= ((*(p+count) >> (30-3*2)) & 0x3) << bit2;

			if(new == 0x5 && old == 0x0)
				ptr[offset] |= ((*(p+count) >> (30-4*2)) & 0x3) << bit2;
			if(new == 0x5 && old == 0x5)
				ptr[offset] |= ((*(p+count) >> (30-5*2)) & 0x3) << bit2;
			if(new == 0x5 && old == 0xa)
				ptr[offset] |= ((*(p+count) >> (30-6*2)) & 0x3) << bit2;
			if(new == 0x5 && old == 0xf)
				ptr[offset] |= ((*(p+count) >> (30-7*2)) & 0x3) << bit2;


			if(new == 0xa && old == 0x0)
				ptr[offset] |= ((*(p+count) >> (30-8*2)) & 0x3) << bit2;
			if(new == 0xa && old == 0x5)
				ptr[offset] |= ((*(p+count) >> (30-9*2)) & 0x3) << bit2;
			if(new == 0xa && old == 0xa)
				ptr[offset] |= ((*(p+count) >> (30-10*2)) & 0x3) << bit2;
			if(new == 0xa && old == 0xf)
				ptr[offset] |= ((*(p+count) >> (30-11*2)) & 0x3) << bit2;


			if(new == 0xf && old == 0x0)
				ptr[offset] |= ((*(p+count) >> (30-12*2)) & 0x3) << bit2;
			if(new == 0xf && old == 0x5)
				ptr[offset] |= ((*(p+count) >> (30-13*2)) & 0x3) << bit2;
			if(new == 0xf && old == 0xa)
				ptr[offset] |= ((*(p+count) >> (30-14*2)) & 0x3) << bit2;
			if(new == 0xf && old == 0xf)
				ptr[offset] |= ((*(p+count) >> (30-15*2)) & 0x3) << bit2;
		}
	}

}



void fill_gu_palette(void)
{

	int max_frm ;
	unsigned int *p;

	get_temp();
	D("%s:  temp(%d) gray level=%d. \n",__func__,epd_temp_level,epd_gray_level);

	if(epd_gray_level == GRAY_LEVEL_4)
	{
		if (epd_temp_level ==  TEMP_LOW_LEVEL)
		{

			max_frm = 48;
			p =  (unsigned int *)&waveform_temp0_gu; // give waveform_temp2_Du to p


		}
		else if (epd_temp_level ==  TEMP_HIGH_LEVEL)
		{

			max_frm = 48;
			p =  (unsigned int *)&waveform_temp1_gu; // give waveform_temp2_Du to p

		}
		else if (epd_temp_level ==  TEMP_HIGHER_LEVEL)
		{

			max_frm = 48;
			p =  (unsigned int *)&waveform_temp2_gu; // give waveform_temp2_Du to p

		}

		else if (epd_temp_level ==  TEMP_HIGHEST_LEVEL)
		{

			max_frm = 42;
			p =  (unsigned int *)&waveform_temp3_gu;

		}
		else{
			printk("Invalid temperature level. \n");
			return;
		}

		fill_4level_gray(max_frm,p);
	}

	if(epd_gray_level == GRAY_LEVEL_8)
	{


		if (epd_temp_level ==  TEMP_LOW_LEVEL)
		{

			max_frm = 48;
			p =  (unsigned int *)&waveform8_temp0_gu; // give waveform_temp2_Du to p


		}
		else if (epd_temp_level ==  TEMP_HIGH_LEVEL)
		{

			max_frm = 48;
			p =  (unsigned int *)&waveform8_temp1_gu; // give waveform_temp2_Du to p

		}
		else if (epd_temp_level ==  TEMP_HIGHER_LEVEL)
		{

			max_frm = 48;
			p =  (unsigned int *)&waveform8_temp2_gu; // give waveform_temp2_Du to p

		}

		else if (epd_temp_level ==  TEMP_HIGHEST_LEVEL)
		{

			max_frm = 42;
			p =  (unsigned int *)&waveform8_temp3_gu;
		}
		else{
			printk("Invalid temperature level. \n");
			return;
		}

		fill_8level_gray(max_frm,p);
	}
	if(epd_gray_level == GRAY_LEVEL_16)
	{
		// add 16level gray support

		if (epd_temp_level ==  TEMP_LOW_LEVEL)
		{

			max_frm = 48;
			p =  (unsigned int *)&waveform16_temp0_gu; // give waveform_temp2_Du to p


		}
		else if (epd_temp_level ==  TEMP_HIGH_LEVEL)
		{

			max_frm = 48;
			p =  (unsigned int *)&waveform16_temp1_gu; // give waveform_temp2_Du to p

		}
		else if (epd_temp_level ==  TEMP_HIGHER_LEVEL)
		{

			max_frm = 48;
			p =  (unsigned int *)&waveform16_temp2_gu; // give waveform_temp2_Du to p

		}

		else if (epd_temp_level ==  TEMP_HIGHEST_LEVEL)
		{

			max_frm = 42;
			p =  (unsigned int *)&waveform16_temp3_gu;
		}
		else{
			printk("Invalid temperature level. \n");
			return;
		}

		fill_16level_gray(max_frm,p);

	}
	dma_cache_wback((unsigned int)(lcd_palette), 4096);
//	printpalette((unsigned short *)((unsigned int)lcd_palette | 0xa0000000), 2048);

}
void fill_gc_palette(void)
{

	int max_frm ;
	unsigned int *p;

	get_temp();
	D("%s:  temp%d gray level=%d. \n",__func__,epd_temp_level,epd_gray_level);

	if(epd_gray_level == GRAY_LEVEL_4)
	{
		if (epd_temp_level ==  TEMP_LOW_LEVEL)
		{

			max_frm = 48;
			p =  (unsigned int *)&waveform_temp0_gc; // give waveform_temp2_Du to p


		}
		else if (epd_temp_level ==  TEMP_HIGH_LEVEL)
		{

			max_frm = 48;
			p =  (unsigned int *)&waveform_temp1_gc; // give waveform_temp2_Du to p

		}
		else if (epd_temp_level ==  TEMP_HIGHER_LEVEL)
		{

			max_frm = 48;
			p =  (unsigned int *)&waveform_temp2_gc; // give waveform_temp2_Du to p

		}

		else if (epd_temp_level ==  TEMP_HIGHEST_LEVEL)
		{

			max_frm = 42;
			p =  (unsigned int *)&waveform_temp3_gc;


		}
		else{
			printk("Invalid temperature level. \n");
			return;
		}

		fill_4level_gray(max_frm,p);
	}

	if(epd_gray_level == GRAY_LEVEL_8)
	{

		if (epd_temp_level ==  TEMP_LOW_LEVEL)
		{

			max_frm = 48;
			p =  (unsigned int *)&waveform8_temp0_gc; // give waveform_temp2_Du to p


		}
		else if (epd_temp_level ==  TEMP_HIGH_LEVEL)
		{

			max_frm = 48;
			p =  (unsigned int *)&waveform8_temp1_gc; // give waveform_temp2_Du to p

		}
		else if (epd_temp_level ==  TEMP_HIGHER_LEVEL)
		{

			max_frm = 48;
			p =  (unsigned int *)&waveform8_temp2_gc; // give waveform_temp2_Du to p

		}

		else if (epd_temp_level ==  TEMP_HIGHEST_LEVEL)
		{

			max_frm = 42;
			p =  (unsigned int *)&waveform8_temp3_gc;
		}
		else{
			printk("Invalid temperature level. \n");
			return;
		}

		fill_8level_gray(max_frm,p);
	}
	if(epd_gray_level == GRAY_LEVEL_16)
	{
		// add 16level gray support

		if (epd_temp_level ==  TEMP_LOW_LEVEL)
		{

			max_frm = 48;
			p =  (unsigned int *)&waveform16_temp0_gc; // give waveform_temp2_Du to p


		}
		else if (epd_temp_level ==  TEMP_HIGH_LEVEL)
		{

			max_frm = 48;
			p =  (unsigned int *)&waveform16_temp1_gc; // give waveform_temp2_Du to p

		}
		else if (epd_temp_level ==  TEMP_HIGHER_LEVEL)
		{

			max_frm = 48;
			p =  (unsigned int *)&waveform16_temp2_gc; // give waveform_temp2_Du to p

		}

		else if (epd_temp_level ==  TEMP_HIGHEST_LEVEL)
		{

			max_frm = 42;
			p =  (unsigned int *)&waveform16_temp3_gc;
		}
		else{
			printk("Invalid temperature level. \n");
			return;
		}

		fill_16level_gray(max_frm,p);

	}
	dma_cache_wback((unsigned int)(lcd_palette), 4096);
//	printpalette((unsigned short *)((unsigned int)lcd_palette | 0xa0000000), 2048);


}

void handwriting_palette(void)
{

	int max_frm ;
	unsigned int *p;
	D("%s:  temp(%d) gray level=%d. \n",__func__,epd_temp_level,epd_gray_level);
	max_frm = 2;
	p =  (unsigned int *)&waveform_handwriting;
	fill_du_4level_gray(max_frm,p);
	dma_cache_wback((unsigned int)(lcd_palette), 4096);
//	printpalette((unsigned short *)((unsigned int)lcd_palette | 0xa0000000), 64);

}

int set_epd_mod(unsigned long arg)
{

	epd_mod_level  = *(unsigned long *)arg;

	if (epd_mod_level == EPD_MOD_INIT )
		fill_init_palette();
	else if (epd_mod_level == EPD_MOD_DU )
		fill_du_palette();
	else if (epd_mod_level == EPD_MOD_GU )
		fill_gu_palette();
	else if (epd_mod_level == EPD_MOD_GC )
		fill_gc_palette();
	else
		return MOD_INVALID;
	return EPD_SUCCESS;


}
