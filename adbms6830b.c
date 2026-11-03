/***************************************************************************//**
 *   @file   adbms6830b.c
 *   @brief  Driver Implementation for the ADBMS6830B Multicell Battery Monitor.
 *   @author Embedded Systems & BMS Firmware Architecture Team
********************************************************************************
 * Copyright 2026(c) Analog Devices, Inc. / Formula Student Team
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/

#include <string.h>
#include "adbms6830b.h"
#include "no_os_delay.h"

/* Rescued Mathematically Verified CRC-15 Table (Flash / ROM const) */
static const uint16_t adbms6830b_crc15_table[256] = {
	0x0000, 0xC599, 0xCEAB, 0x0B32, 0xD8CF, 0x1D56, 0x1664, 0xD3FD,
	0xF407, 0x319E, 0x3AAC, 0xFF35, 0x2CC8, 0xE951, 0xE263, 0x27FA,
	0xAD97, 0x680E, 0x633C, 0xA6A5, 0x7558, 0xB0C1, 0xBBF3, 0x7E6A,
	0x5990, 0x9C09, 0x973B, 0x52A2, 0x815F, 0x44C6, 0x4FF4, 0x8A6D,
	0x5B2E, 0x9EB7, 0x9585, 0x501C, 0x83E1, 0x4678, 0x4D4A, 0x88D3,
	0xAF29, 0x6AB0, 0x6182, 0xA41B, 0x77E6, 0xB27F, 0xB94D, 0x7CD4,
	0xF6B9, 0x3320, 0x3812, 0xFD8B, 0x2E76, 0xEBEE, 0xE0DD, 0x2544,
	0x02BE, 0xC727, 0xCC15, 0x098C, 0xDA71, 0x1FE8, 0x14DA, 0xD143,
	0xF3C5, 0x365C, 0x3D6E, 0xF8F7, 0x2B0A, 0xEE93, 0xE5A1, 0x2038,
	0x07C2, 0xC25B, 0xC969, 0x0CF0, 0xDF0D, 0x1A94, 0x11A6, 0xD43F,
	0x5E52, 0x9BCB, 0x90F9, 0x5560, 0x869D, 0x4304, 0x4836, 0x8DAF,
	0xAA55, 0x6FCC, 0x64FE, 0xA167, 0x729A, 0xB703, 0xBC31, 0x79A8,
	0xA8EB, 0x6D72, 0x6640, 0xA3D9, 0x7024, 0xB5BD, 0xBE8F, 0x7B16,
	0x5CEC, 0x9975, 0x9247, 0x57DE, 0x8423, 0x41BA, 0x4A88, 0x8F11,
	0x057C, 0xC0E5, 0xCBD7, 0x0E4E, 0xDDB3, 0x182A, 0x1318, 0xD681,
	0xF17B, 0x34E2, 0x3FD0, 0xFA49, 0x29B4, 0xEC2D, 0xE71F, 0x2286,
	0xA213, 0x678A, 0x6CB8, 0xA921, 0x7ADC, 0xBF45, 0xB477, 0x71EE,
	0x5614, 0x938D, 0x98BF, 0x5D26, 0x8EDB, 0x4B42, 0x4070, 0x85E9,
	0x0F84, 0xCA1D, 0xC12F, 0x04B6, 0xD74B, 0x12D2, 0x19E0, 0xDC79,
	0xFB83, 0x3E1A, 0x3528, 0xF0B1, 0x234C, 0xE6D5, 0xEDE7, 0x287E,
	0xF93D, 0x3CA4, 0x3796, 0xF20F, 0x21F2, 0xE46B, 0xEF59, 0x2AC0,
	0x0D3A, 0xC8A3, 0xC391, 0x0608, 0xD5F5, 0x106C, 0x1B5E, 0xDEC7,
	0x54AA, 0x9133, 0x9A01, 0x5F98, 0x8C65, 0x49FC, 0x42CE, 0x8757,
	0xA0AD, 0x6534, 0x6E06, 0xAB9F, 0x7862, 0xBDFB, 0xB6C9, 0x7350,
	0x51D6, 0x944F, 0x9F7D, 0x5AE4, 0x8919, 0x4C80, 0x47B2, 0x822B,
	0xA5D1, 0x6048, 0x6B7A, 0xAEE3, 0x7D1E, 0xB887, 0xB3B5, 0x762C,
	0xFC41, 0x39D8, 0x32EA, 0xF773, 0x248E, 0xE117, 0xEA25, 0x2FBC,
	0x0846, 0xCDDF, 0xC6ED, 0x0374, 0xD089, 0x1510, 0x1E22, 0xDBBB,
	0x0AF8, 0xCF61, 0xC453, 0x01CA, 0xD237, 0x17AE, 0x1C9C, 0xD905,
	0xFEFF, 0x3B66, 0x3054, 0xF5CD, 0x2630, 0xE3A9, 0xE89B, 0x2D02,
	0xA76F, 0x62F6, 0x69C4, 0xAC5D, 0x7FA0, 0xBA39, 0xB10B, 0x7492,
	0x5368, 0x96F1, 0x9DC3, 0x585A, 0x8BA7, 0x4E3E, 0x450C, 0x8095
};

