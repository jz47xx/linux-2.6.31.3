//------------------------------------------------------------------------------
// <copyright file="target_reg_table.h" company="Atheros">
//    Copyright (c) 2004-2010 Atheros Corporation.  All rights reserved.
//
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
//
//------------------------------------------------------------------------------
//==============================================================================
// Target register table macros and structure definitions
//
// Author(s): ="Atheros"
//==============================================================================

#ifndef TARGET_REG_TABLE_H_
#define TARGET_REG_TABLE_H_

#include "targaddrs.h"

/*** WARNING : Add to the end of the TABLE! do not change the order ****/
typedef struct targetdef_s {
    A_UINT32 d_RTC_BASE_ADDRESS;
    A_UINT32 d_SYSTEM_SLEEP_OFFSET;
    A_UINT32 d_SYSTEM_SLEEP_DISABLE_LSB;
    A_UINT32 d_SYSTEM_SLEEP_DISABLE_MASK;
    A_UINT32 d_CLOCK_CONTROL_OFFSET;
    A_UINT32 d_CLOCK_CONTROL_SI0_CLK_MASK;
    A_UINT32 d_RESET_CONTROL_OFFSET;
    A_UINT32 d_RESET_CONTROL_SI0_RST_MASK;
    A_UINT32 d_GPIO_BASE_ADDRESS;
    A_UINT32 d_GPIO_PIN0_OFFSET;
    A_UINT32 d_GPIO_PIN1_OFFSET;
    A_UINT32 d_GPIO_PIN0_CONFIG_MASK;
    A_UINT32 d_GPIO_PIN1_CONFIG_MASK;
    A_UINT32 d_SI_CONFIG_BIDIR_OD_DATA_LSB;
    A_UINT32 d_SI_CONFIG_BIDIR_OD_DATA_MASK;
    A_UINT32 d_SI_CONFIG_I2C_LSB;
    A_UINT32 d_SI_CONFIG_I2C_MASK;
    A_UINT32 d_SI_CONFIG_POS_SAMPLE_LSB;
    A_UINT32 d_SI_CONFIG_POS_SAMPLE_MASK;
    A_UINT32 d_SI_CONFIG_INACTIVE_CLK_LSB;
    A_UINT32 d_SI_CONFIG_INACTIVE_CLK_MASK;
    A_UINT32 d_SI_CONFIG_INACTIVE_DATA_LSB;
    A_UINT32 d_SI_CONFIG_INACTIVE_DATA_MASK;
    A_UINT32 d_SI_CONFIG_DIVIDER_LSB;
    A_UINT32 d_SI_CONFIG_DIVIDER_MASK;
    A_UINT32 d_SI_BASE_ADDRESS;
    A_UINT32 d_SI_CONFIG_OFFSET;
    A_UINT32 d_SI_TX_DATA0_OFFSET;
    A_UINT32 d_SI_TX_DATA1_OFFSET;
    A_UINT32 d_SI_RX_DATA0_OFFSET;
    A_UINT32 d_SI_RX_DATA1_OFFSET;
    A_UINT32 d_SI_CS_OFFSET;
    A_UINT32 d_SI_CS_DONE_ERR_MASK;
    A_UINT32 d_SI_CS_DONE_INT_MASK;
    A_UINT32 d_SI_CS_START_LSB;
    A_UINT32 d_SI_CS_START_MASK;
    A_UINT32 d_SI_CS_RX_CNT_LSB;
    A_UINT32 d_SI_CS_RX_CNT_MASK;
    A_UINT32 d_SI_CS_TX_CNT_LSB;
    A_UINT32 d_SI_CS_TX_CNT_MASK;
    A_UINT32 d_BOARD_DATA_SZ;
    A_UINT32 d_BOARD_EXT_DATA_SZ;
} TARGET_REGISTER_TABLE;

#define BOARD_DATA_SZ_MAX 2048

#if defined(MY_TARGET_DEF) /* { */

#ifdef ATH_REG_TABLE_DIRECT_ASSIGN

