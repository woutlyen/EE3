/**
* Author: Robbe Decapmaker <debber@dcpm.be>
* Team: HOME2
* Date: 10/02/2024
*/

#ifndef _STM8_NFR24_H_
#define _STM8_NRF24_H_

// Includes for the NRF24 lib
#include "stm8s.h"

// Relevant structs
typedef struct addr{ // See figre 7 on page 18 of NRF24 docs
	uint8_t byte0;
	uint8_t byte1;
	uint8_t byte2;
	uint8_t byte3;
	uint8_t byte4;
} addr;

typedef struct datagram{ // Datagram defining comm protocol STM8 and ESP32
	uint8_t dev; // Which device connected to the MCU do you want to use
	uint8_t command; // What do you want to do with that device
	uint8_t data_byte0; // Extra data to go along with the command
	uint8_t data_byte1; 
	uint8_t status; // status of the sending MCU
} datagram;

// SPI instructions
#define R_RX_PAYLOAD 0x61
#define W_TX_PAYLOAD 0xA0
#define FLUSH_TX 0xE1
#define FLUSH_RX  0xE2
#define REUSE_TX_PL 0xE3
#define NRF_NOP 0xFF

// SPI helpers
#define ENABLE_SPI_NRF24 0xDF
#define DISABLE_SPI_NRF24 0xFF

// Functions

/**
* This function is responsible for setting up the SPI bus needed to communicate with the NRF
*/
void nrf24_setup_bus(void);

/**
*	This function is responsible for reading a regular register from the NRF24
* It expects a Memory Map Address and returns the data stored at that address
* It also updates the status
*/
uint8_t nrf24_read_register(uint8_t mem_addr);

/**
*	This function is responsible for writing a regular register of the NRF24
* It expects a Memory Map Address, data to be written and returns nothing.
* It also updates the status
*/
void nrf24_write_register(uint8_t mem_addr, uint8_t data);

/**
*	This function is responsible for reading an address register from the NRF24
* It expects a Memory Map Address and returns the data stored at that address
* It also updates the status
*/
addr nrf24_read_address(uint8_t mem_addr);

/**
*	This function is responsible for writing an address register of the NRF24
* It expects a Memory Map Address, data to be written and returns nothing.
* It also updates the status
*/
void nrf24_write_address(uint8_t mem_addr, addr data);

/**
*	This function is responsible for writing a payload to the TX FIFO of the NRF24
* It expects data to be written and returns nothing.
* It also updates the status
*/
void nrf8_write_payload(datagram data);

/**
*	This function is responsible for reading the RX FIFO from the NRF24
* It returns the datagram present there
* It also updates the status
*/
datagram nrf24_read_payload(void);

/*
*	This function flushes the RX FIFO 
* It also updates the status
*/
void nrf24_flush_rx(void);

/*
*	This function flushes the TX FIFO 
* It also updates the status
*/
void nrf24_flush_tx(void);

/**
* Get the last known status of the NRF24
*/
uint8_t nrf24_get_status(void);

/**
*	Update the status register and get it
*/
uint8_t nrf24_update_status(void);

/**
* Turn on the CE pin for the NRF24
*/
void nrf24_CE_on(void);

/**
* Turn off the CE pin for the NRF24
*/
void nrf24_CE_off(void);

/**
* Turn on the RX mode for the NRF24
*/
void nrf24_RX_mode(void);

/**
* Turn off the TX mode for the NRF24
*/
void nrf24_TX_mode(void);

void send_data_nrf(datagram data, uint8_t depth);
void config_nrf(void);
void setup_interupt(void);

#endif /* _STM8_NRF24_H_*/