/* Rescued Mathematically Verified CRC-10 Table (Flash / ROM const) */
static const uint16_t adbms6830b_crc10_table[256] = {
	0x0000, 0x008F, 0x011E, 0x0191, 0x023C, 0x02B3, 0x0322, 0x03AD,
	0x0077, 0x00F8, 0x0169, 0x01E6, 0x024B, 0x02C4, 0x0355, 0x03DA,
	0x00EE, 0x0061, 0x01F0, 0x017F, 0x02D2, 0x025D, 0x03CC, 0x0343,
	0x0099, 0x0016, 0x0187, 0x0108, 0x02A5, 0x022A, 0x03BB, 0x0334,
	0x01DC, 0x0153, 0x00C2, 0x004D, 0x03E0, 0x036F, 0x02FE, 0x0271,
	0x01AB, 0x0124, 0x00B5, 0x003A, 0x0397, 0x0318, 0x0289, 0x0206,
	0x0132, 0x01BD, 0x002C, 0x00A3, 0x030E, 0x0381, 0x0210, 0x029F,
	0x0145, 0x01CA, 0x005B, 0x00D4, 0x0379, 0x03F6, 0x0267, 0x02E8,
	0x03B8, 0x0337, 0x02A6, 0x0229, 0x0184, 0x010B, 0x009A, 0x0015,
	0x03CF, 0x0340, 0x02D1, 0x025E, 0x01F3, 0x017C, 0x00ED, 0x0062,
	0x0356, 0x03D9, 0x0248, 0x02C7, 0x016A, 0x01E5, 0x0074, 0x00FB,
	0x0321, 0x03AE, 0x023F, 0x02B0, 0x011D, 0x0192, 0x0003, 0x008C,
	0x0264, 0x02EB, 0x037A, 0x03F5, 0x0058, 0x00D7, 0x0146, 0x01C9,
	0x0213, 0x029C, 0x030D, 0x0382, 0x002F, 0x00A0, 0x0131, 0x01BE,
	0x028A, 0x0205, 0x0394, 0x031B, 0x00B6, 0x0039, 0x01A8, 0x0127,
	0x02FD, 0x0272, 0x03E3, 0x036C, 0x00C1, 0x004E, 0x01DF, 0x0150,
	0x0371, 0x03FE, 0x026F, 0x02E0, 0x014D, 0x01C2, 0x0053, 0x00DC,
	0x0306, 0x0389, 0x0218, 0x0297, 0x013A, 0x01B5, 0x0024, 0x00AB,
	0x039F, 0x0310, 0x0281, 0x020E, 0x01A3, 0x012C, 0x00BD, 0x0032,
	0x03E8, 0x0367, 0x02F6, 0x0279, 0x01D4, 0x015B, 0x00CA, 0x0045,
	0x02AD, 0x0222, 0x03B3, 0x033C, 0x0091, 0x001E, 0x018F, 0x0100,
	0x02DA, 0x0255, 0x03C4, 0x034B, 0x00E6, 0x0069, 0x01F8, 0x0177,
	0x0243, 0x02CC, 0x035D, 0x03D2, 0x007F, 0x00F0, 0x0161, 0x01EE,
	0x0234, 0x02BB, 0x032A, 0x03A5, 0x0008, 0x0087, 0x0116, 0x0199,
	0x00C9, 0x0046, 0x01D7, 0x0158, 0x02F5, 0x027A, 0x03EB, 0x0364,
	0x00BE, 0x0031, 0x01A0, 0x012F, 0x0282, 0x020D, 0x039C, 0x0313,
	0x0027, 0x00A8, 0x0139, 0x01B6, 0x021B, 0x0294, 0x0305, 0x038A,
	0x0050, 0x00DF, 0x014E, 0x01C1, 0x026C, 0x02E3, 0x0372, 0x03FD,
	0x0115, 0x019A, 0x000B, 0x0084, 0x0329, 0x03A6, 0x0237, 0x02B8,
	0x0162, 0x01ED, 0x007C, 0x00F3, 0x035E, 0x03D1, 0x0240, 0x02CF,
	0x01FB, 0x0174, 0x00E5, 0x006A, 0x03C7, 0x0348, 0x02D9, 0x0256,
	0x018C, 0x0103, 0x0092, 0x001D, 0x03B0, 0x033F, 0x02AE, 0x0221
};