static struct targetdef_s my_target_def = {
    RTC_BASE_ADDRESS,
    SYSTEM_SLEEP_OFFSET,
    SYSTEM_SLEEP_DISABLE_LSB,
    SYSTEM_SLEEP_DISABLE_MASK,
    CLOCK_CONTROL_OFFSET,
    CLOCK_CONTROL_SI0_CLK_MASK,
    RESET_CONTROL_OFFSET,
    RESET_CONTROL_SI0_RST_MASK,
    GPIO_BASE_ADDRESS,
    GPIO_PIN0_OFFSET,
    GPIO_PIN0_CONFIG_MASK,
    GPIO_PIN1_OFFSET,
    GPIO_PIN1_CONFIG_MASK,
    SI_CONFIG_BIDIR_OD_DATA_LSB,
    SI_CONFIG_BIDIR_OD_DATA_MASK,
    SI_CONFIG_I2C_LSB,
    SI_CONFIG_I2C_MASK,
    SI_CONFIG_POS_SAMPLE_LSB,
    SI_CONFIG_POS_SAMPLE_MASK,
    SI_CONFIG_INACTIVE_CLK_LSB,
    SI_CONFIG_INACTIVE_CLK_MASK,
    SI_CONFIG_INACTIVE_DATA_LSB,
    SI_CONFIG_INACTIVE_DATA_MASK,
    SI_CONFIG_DIVIDER_LSB,
    SI_CONFIG_DIVIDER_MASK,
    SI_BASE_ADDRESS,
    SI_CONFIG_OFFSET,
    SI_TX_DATA0_OFFSET,
    SI_TX_DATA1_OFFSET,
    SI_RX_DATA0_OFFSET,
    SI_RX_DATA1_OFFSET,
    SI_CS_OFFSET,
    SI_CS_DONE_ERR_MASK,
    SI_CS_DONE_INT_MASK,
    SI_CS_START_LSB,
    SI_CS_START_MASK,
    SI_CS_RX_CNT_LSB,
    SI_CS_RX_CNT_MASK,
    SI_CS_TX_CNT_LSB,
    SI_CS_TX_CNT_MASK,
    MY_TARGET_BOARD_DATA_SZ,
    MY_TARGET_BOARD_EXT_DATA_SZ,
};

#else

static struct targetdef_s my_target_def = {
    .d_RTC_BASE_ADDRESS = RTC_BASE_ADDRESS,
    .d_SYSTEM_SLEEP_OFFSET = SYSTEM_SLEEP_OFFSET,
    .d_SYSTEM_SLEEP_DISABLE_LSB = SYSTEM_SLEEP_DISABLE_LSB,
    .d_SYSTEM_SLEEP_DISABLE_MASK = SYSTEM_SLEEP_DISABLE_MASK,
    .d_CLOCK_CONTROL_OFFSET = CLOCK_CONTROL_OFFSET,
    .d_CLOCK_CONTROL_SI0_CLK_MASK = CLOCK_CONTROL_SI0_CLK_MASK,
    .d_RESET_CONTROL_OFFSET = RESET_CONTROL_OFFSET,
    .d_RESET_CONTROL_SI0_RST_MASK = RESET_CONTROL_SI0_RST_MASK,
    .d_GPIO_BASE_ADDRESS = GPIO_BASE_ADDRESS,
    .d_GPIO_PIN0_OFFSET = GPIO_PIN0_OFFSET,
    .d_GPIO_PIN0_CONFIG_MASK = GPIO_PIN0_CONFIG_MASK,
    .d_GPIO_PIN1_OFFSET = GPIO_PIN1_OFFSET,
    .d_GPIO_PIN1_CONFIG_MASK = GPIO_PIN1_CONFIG_MASK,
    .d_SI_CONFIG_BIDIR_OD_DATA_LSB = SI_CONFIG_BIDIR_OD_DATA_LSB,
    .d_SI_CONFIG_BIDIR_OD_DATA_MASK = SI_CONFIG_BIDIR_OD_DATA_MASK,
    .d_SI_CONFIG_I2C_LSB = SI_CONFIG_I2C_LSB,
    .d_SI_CONFIG_I2C_MASK = SI_CONFIG_I2C_MASK,
    .d_SI_CONFIG_POS_SAMPLE_LSB = SI_CONFIG_POS_SAMPLE_LSB,
    .d_SI_CONFIG_POS_SAMPLE_MASK = SI_CONFIG_POS_SAMPLE_MASK,
    .d_SI_CONFIG_INACTIVE_CLK_LSB = SI_CONFIG_INACTIVE_CLK_LSB,
    .d_SI_CONFIG_INACTIVE_CLK_MASK = SI_CONFIG_INACTIVE_CLK_MASK,
    .d_SI_CONFIG_INACTIVE_DATA_LSB = SI_CONFIG_INACTIVE_DATA_LSB,
    .d_SI_CONFIG_INACTIVE_DATA_MASK = SI_CONFIG_INACTIVE_DATA_MASK,
    .d_SI_CONFIG_DIVIDER_LSB = SI_CONFIG_DIVIDER_LSB,
    .d_SI_CONFIG_DIVIDER_MASK = SI_CONFIG_DIVIDER_MASK,
    .d_SI_BASE_ADDRESS = SI_BASE_ADDRESS,
    .d_SI_CONFIG_OFFSET = SI_CONFIG_OFFSET,
    .d_SI_TX_DATA0_OFFSET = SI_TX_DATA0_OFFSET,
    .d_SI_TX_DATA1_OFFSET = SI_TX_DATA1_OFFSET,
    .d_SI_RX_DATA0_OFFSET = SI_RX_DATA0_OFFSET,
    .d_SI_RX_DATA1_OFFSET = SI_RX_DATA1_OFFSET,
    .d_SI_CS_OFFSET = SI_CS_OFFSET,
    .d_SI_CS_DONE_ERR_MASK = SI_CS_DONE_ERR_MASK,
    .d_SI_CS_DONE_INT_MASK = SI_CS_DONE_INT_MASK,
    .d_SI_CS_START_LSB = SI_CS_START_LSB,
    .d_SI_CS_START_MASK = SI_CS_START_MASK,
    .d_SI_CS_RX_CNT_LSB = SI_CS_RX_CNT_LSB,
    .d_SI_CS_RX_CNT_MASK = SI_CS_RX_CNT_MASK,
    .d_SI_CS_TX_CNT_LSB = SI_CS_TX_CNT_LSB,
    .d_SI_CS_TX_CNT_MASK = SI_CS_TX_CNT_MASK,
    .d_BOARD_DATA_SZ = MY_TARGET_BOARD_DATA_SZ,
    .d_BOARD_EXT_DATA_SZ = MY_TARGET_BOARD_EXT_DATA_SZ,
};

