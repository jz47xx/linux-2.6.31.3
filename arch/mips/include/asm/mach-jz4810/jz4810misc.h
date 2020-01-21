/*
 * linux/include/asm-mips/mach-jz4810/jz4810misc.h
 *
 * JZ4810 misc definition.
 *
 * Copyright (C) 2010 Ingenic Semiconductor Co., Ltd.
 */

#ifndef __JZ4810MISC_H__
#define __JZ4810MISC_H__


#if defined(__ASSEMBLY__) || defined(__LANGUAGE_ASSEMBLY)
        #ifndef __MIPS_ASSEMBLER
                #define __MIPS_ASSEMBLER
        #endif
        #define REG8(addr)	(addr)
        #define REG16(addr)	(addr)
        #define REG32(addr)	(addr)
#else
        #define REG8(addr)	*((volatile unsigned char *)(addr))
        #define REG16(addr)	*((volatile unsigned short *)(addr))
        #define REG32(addr)	*((volatile unsigned int *)(addr))
#endif


#endif /* __JZ4810MISC_H__ */
