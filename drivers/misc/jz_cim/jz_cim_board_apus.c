

#include <asm/jzsoc.h>

#define GPIO_CAMERA_RST	(32*4+8) /*GPE8 mclk*/

void cim_power_off(void)
{
	__camera_power_off();
}

void cim_power_on(void)
{
	__camera_power_on();
}