#endif

#if MY_TARGET_BOARD_DATA_SZ > BOARD_DATA_SZ_MAX
#error "BOARD_DATA_SZ_MAX is too small"
#endif

struct targetdef_s *MY_TARGET_DEF = &my_target_def;

#else /* } { */

#define RTC_BASE_ADDRESS (targetdef->d_RTC_BASE_ADDRESS)
#define SYSTEM_SLEEP_OFFSET (targetdef->d_SYSTEM_SLEEP_OFFSET)
#define SYSTEM_SLEEP_DISABLE_LSB (targetdef->d_SYSTEM_SLEEP_DISABLE_LSB)
#define SYSTEM_SLEEP_DISABLE_MASK (targetdef->d_SYSTEM_SLEEP_DISABLE_MASK)
#define CLOCK_CONTROL_OFFSET (targetdef->d_CLOCK_CONTROL_OFFSET)
#define CLOCK_CONTROL_SI0_CLK_MASK (targetdef->d_CLOCK_CONTROL_SI0_CLK_MASK)
#define RESET_CONTROL_OFFSET (targetdef->d_RESET_CONTROL_OFFSET)
#define RESET_CONTROL_SI0_RST_MASK (targetdef->d_RESET_CONTROL_SI0_RST_MASK)
#define GPIO_BASE_ADDRESS (targetdef->d_GPIO_BASE_ADDRESS)
#define GPIO_PIN0_OFFSET (targetdef->d_GPIO_PIN0_OFFSET)
#define GPIO_PIN0_CONFIG_MASK (targetdef->d_GPIO_PIN0_CONFIG_MASK)
#define GPIO_PIN1_OFFSET (targetdef->d_GPIO_PIN1_OFFSET)
#define GPIO_PIN1_CONFIG_MASK (targetdef->d_GPIO_PIN1_CONFIG_MASK)
#define SI_CONFIG_BIDIR_OD_DATA_LSB (targetdef->d_SI_CONFIG_BIDIR_OD_DATA_LSB)
#define SI_CONFIG_BIDIR_OD_DATA_MASK (targetdef->d_SI_CONFIG_BIDIR_OD_DATA_MASK)
#define SI_CONFIG_I2C_LSB (targetdef->d_SI_CONFIG_I2C_LSB)
#define SI_CONFIG_I2C_MASK (targetdef->d_SI_CONFIG_I2C_MASK)
#define SI_CONFIG_POS_SAMPLE_LSB (targetdef->d_SI_CONFIG_POS_SAMPLE_LSB)
#define SI_CONFIG_POS_SAMPLE_MASK (targetdef->d_SI_CONFIG_POS_SAMPLE_MASK)
#define SI_CONFIG_INACTIVE_CLK_LSB (targetdef->d_SI_CONFIG_INACTIVE_CLK_LSB)
#define SI_CONFIG_INACTIVE_CLK_MASK (targetdef->d_SI_CONFIG_INACTIVE_CLK_MASK)
#define SI_CONFIG_INACTIVE_DATA_LSB (targetdef->d_SI_CONFIG_INACTIVE_DATA_LSB)
#define SI_CONFIG_INACTIVE_DATA_MASK (targetdef->d_SI_CONFIG_INACTIVE_DATA_MASK)
#define SI_CONFIG_DIVIDER_LSB (targetdef->d_SI_CONFIG_DIVIDER_LSB)
#define SI_CONFIG_DIVIDER_MASK (targetdef->d_SI_CONFIG_DIVIDER_MASK)
#define SI_BASE_ADDRESS (targetdef->d_SI_BASE_ADDRESS)
#define SI_CONFIG_OFFSET (targetdef->d_SI_CONFIG_OFFSET)
#define SI_TX_DATA0_OFFSET (targetdef->d_SI_TX_DATA0_OFFSET)
#define SI_TX_DATA1_OFFSET (targetdef->d_SI_TX_DATA1_OFFSET)
#define SI_RX_DATA0_OFFSET (targetdef->d_SI_RX_DATA0_OFFSET)
#define SI_RX_DATA1_OFFSET (targetdef->d_SI_RX_DATA1_OFFSET)
#define SI_CS_OFFSET (targetdef->d_SI_CS_OFFSET)
#define SI_CS_DONE_ERR_MASK (targetdef->d_SI_CS_DONE_ERR_MASK)
#define SI_CS_DONE_INT_MASK (targetdef->d_SI_CS_DONE_INT_MASK)
#define SI_CS_START_LSB (targetdef->d_SI_CS_START_LSB)
#define SI_CS_START_MASK (targetdef->d_SI_CS_START_MASK)
#define SI_CS_RX_CNT_LSB (targetdef->d_SI_CS_RX_CNT_LSB)
#define SI_CS_RX_CNT_MASK (targetdef->d_SI_CS_RX_CNT_MASK)
#define SI_CS_TX_CNT_LSB (targetdef->d_SI_CS_TX_CNT_LSB)
#define SI_CS_TX_CNT_MASK (targetdef->d_SI_CS_TX_CNT_MASK)
#define EEPROM_SZ (targetdef->d_BOARD_DATA_SZ)
#define EEPROM_EXT_SZ (targetdef->d_BOARD_EXT_DATA_SZ)

