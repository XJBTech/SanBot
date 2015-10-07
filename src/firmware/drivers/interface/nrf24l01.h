/*
 *  ____    ____    __   __   ______  ______
 * /\  _`\ /\  _`\ /\ \ /\ \ /\__  _\/\  _  \
 * \ \ \/\ \ \ \L\_\ `\`\/'/'\/_/\ \/\ \ \L\ \
 *  \ \ \ \ \ \  _\L`\/ > <     \ \ \ \ \  __ \
 *   \ \ \_\ \ \ \L\ \ \/'/\`\   \ \ \ \ \ \/\ \
 *    \ \____/\ \____/ /\_\\ \_\  \ \_\ \ \_\ \_\
 *     \/___/  \/___/  \/_/ \/_/   \/_/  \/_/\/_/
 *
 * Originally created by Dexta Robotics.
 * Copyright <C> Dexta Robotics, 2015.
 * All rights reserved.
 */

#ifndef __NRF24L01_H
#define __NRF24L01_H

#define SBN1_PAYLOAD_WIDTH 32

#define TX_ADR_WIDTH 	5
#define TX_PLOAD_WIDTH  SBN1_PAYLOAD_WIDTH

#define RX_ADR_WIDTH    5
#define RX_PLOAD_WIDTH  SBN1_PAYLOAD_WIDTH

#define CHANNEL 40

#define NRF_READ_REG    0x00  // Define read command to register
#define NRF_WRITE_REG   0x20  // Define write command to register
#define RD_RX_PLOAD     0x61  // Define RX payload register address
#define WR_TX_PLOAD 	0xA0  // Define TX payload register address
#define FLUSH_TX    	0xE1  // Define flush TX register command
#define FLUSH_RX    	0xE2  // Define flush RX register command
#define REUSE_TX_PL 	0xE3  // Define reuse TX payload register command
#define NOP         	0xFF  // Define No Operation, might be used to read status register

#define CONFIG      0x00  // 'Config' register address
#define EN_AA       0x01  // 'Enable Auto Acknowledgment' register address
#define EN_RXADDR   0x02  // 'Enabled RX addresses' register address
#define SETUP_AW    0x03  // 'Setup address width' register address
#define SETUP_RETR  0x04  // 'Setup Auto. Retrans' register address
#define RF_CH       0x05  // 'RF channel' register address
#define RF_SETUP    0x06  // 'RF setup' register address
#define STATUS      0x07  // 'Status' register address
#define OBSERVE_TX  0x08  // 'Observe TX' register address
#define CD          0x09  // 'Carrier Detect' register address
#define RX_ADDR_P0  0x0A  // 'RX address pipe0' register address
#define RX_ADDR_P1  0x0B  // 'RX address pipe1' register address
#define RX_ADDR_P2  0x0C  // 'RX address pipe2' register address
#define RX_ADDR_P3  0x0D  // 'RX address pipe3' register address
#define RX_ADDR_P4  0x0E  // 'RX address pipe4' register address
#define RX_ADDR_P5  0x0F  // 'RX address pipe5' register address
#define TX_ADDR     0x10  // 'TX address' register address
#define RX_PW_P0    0x11  // 'RX payload width, pipe0' register address
#define RX_PW_P1    0x12  // 'RX payload width, pipe1' register address
#define RX_PW_P2    0x13  // 'RX payload width, pipe2' register address
#define RX_PW_P3    0x14  // 'RX payload width, pipe3' register address
#define RX_PW_P4    0x15  // 'RX payload width, pipe4' register address
#define RX_PW_P5    0x16  // 'RX payload width, pipe5' register address
#define FIFO_STATUS 0x17  // 'FIFO Status Register' register address

#define MAX_RT      0x10
#define TX_DS		0x20
#define RX_DR		0x40

#define SPI1_DR_Addr ( (u32)0x4001300C )
#define SPI2_DR_Addr ( (u32)0x4000380C )

uint8_t nrf24l01SetAddress(void);
void nrf24l01Init(void);
void nrf24l01RxMode(void);
void nrf24l01TxMode(void);
uint8_t nrf24l01TxData(uint8_t *_pTxBuf);
uint8_t nrf24l01RxData(uint8_t *_pRxBuf);
uint8_t nrf24l01ConnectCheck(void);
void nrf24l01ChangeAddress(uint8_t Address);

extern uint8_t nRF_ReceiveBuffer[RX_PLOAD_WIDTH], nRF_SendBuffer[TX_PLOAD_WIDTH];
extern uint8_t nRF_Address;
extern uint8_t nrf_led;

#endif