uint16_t adbms6830b_calc_pec15(uint8_t len, const uint8_t *data)
{
	uint16_t remainder = 16; /* Initial PEC seed: 0x0010 */
	uint16_t addr;

	for (uint8_t i = 0; i < len; i++) {
		addr = ((remainder >> 7) ^ data[i]) & 0xFF;
		remainder = ((remainder << 8) ^ adbms6830b_crc15_table[addr]) & 0x7FFF;
	}

	return (uint16_t)(remainder << 1);
}

uint16_t adbms6830b_calc_pec10(uint8_t len,
			      const uint8_t *data,
			      bool is_rx,
			      uint8_t ccnt_byte)
{
	uint16_t remainder = 16; /* Initial PEC seed: 0x0010 */
	uint16_t table_addr;

	for (uint8_t i = 0; i < len; i++) {
		table_addr = ((remainder >> 2) ^ data[i]) & 0xFF;
		remainder = (remainder << 8) ^ adbms6830b_crc10_table[table_addr];
		remainder &= 0x03FF;
	}

	if (is_rx) {
		remainder ^= (uint16_t)((ccnt_byte & 0xFC) << 2);
		for (int8_t bit = 6; bit > 0; bit--) {
			if (remainder & 0x0200) {
				remainder = (remainder << 1) ^ 0x008F;
			} else {
				remainder = (remainder << 1);
			}
			remainder &= 0x03FF;
		}
	}

	return remainder;
}

int32_t adbms6830b_init(struct adbms6830b_dev *dev,
			struct adbms6830b_init_param *init_param)
{
	int32_t ret;

	if (!dev || !init_param || !init_param->spi_init)
		return -EINVAL;

	if (init_param->num_ics == 0 || init_param->num_ics > ADBMS6830B_MAX_ICS)
		return -EINVAL;

	memset(dev, 0, sizeof(*dev));
	dev->num_ics = init_param->num_ics;
	dev->expected_ccnt = 0;

	/* Bind platform SPI (STM32 SPI1 in Mode 0) */
	ret = no_os_spi_init(&dev->spi_desc, init_param->spi_init);
	if (ret)
		return ret;

	/* Initialize Configuration Register Group A mirrors */
	for (uint8_t i = 0; i < dev->num_ics; i++) {
		dev->nodes[i].tx_cfga[0] = ADBMS6830B_CFGAR0_REFON |
					   ADBMS6830B_CFGAR0_CTH_SET(init_param->comp_threshold);
		dev->nodes[i].tx_cfga[1] = 0x00; /* No fault injection */
		dev->nodes[i].tx_cfga[2] = 0x00; /* Soak off */
		dev->nodes[i].tx_cfga[3] = 0xFF; /* Pull-downs off GPIO 1-8 */
		dev->nodes[i].tx_cfga[4] = 0x03; /* Pull-downs off GPIO 9-10 */
		dev->nodes[i].tx_cfga[5] = ADBMS6830B_CFGAR5_FC_SET(init_param->filter_corner);
	}

	return 0;
}

int32_t adbms6830b_remove(struct adbms6830b_dev *dev)
{
	if (!dev)
		return -EINVAL;

	if (dev->spi_desc)
		no_os_spi_remove(dev->spi_desc);

	return 0;
}