/* SET macros */
#define SYSTEM_SLEEP_DISABLE_SET(x)              (((x) << SYSTEM_SLEEP_DISABLE_LSB) & SYSTEM_SLEEP_DISABLE_MASK)
#define SI_CONFIG_BIDIR_OD_DATA_SET(x) (((x) << SI_CONFIG_BIDIR_OD_DATA_LSB) & SI_CONFIG_BIDIR_OD_DATA_MASK)
#define SI_CONFIG_I2C_SET(x) (((x) << SI_CONFIG_I2C_LSB) & SI_CONFIG_I2C_MASK)
#define SI_CONFIG_POS_SAMPLE_SET(x) (((x) << SI_CONFIG_POS_SAMPLE_LSB) & SI_CONFIG_POS_SAMPLE_MASK)
#define SI_CONFIG_INACTIVE_CLK_SET(x) (((x) << SI_CONFIG_INACTIVE_CLK_LSB) & SI_CONFIG_INACTIVE_CLK_MASK)
#define SI_CONFIG_INACTIVE_DATA_SET(x) (((x) << SI_CONFIG_INACTIVE_DATA_LSB) & SI_CONFIG_INACTIVE_DATA_MASK)
#define SI_CONFIG_DIVIDER_SET(x) (((x) << SI_CONFIG_DIVIDER_LSB) & SI_CONFIG_DIVIDER_MASK)
#define SI_CS_START_SET(x) (((x) << SI_CS_START_LSB) & SI_CS_START_MASK)
#define SI_CS_RX_CNT_SET(x) (((x) << SI_CS_RX_CNT_LSB) & SI_CS_RX_CNT_MASK)
#define SI_CS_TX_CNT_SET(x) (((x) << SI_CS_TX_CNT_LSB) & SI_CS_TX_CNT_MASK)

#endif /* } */

#endif /*TARGET_REG_TABLE_H_*/
