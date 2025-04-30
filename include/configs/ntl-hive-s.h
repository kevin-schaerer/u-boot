/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 * Copyright (c) 2025 Kevin Schaerer <kevin.schaerer@nettimelogic.com>
 */

#ifndef __NTL_HIVE_S_CONFIG_H
#define __NTL_HIVE_S_CONFIG_H

#include <linux/sizes.h>

/* Environment options */
#define CFG_SYS_SDRAM_BASE 0x40000000

/* ---------------------------------------------------------------------
 * Board boot configuration
 */

#define CFG_EXTRA_ENV_SETTINGS \
	"fdt_addr_r=0x40ef0000\0" \
	"kernel_addr_r=0x01480000\0" \
	"image=boot/Image.gz\0"

#endif/* __CONFIG_H */