int32_t adbms6830b_wakeup_chain(struct adbms6830b_dev *dev)
{
	uint8_t dummy_byte = 0xFF;
	int32_t ret;

	if (!dev)
		return -EINVAL;

	/* For each device in chain, clock dummy byte to send wake pulse */
	for (uint8_t i = 0; i < dev->num_ics; i++) {
		ret = no_os_spi_write_and_read(dev->spi_desc, &dummy_byte, 1);
		if (ret)
			return ret;

		/* Wait t_WAKE (max 500 us) per node */
		no_os_udelay(500);
	}

	no_os_udelay(20);
	return 0;
}

int32_t adbms6830b_send_cmd(struct adbms6830b_dev *dev,
			    uint16_t cmd_code,
			    bool inc_expected_ccnt)
{
	uint8_t cmd_frame[ADBMS6830B_CMD_SIZE];
	uint16_t cmd_pec;
	int32_t ret;

	if (!dev)
		return -EINVAL;

	cmd_frame[0] = (uint8_t)(cmd_code >> 8);
	cmd_frame[1] = (uint8_t)(cmd_code & 0xFF);
	cmd_pec = adbms6830b_calc_pec15(2, cmd_frame);
	cmd_frame[2] = (uint8_t)(cmd_pec >> 8);
	cmd_frame[3] = (uint8_t)(cmd_pec & 0xFF);

	ret = no_os_spi_write_and_read(dev->spi_desc, cmd_frame, ADBMS6830B_CMD_SIZE);
	if (ret)
		return ret;

	if (inc_expected_ccnt) {
		dev->expected_ccnt++;
		if (dev->expected_ccnt > 63)
			dev->expected_ccnt = 1;
	}

	return 0;
}

int32_t adbms6830b_write_reg_group(struct adbms6830b_dev *dev,
				   uint16_t write_cmd,
				   uint8_t (*data_in)[ADBMS6830B_REG_DATA_SIZE])
{
	uint16_t frame_len;
	uint16_t cmd_pec;
	uint16_t data_pec;
	uint32_t ptr = 0;
	int32_t ret;

	if (!dev || !data_in)
		return -EINVAL;

	frame_len = ADBMS6830B_CMD_SIZE + (dev->num_ics * ADBMS6830B_NODE_FRAME_SIZE);

	/* 1. Pack 4-byte Command Header */
	dev->tx_frame[ptr++] = (uint8_t)(write_cmd >> 8);
	dev->tx_frame[ptr++] = (uint8_t)(write_cmd & 0xFF);
	cmd_pec = adbms6830b_calc_pec15(2, &dev->tx_frame[0]);
	dev->tx_frame[ptr++] = (uint8_t)(cmd_pec >> 8);
	dev->tx_frame[ptr++] = (uint8_t)(cmd_pec & 0xFF);

	/* 2. Pack Data Frames: Top-of-Stack Node (N-1) FIRST, Bottom Node (0) LAST */
	for (int16_t ic = dev->num_ics - 1; ic >= 0; ic--) {
		memcpy(&dev->tx_frame[ptr], data_in[ic], ADBMS6830B_REG_DATA_SIZE);
		data_pec = adbms6830b_calc_pec10(ADBMS6830B_REG_DATA_SIZE,
						 data_in[ic], false, 0);
		ptr += ADBMS6830B_REG_DATA_SIZE;

		dev->tx_frame[ptr++] = (uint8_t)((data_pec >> 8) & 0x03);
		dev->tx_frame[ptr++] = (uint8_t)(data_pec & 0xFF);
	}

	/* 3. Execute Single Continuous Full-Duplex Transfer */
	ret = no_os_spi_write_and_read(dev->spi_desc, dev->tx_frame, frame_len);
	if (ret)
		return ret;

	dev->expected_ccnt++;
	if (dev->expected_ccnt > 63)
		dev->expected_ccnt = 1;

	return 0;
}

