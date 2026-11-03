/***************************************************************************//**
 *   @file   adbms6830b.h
 *   @brief  Header file for the ADBMS6830B Multicell Battery Monitor Driver.
 *   @author Embedded Systems & BMS Firmware Architecture Team
********************************************************************************
 * Copyright 2026(c) Analog Devices, Inc. / Formula Student Team
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/

#ifndef _ADBMS6830B_H_
#define _ADBMS6830B_H_

#include <stdint.h>
#include <stdbool.h>
#include "no_os_error.h"
#include "no_os_util.h"
#include "no_os_spi.h"
#include "no_os_gpio.h"

/* Stack capacity and frame sizing constraints */
#define ADBMS6830B_MAX_ICS                  11
#define ADBMS6830B_REG_DATA_SIZE            6
#define ADBMS6830B_PEC_SIZE                 2
#define ADBMS6830B_CMD_SIZE                 4
#define ADBMS6830B_NODE_FRAME_SIZE          (ADBMS6830B_REG_DATA_SIZE + ADBMS6830B_PEC_SIZE) // 8 Bytes
#define ADBMS6830B_MAX_FRAME_SIZE           (ADBMS6830B_CMD_SIZE + (ADBMS6830B_MAX_ICS * ADBMS6830B_NODE_FRAME_SIZE)) // 92 Bytes

/* ADBMS6830B Command Codes (11-bit Command Code mapped to 16-bit word, Table 50) */
#define ADBMS6830B_CMD_WRCFGA               0x0001
#define ADBMS6830B_CMD_WRCFGB               0x0024
#define ADBMS6830B_CMD_RDCFGA               0x0002
#define ADBMS6830B_CMD_RDCFGB               0x0026

#define ADBMS6830B_CMD_RDCVA                0x0004
#define ADBMS6830B_CMD_RDCVB                0x0006
#define ADBMS6830B_CMD_RDCVC                0x0008
#define ADBMS6830B_CMD_RDCVD                0x000A
#define ADBMS6830B_CMD_RDCVE                0x0009
#define ADBMS6830B_CMD_RDCVF                0x000B

#define ADBMS6830B_CMD_RDSVA                0x0003
#define ADBMS6830B_CMD_RDSVB                0x0005
#define ADBMS6830B_CMD_RDSVC                0x0007
#define ADBMS6830B_CMD_RDSVD                0x000D
#define ADBMS6830B_CMD_RDSVE                0x000E
#define ADBMS6830B_CMD_RDSVF                0x000F

#define ADBMS6830B_CMD_RDAUXA               0x0019
#define ADBMS6830B_CMD_RDAUXB               0x001A
#define ADBMS6830B_CMD_RDAUXC               0x001B
#define ADBMS6830B_CMD_RDAUXD               0x001F

#define ADBMS6830B_CMD_RDSTATA              0x0030
#define ADBMS6830B_CMD_RDSTATB              0x0031
#define ADBMS6830B_CMD_RDSTATC              0x0032

#define ADBMS6830B_CMD_ADCV_BASE            0x0260
#define ADBMS6830B_CMD_ADSV_BASE            0x0168
#define ADBMS6830B_CMD_SNAP                 0x002D
#define ADBMS6830B_CMD_UNSNAP               0x002F
#define ADBMS6830B_CMD_SRST                 0x0027
#define ADBMS6830B_CMD_RSTCC                0x002E

/* Configuration Register Group A Bitfield Definitions (Table 55 & 102) */
#define ADBMS6830B_CFGAR0_REFON             NO_OS_BIT(7)
#define ADBMS6830B_CFGAR0_CTH_MASK          NO_OS_GENMASK(2, 0)
#define ADBMS6830B_CFGAR0_CTH_SET(val)      no_os_field_prep(ADBMS6830B_CFGAR0_CTH_MASK, val)

#define ADBMS6830B_CFGAR5_FC_MASK           NO_OS_GENMASK(2, 0)
#define ADBMS6830B_CFGAR5_FC_SET(val)       no_os_field_prep(ADBMS6830B_CFGAR5_FC_MASK, val)
#define ADBMS6830B_CFGAR5_COMM_BK           NO_OS_BIT(3)