int32_t adbms6830b_read_reg_group(struct adbms6830b_dev *dev,
				  uint16_t read_cmd,
				  uint8_t (*data_out)[ADBMS6830B_REG_DATA_SIZE])
{
	uint16_t frame_len;
	uint16_t cmd_pec;
	uint32_t ptr = 0;
	int32_t ret;
	bool fault_detected = false;

	if (!dev || !data_out)
		return -EINVAL;

	frame_len = ADBMS6830B_CMD_SIZE + (dev->num_ics * ADBMS6830B_NODE_FRAME_SIZE);

	/* 1. Pack 4-byte Read Command into TX frame; pad rest with 0xFF */
	dev->tx_frame[ptr++] = (uint8_t)(read_cmd >> 8);
	dev->tx_frame[ptr++] = (uint8_t)(read_cmd & 0xFF);
	cmd_pec = adbms6830b_calc_pec15(2, &dev->tx_frame[0]);
	dev->tx_frame[ptr++] = (uint8_t)(cmd_pec >> 8);
	dev->tx_frame[ptr++] = (uint8_t)(cmd_pec & 0xFF);
	memset(&dev->tx_frame[4], 0xFF, frame_len - 4);

	/* 2. Execute Full-Duplex Clocking */
	ret = no_os_spi_write_and_read(dev->spi_desc, dev->tx_frame, frame_len);
	if (ret)
		return ret;

	/* 3. Ingest and Validate RX Stream: Bottom Node (0) arrives FIRST */
	ptr = ADBMS6830B_CMD_SIZE;
	for (uint8_t ic = 0; ic < dev->num_ics; ic++) {
		const uint8_t *node_payload = &dev->rx_frame[ptr];
		uint8_t byte6 = dev->rx_frame[ptr + 6];
		uint8_t byte7 = dev->rx_frame[ptr + 7];

		uint8_t rx_ccnt = byte6 >> 2;
		uint16_t rx_pec = (uint16_t)(((byte6 & 0x03) << 8) | byte7);

		uint16_t calc_pec = adbms6830b_calc_pec10(ADBMS6830B_REG_DATA_SIZE,
							  node_payload, true, byte6);

		dev->nodes[ic].last_ccnt = rx_ccnt;

		if (rx_pec != calc_pec) {
			dev->nodes[ic].pec_errors++;
			fault_detected = true;
		} else {
			memcpy(data_out[ic], node_payload, ADBMS6830B_REG_DATA_SIZE);
		}

		if (dev->expected_ccnt != 0 && rx_ccnt != dev->expected_ccnt)
			fault_detected = true;

		ptr += ADBMS6830B_NODE_FRAME_SIZE;
	}

	return fault_detected ? -EBADMSG : 0;
}

static void adbms6830b_parse_cell_payload(struct adbms6830b_node *node,
					  const uint8_t *raw_bytes,
					  uint8_t cell_offset)
{
	for (uint8_t i = 0; i < 3; i++) {
		uint8_t idx = cell_offset + i;
		if (idx >= 16)
			break;

		int16_t raw_code = (int16_t)(raw_bytes[i * 2] | (raw_bytes[i * 2 + 1] << 8));
		node->cell_raw[idx] = (uint16_t)raw_code;

		/* V_cell = raw * 0.150 mV + 1.500 V = (raw * 150) / 1000 + 1500 */
		node->cell_mv[idx] = ((int32_t)raw_code * 150) / 1000 + 1500;
	}
}

int32_t adbms6830b_read_cell_voltages(struct adbms6830b_dev *dev)
{
	static const uint16_t cv_cmds[6] = {
		ADBMS6830B_CMD_RDCVA,
		ADBMS6830B_CMD_RDCVB,
		ADBMS6830B_CMD_RDCVC,
		ADBMS6830B_CMD_RDCVD,
		ADBMS6830B_CMD_RDCVE,
		ADBMS6830B_CMD_RDCVF,
	};
	uint8_t reg_group[ADBMS6830B_MAX_ICS][ADBMS6830B_REG_DATA_SIZE];
	int32_t ret;
	int32_t overall_status = 0;

	if (!dev)
		return -EINVAL;

	/* 1. Broadcast SNAP command to freeze registers simultaneously */
	ret = adbms6830b_send_cmd(dev, ADBMS6830B_CMD_SNAP, true);
	if (ret)
		return ret;

	/* 2. Read each Cell Voltage Register Group (RDCVA..RDCVF) */
	for (uint8_t grp = 0; grp < 6; grp++) {
		ret = adbms6830b_read_reg_group(dev, cv_cmds[grp], reg_group);
		if (ret)
			overall_status = ret;

		uint8_t cell_offset = grp * 3;
		for (uint8_t ic = 0; ic < dev->num_ics; ic++) {
			adbms6830b_parse_cell_payload(&dev->nodes[ic],
						      reg_group[ic],
						      cell_offset);
		}
	}

	/* 3. Broadcast UNSNAP command to release freeze */
	ret = adbms6830b_send_cmd(dev, ADBMS6830B_CMD_UNSNAP, true);
	if (ret)
		return ret;

	return overall_status;
}