/* Filter Corner Frequency Enum (Table 21) */
enum adbms6830b_fc {
	ADBMS6830B_FC_DISABLED = 0,
	ADBMS6830B_FC_110HZ    = 1,
	ADBMS6830B_FC_45HZ     = 2,
	ADBMS6830B_FC_21HZ     = 3,
	ADBMS6830B_FC_10HZ     = 4,
	ADBMS6830B_FC_5HZ      = 5,
	ADBMS6830B_FC_1_25HZ   = 6,
	ADBMS6830B_FC_0_625HZ  = 7,
};

/* Comparison Threshold Enum for CTH[2:0] (Table 102) */
enum adbms6830b_cth {
	ADBMS6830B_CTH_5_1MV  = 0,
	ADBMS6830B_CTH_8_1MV  = 1, /* Default */
	ADBMS6830B_CTH_9_0MV  = 2,
	ADBMS6830B_CTH_10_0MV = 3,
	ADBMS6830B_CTH_15_0MV = 4,
	ADBMS6830B_CTH_20_0MV = 5,
	ADBMS6830B_CTH_25_0MV = 6,
	ADBMS6830B_CTH_40_0MV = 7,
};

/* Individual Node Physical State Representation */
struct adbms6830b_node {
	uint16_t cell_raw[16];          /* Raw 16-bit signed codes */
	int32_t  cell_mv[16];           /* Calibrated voltage in millivolts */
	uint16_t s_volt_raw[16];        /* Redundant S-ADC raw codes */
	int32_t  s_volt_mv[16];         /* Redundant S-ADC millivolts */
	uint16_t aux_raw[10];           /* GPIO raw measurement */
	uint8_t  tx_cfga[ADBMS6830B_REG_DATA_SIZE];
	uint8_t  rx_cfga[ADBMS6830B_REG_DATA_SIZE];
	uint8_t  tx_cfgb[ADBMS6830B_REG_DATA_SIZE];
	uint8_t  rx_cfgb[ADBMS6830B_REG_DATA_SIZE];
	uint8_t  last_ccnt;             /* Received 6-bit Command Counter */
	uint32_t pec_errors;            /* Cumulative PEC10 mismatch counter */
};

/* Driver Initialization Parameters (Platform & Topology Binding) */
struct adbms6830b_init_param {
	struct no_os_spi_init_param *spi_init;
	uint8_t num_ics;                /* Active nodes (1 for bench, 11 for car) */
	enum adbms6830b_fc filter_corner;
	enum adbms6830b_cth comp_threshold;
};

/* Driver Instance Descriptor (Zero Runtime Dynamic Allocation) */
struct adbms6830b_dev {
	struct no_os_spi_desc *spi_desc;
	uint8_t num_ics;
	uint8_t expected_ccnt;          /* Host Command Counter tracker [0..63] */
	struct adbms6830b_node nodes[ADBMS6830B_MAX_ICS];

	/* Static DMA-safe physical transfer buffers */
	uint8_t tx_frame[ADBMS6830B_MAX_FRAME_SIZE];
	uint8_t rx_frame[ADBMS6830B_MAX_FRAME_SIZE];
};

/* Core Public API Prototypes */
uint16_t adbms6830b_calc_pec15(uint8_t len, const uint8_t *data);
uint16_t adbms6830b_calc_pec10(uint8_t len, const uint8_t *data, bool is_rx, uint8_t ccnt_byte);

int32_t adbms6830b_init(struct adbms6830b_dev *dev,
			struct adbms6830b_init_param *init_param);

int32_t adbms6830b_remove(struct adbms6830b_dev *dev);

int32_t adbms6830b_wakeup_chain(struct adbms6830b_dev *dev);

int32_t adbms6830b_send_cmd(struct adbms6830b_dev *dev,
			    uint16_t cmd_code,
			    bool inc_expected_ccnt);

int32_t adbms6830b_write_reg_group(struct adbms6830b_dev *dev,
				   uint16_t write_cmd,
				   uint8_t (*data_in)[ADBMS6830B_REG_DATA_SIZE]);

int32_t adbms6830b_read_reg_group(struct adbms6830b_dev *dev,
				  uint16_t read_cmd,
				  uint8_t (*data_out)[ADBMS6830B_REG_DATA_SIZE]);

int32_t adbms6830b_read_cell_voltages(struct adbms6830b_dev *dev);

#endif /* _ADBMS6830B_H_ */
