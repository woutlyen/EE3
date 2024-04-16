/**
* Author: Robbe Decapmaker <debber@dcpm.be>
* Co-author: Kobe Michiels <kobe.michiels2@student.kuleuven.be>
* Team: HOME2
* Date: 10/02/2024
*/

#include "stm8_nrf24.h"

volatile uint8_t nrf24_status = 0;

void nrf24_setup_bus(void){
	// Enable SPI MOSI, MISO, CLK on port C
	GPIOC->DDR &= ~(1<<7);
	GPIOC->DDR |= (1 << 6);
	GPIOC->DDR |= (1 << 5);
	GPIOC->CR1 |= (1 << 7);
	GPIOC->CR1 |= (1 << 6);
	GPIOC->CR1 |= (1 << 5);
	GPIOC->CR2 &= ~(1<<7);
	GPIOC->CR2 &= ~(1<<6);
	GPIOC->CR2 &= ~(1<<5);
	
	// Enable CSN on port D5
	GPIOD->DDR |= (1 << 5);
	GPIOD->CR1 |= (1 << 5);
	
	// Enable Chip Enable (CE)
	GPIOD->DDR = ((1 << 0)| GPIOD->DDR);
	GPIOD->CR1 = ((1 << 0)| GPIOD->CR1);
	GPIOD->ODR = ((1 << 0)| GPIOD->ODR); // Turn the chip on
	// Deselect NRF
	GPIOD->ODR |= (1 << 5);
	
	// Init the STM8
	SPI_DeInit();
	SPI_Init(SPI_FIRSTBIT_MSB, SPI_BAUDRATEPRESCALER_16 ,SPI_MODE_MASTER, SPI_CLOCKPOLARITY_LOW, SPI_CLOCKPHASE_1EDGE, SPI_DATADIRECTION_2LINES_FULLDUPLEX, SPI_NSS_SOFT, SPI_CRC_TX);
	SPI_Cmd(ENABLE);
}

uint8_t nrf24_read_register(uint8_t mem_addr){
	// Init local vars
	volatile uint8_t status;
	volatile uint8_t contents;
	volatile uint8_t command;
	
	// Load addr into SPI command
	command = mem_addr;
	

	// Set the first three bits correct (000) (see page 19 of NRF doc)
	command &= ~(1<<5);
	command &= ~(1<<6);
	command &= ~(1<<7);
	
	// Select the NRF for comms
	GPIOD->ODR &= ~(1<<5);
	
	// Send the command over SPI
	SPI_SendData(command);
	
	// Wait for the SPI Status Register (SR) to give the goahead to buffer 2nd byte
	while(SPI_GetFlagStatus(SPI_FLAG_TXE) == RESET){
		nop();
	}
	
	// Send dummmy data to receive contents of nrf24
	SPI_SendData(0);
		
	// Wait for the SPI Status Register (SR) to give the goahead to read status
	while(SPI_GetFlagStatus(SPI_FLAG_RXNE) == RESET){
		nop();
	}
	
	// Get status register value
	status = SPI_ReceiveData();
	nrf24_status = status; // Update global status
	
	
	// Wait for the SPI Status Register (SR) to give the goahead to read data
	while(SPI_GetFlagStatus(SPI_FLAG_RXNE) == RESET){
		nop();
	}
	
	// Get status register value
	contents = SPI_ReceiveData();
	
	// Wait for the SPI Status Register (SR) to give the goahead to end write routine
	while(SPI_GetFlagStatus(SPI_FLAG_BSY) != RESET){
		nop();
	}
	
	
	// Deselect the NRF for comms
	GPIOD->ODR |= (1 << 5);
	
	// Return
	return contents;
}

void nrf24_write_register(uint8_t mem_addr, uint8_t data){
	// Init local vars
	volatile uint8_t status;
	volatile uint8_t command;
	
	// Load addr into SPI command
	command = mem_addr;
	

	// Set the first three bits correct (001) (see page 19 of NRF doc)
	command = ((1 << 5)| command);
	command &= ~(1<<6);
	command &= ~(1<<7);
	
	// Select the NRF for comms
	GPIOD->ODR &= ~(1<<5);
	
	// Send the command over SPI
	SPI_SendData(command);
	
	// Wait for the SPI Status Register (SR) to give the goahead to buffer 2nd byte
	while(SPI_GetFlagStatus(SPI_FLAG_TXE) == RESET){
		nop();
	}
	
	// Send data to register contents of nrf24
	SPI_SendData(data);
	
	// Wait for the SPI Status Register (SR) to give the goahead to end write routine
	while(SPI_GetFlagStatus(SPI_FLAG_BSY) != RESET){
		nop();
	}
	
	// Get status register value
	status = SPI_ReceiveData();
	nrf24_status = status; // Update global status
	
	// Deselect the NRF for comms
	GPIOD->ODR |= (1 << 5);
}

addr nrf24_read_address(uint8_t mem_addr){
	// Init local vars
	volatile uint8_t status;
	volatile uint8_t command;
	volatile addr ret_addr;
	
	// Load addr into SPI command
	command = mem_addr;
	

	// Set the first three bits correct (000) (see page 19 of NRF doc)
	command &= ~(1<<5);
	command &= ~(1<<6);
	command &= ~(1<<7);
	
	// Select the NRF for comms
	GPIOD->ODR &= ~(1<<5);
	
	// Send the command over SPI
	SPI_SendData(command);
	
	// Wait for the SPI Status Register (SR) to give the goahead to buffer 2nd byte
	while(SPI_GetFlagStatus(SPI_FLAG_TXE) == RESET){
		nop();
	}
	
	// Send dummy byte to nrf24 to get contents
	SPI_SendData(0);
		
	// Wait for the SPI Status Register (SR) to give the goahead to read status
	while(SPI_GetFlagStatus(SPI_FLAG_RXNE) == RESET){
		nop();
	}
	
	// Get status register value
	status = SPI_ReceiveData();
	nrf24_status = status; // Update global status
	
	// Wait for the SPI Status Register (SR) to give the goahead to buffer 2nd byte
	while(SPI_GetFlagStatus(SPI_FLAG_TXE) == RESET){
		nop();
	}
	
	// Send dummy byte to nrf24 to get contents
	SPI_SendData(0);
	
	// Wait for the SPI Status Register (SR) to give the goahead to read status
	while(SPI_GetFlagStatus(SPI_FLAG_RXNE) == RESET){
		nop();
	}
	
	// Get first byte from the register value
	ret_addr.byte0 = SPI_ReceiveData();

	// Wait for the SPI Status Register (SR) to give the goahead to buffer 2nd byte
	while(SPI_GetFlagStatus(SPI_FLAG_TXE) == RESET){
		nop();
	}
	
	// Send dummy byte to nrf24 to get contents
	SPI_SendData(0);
	
	// Wait for the SPI Status Register (SR) to give the goahead to read status
	while(SPI_GetFlagStatus(SPI_FLAG_RXNE) == RESET){
		nop();
	}
	
	// Get first byte from the register value
	ret_addr.byte1 = SPI_ReceiveData();
	
	// Send dummy byte to nrf24 to get contents
	SPI_SendData(0);
	
	// Wait for the SPI Status Register (SR) to give the goahead to read status
	while(SPI_GetFlagStatus(SPI_FLAG_RXNE) == RESET){
		nop();
	}
	
	// Get first byte from the register value
	ret_addr.byte2 = SPI_ReceiveData();
	
	// Wait for the SPI Status Register (SR) to give the goahead to buffer 2nd byte
	while(SPI_GetFlagStatus(SPI_FLAG_TXE) == RESET){
		nop();
	}
	
	// Send dummy byte to nrf24 to get contents
	SPI_SendData(0);
	
	// Wait for the SPI Status Register (SR) to give the goahead to read status
	while(SPI_GetFlagStatus(SPI_FLAG_RXNE) == RESET){
		nop();
	}
	
	// Get first byte from the register value
	ret_addr.byte3 = SPI_ReceiveData();
	
	// Wait for the SPI Status Register (SR) to give the goahead to read status
	while(SPI_GetFlagStatus(SPI_FLAG_RXNE) == RESET){
		nop();
	}
	
	// Get first byte from the register value
	ret_addr.byte4 = SPI_ReceiveData();
	
	// Wait for the SPI Status Register (SR) to give the goahead to end write routine
	while(SPI_GetFlagStatus(SPI_FLAG_BSY) != RESET){
		nop();
	}
	
	// Deselect the NRF for comms
	GPIOD->ODR |= (1 << 5);
	return ret_addr;
}

void nrf24_write_address(uint8_t mem_addr, addr data){
	// Init local vars
	volatile uint8_t status;
	volatile uint8_t command;
	
	// Load addr into SPI command
	command = mem_addr;
	

	// Set the first three bits correct (001) (see page 19 of NRF doc)
	command = ((1 << 5)| command);
	command &= ~(1<<6);
	command &= ~(1<<7);
	
	// Select the NRF for comms
	GPIOD->ODR &= ~(1<<5);
	
	// Send the command over SPI
	SPI_SendData(command);
	
	// Wait for the SPI Status Register (SR) to give the goahead to buffer 1st byte
	while(SPI_GetFlagStatus(SPI_FLAG_TXE) == RESET){
		nop();
	}
	
	// Send data to register contents of nrf24
	SPI_SendData(data.byte0);
	
	// Wait for the SPI Status Register (SR) to give the goahead to read status
	while(SPI_GetFlagStatus(SPI_FLAG_RXNE) == RESET){
		nop();
	}
	
	// Get status register value
	status = SPI_ReceiveData();
	nrf24_status = status; // Update global status
	
	
	// Wait for the SPI Status Register (SR) to give the goahead to buffer 2nd byte
	while(SPI_GetFlagStatus(SPI_FLAG_TXE) == RESET){
		nop();
	}
	
	// Send data to register contents of nrf24
	SPI_SendData(data.byte1);
	
	// Wait for the SPI Status Register (SR) to give the goahead to buffer 3rd byte
	while(SPI_GetFlagStatus(SPI_FLAG_TXE) == RESET){
		nop();
	}
	
	// Send data to register contents of nrf24
	SPI_SendData(data.byte2);
	
	// Wait for the SPI Status Register (SR) to give the goahead to buffer 4th byte
	while(SPI_GetFlagStatus(SPI_FLAG_TXE) == RESET){
		nop();
	}
	
	// Send data to register contents of nrf24
	SPI_SendData(data.byte3);
	
	// Wait for the SPI Status Register (SR) to give the goahead to buffer 5th byte
	while(SPI_GetFlagStatus(SPI_FLAG_TXE) == RESET){
		nop();
	}
	
	// Send data to register contents of nrf24
	SPI_SendData(data.byte4);
	
	// Wait for the SPI Status Register (SR) to give the goahead to end write routine
	while(SPI_GetFlagStatus(SPI_FLAG_BSY) != RESET){
		nop();
	}
	
	// Deselect the NRF for comms
	GPIOD->ODR |= (1 << 5);
}

void nrf8_write_payload(datagram data){
	// Init local vars
	volatile uint8_t status;
	volatile uint8_t command;
	
	// Set the right NRF command
	command = W_TX_PAYLOAD;
	
	// Select the NRF for comms
	GPIOD->ODR &= ~(1<<5);
	
	// Send the command over SPI
	SPI_SendData(command);
	
	// Wait for the SPI Status Register (SR) to give the goahead to buffer 1st byte
	while(SPI_GetFlagStatus(SPI_FLAG_TXE) == RESET){
		nop();
	}
	
	// Send data to register contents of nrf24
	SPI_SendData(data.dev);
	
	// Wait for the SPI Status Register (SR) to give the goahead to read status
	while(SPI_GetFlagStatus(SPI_FLAG_RXNE) == RESET){
		nop();
	}
	
	// Get status register value
	status = SPI_ReceiveData();
	nrf24_status = status; // Update global status
	
	
	// Wait for the SPI Status Register (SR) to give the goahead to buffer 2nd byte
	while(SPI_GetFlagStatus(SPI_FLAG_TXE) == RESET){
		nop();
	}
	
	// Send data to register contents of nrf24
	SPI_SendData(data.command);
	
	// Wait for the SPI Status Register (SR) to give the goahead to buffer 3rd byte
	while(SPI_GetFlagStatus(SPI_FLAG_TXE) == RESET){
		nop();
	}
	
	// Send data to register contents of nrf24
	SPI_SendData(data.data_byte0);
	
	// Wait for the SPI Status Register (SR) to give the goahead to buffer 4th byte
	while(SPI_GetFlagStatus(SPI_FLAG_TXE) == RESET){
		nop();
	}
	
	// Send data to register contents of nrf24
	SPI_SendData(data.data_byte1);
	
	// Wait for the SPI Status Register (SR) to give the goahead to buffer 5th byte
	while(SPI_GetFlagStatus(SPI_FLAG_TXE) == RESET){
		nop();
	}
	
	// Send data to register contents of nrf24
	SPI_SendData(data.status);
	
	// Wait for the SPI Status Register (SR) to give the goahead to end write routine
	while(SPI_GetFlagStatus(SPI_FLAG_BSY) != RESET){
		nop();
	}
	
	// Deselect the NRF for comms
	GPIOD->ODR |= (1 << 5);
}

datagram nrf24_read_payload(void){
		// Init local vars
	volatile uint8_t status;
	volatile uint8_t command;
	volatile datagram ret_data;
	
	// Load read rx into SPI command
	command = R_RX_PAYLOAD;
	
	// Select the NRF for comms
	GPIOD->ODR &= ~(1<<5);
	
	// Send the command over SPI
	SPI_SendData(command);
	
	// Wait for the SPI Status Register (SR) to give the goahead to buffer 2nd byte
	while(SPI_GetFlagStatus(SPI_FLAG_TXE) == RESET){
		nop();
	}
	
	// Send dummy byte to nrf24 to get contents
	SPI_SendData(0);
		
	// Wait for the SPI Status Register (SR) to give the goahead to read status
	while(SPI_GetFlagStatus(SPI_FLAG_RXNE) == RESET){
		nop();
	}
	
	// Get status register value
	status = SPI_ReceiveData();
	nrf24_status = status; // Update global status
	
	// Wait for the SPI Status Register (SR) to give the goahead to buffer 2nd byte
	while(SPI_GetFlagStatus(SPI_FLAG_TXE) == RESET){
		nop();
	}
	
	// Send dummy byte to nrf24 to get contents
	SPI_SendData(0);
	
	// Wait for the SPI Status Register (SR) to give the goahead to read status
	while(SPI_GetFlagStatus(SPI_FLAG_RXNE) == RESET){
		nop();
	}
	
	// Get first byte from the register value
	ret_data.dev = SPI_ReceiveData();

	// Wait for the SPI Status Register (SR) to give the goahead to buffer 2nd byte
	while(SPI_GetFlagStatus(SPI_FLAG_TXE) == RESET){
		nop();
	}
	
	// Send dummy byte to nrf24 to get contents
	SPI_SendData(0);
	
	// Wait for the SPI Status Register (SR) to give the goahead to read status
	while(SPI_GetFlagStatus(SPI_FLAG_RXNE) == RESET){
		nop();
	}
	
	// Get first byte from the register value
	ret_data.command = SPI_ReceiveData();
	
	// Send dummy byte to nrf24 to get contents
	SPI_SendData(0);
	
	// Wait for the SPI Status Register (SR) to give the goahead to read status
	while(SPI_GetFlagStatus(SPI_FLAG_RXNE) == RESET){
		nop();
	}
	
	// Get first byte from the register value
	ret_data.data_byte0 = SPI_ReceiveData();
	
	// Wait for the SPI Status Register (SR) to give the goahead to buffer 2nd byte
	while(SPI_GetFlagStatus(SPI_FLAG_TXE) == RESET){
		nop();
	}
	
	// Send dummy byte to nrf24 to get contents
	SPI_SendData(0);
	
	// Wait for the SPI Status Register (SR) to give the goahead to read status
	while(SPI_GetFlagStatus(SPI_FLAG_RXNE) == RESET){
		nop();
	}
	
	// Get first byte from the register value
	ret_data.data_byte1 = SPI_ReceiveData();
	
	// Wait for the SPI Status Register (SR) to give the goahead to read status
	while(SPI_GetFlagStatus(SPI_FLAG_RXNE) == RESET){
		nop();
	}
	
	// Get first byte from the register value
	ret_data.status = SPI_ReceiveData();
	
	// Wait for the SPI Status Register (SR) to give the goahead to end write routine
	while(SPI_GetFlagStatus(SPI_FLAG_BSY) != RESET){
		nop();
	}
	
	// Deselect the NRF for comms
	GPIOD->ODR |= (1 << 5);
	return ret_data;
}

uint8_t nrf24_get_status(void){
	return nrf24_status;
}

void nrf24_flush_rx(void){
	volatile uint8_t status;
	
	// Select the NRF for comms
	GPIOD->ODR &= ~(1<<5);
	
	// Send the command over SPI
	SPI_SendData(FLUSH_RX);
	
	// Wait for the SPI Status Register (SR) to give the goahead to read status
	while(SPI_GetFlagStatus(SPI_FLAG_RXNE) == RESET){
		nop();
	}
	
	// Get status register value
	status = SPI_ReceiveData();
	nrf24_status = status; // Update global status
	
	// Wait for the SPI Status Register (SR) to give the goahead to end write routine
	while(SPI_GetFlagStatus(SPI_FLAG_BSY) != RESET){
		nop();
	}
	
	// Deselect the NRF for comms
	GPIOD->ODR |= (1 << 5);
}

void nrf24_flush_tx(void){
	volatile uint8_t status;
	
	// Select the NRF for comms
	GPIOD->ODR &= ~(1<<5);
	
	// Send the command over SPI
	SPI_SendData(FLUSH_TX);
	
	// Wait for the SPI Status Register (SR) to give the goahead to read status
	while(SPI_GetFlagStatus(SPI_FLAG_RXNE) == RESET){
		nop();
	}
	
	// Get status register value
	status = SPI_ReceiveData();
	nrf24_status = status; // Update global status
	
	// Wait for the SPI Status Register (SR) to give the goahead to end write routine
	while(SPI_GetFlagStatus(SPI_FLAG_BSY) != RESET){
		nop();
	}
	
	// Deselect the NRF for comms
	GPIOD->ODR |= (1 << 5);
}

uint8_t nrf24_update_status(void){
	volatile uint8_t status;
	
	// Select the NRF for comms
	GPIOD->ODR &= ~(1<<5);
	
	// Send the command over SPI
	SPI_SendData(NRF_NOP);
	
	// Wait for the SPI Status Register (SR) to give the goahead to read status
	while(SPI_GetFlagStatus(SPI_FLAG_RXNE) == RESET){
		nop();
	}
	
	// Get status register value
	status = SPI_ReceiveData();
	nrf24_status = status; // Update global status
	
	// Wait for the SPI Status Register (SR) to give the goahead to end nop routine
	while(SPI_GetFlagStatus(SPI_FLAG_BSY) != RESET){
		nop();
	}
	
	
	// Deselect the NRF for comms
	GPIOD->ODR |= (1 << 5);
	
	// Return
	return status;
}

void nrf24_CE_on(void){
	GPIOD->ODR = ((1 << 0)| GPIOD->ODR); // Turn the chip on 
}


void nrf24_CE_off(void){
	GPIOD->ODR &= ~(1<<0); // Turn the chip off
}

void nrf24_RX_mode(void){
	volatile uint8_t current_config = 0;
	current_config = nrf24_read_register(0x00);
	current_config = ((1 << 0)| current_config);
	nrf24_write_register(0x00, current_config);
	nrf24_CE_on();
}

void nrf24_TX_mode(void){
	volatile uint8_t current_config;
	current_config = nrf24_read_register(0x00);
	current_config &= ~(1<<0);
	nrf24_write_register(0x00, current_config);
	nrf24_CE_on();
}

void config_nrf(void){
	volatile addr addr_buffer;
	volatile addr addr_tx;
	
	nrf24_CE_off();
	// RF_CH
	nrf24_write_register(0x05, 0x62);

	// RX_PW_P0
	nrf24_write_register(0x11, 0x05);
	
	// RX_PW_P1
	nrf24_write_register(0x12, 0x05);
	
	// Config register
	nrf24_write_register(0x00, 0x0B);
	
	nrf24_CE_on();
	
	// status register -- resetting the IRQ of the NRF
	nrf24_write_register(0x07, 0x40);
	
	// status register
	nrf24_write_register(0x07, 0x30);
	
	nrf24_flush_rx();
	
	addr_buffer.byte0 = 0x46;
	addr_buffer.byte1 = 0x47;
	addr_buffer.byte2 = 0x48;
	addr_buffer.byte3 = 0x49;
	addr_buffer.byte4 = 0x4A;
	
	// Write data to address (RX_ADDR_P1)
	nrf24_write_address(0x0B, addr_buffer);
	
	addr_tx.byte0 = 0x4B;
	addr_tx.byte1 = 0x4C;
	addr_tx.byte2 = 0x4D;
	addr_tx.byte3 = 0x4E;
	addr_tx.byte4 = 0x4F;
	
	// Write data to address (RX_ADDR_P1)
	nrf24_write_address(0x10, addr_buffer);
	nrf24_write_address(0x0A, addr_buffer);
	
	
	
	// setup_retr
	nrf24_write_register(0x04, 0x03);
	
	// RF_setup
	nrf24_write_register(0x06, 0x07);
	
	// EN_AA
	nrf24_write_register(0x01, 0x3F);
	
	// EN_RXADDR
	nrf24_write_register(0x02, 0x03);
	
	// Setup_AW
	nrf24_write_register(0x03, 0x03);
	

}

void setup_interupt(void){
	// GPIO_DeInit(GPIOC);
	// GPIO_Init(GPIOC, GPIO_PIN_4, GPIO_MODE_IN_FL_IT);
	GPIOC->CR2 = ((1 << 4)| GPIOC->CR2);
	GPIOC->DDR &= ~(1<<4);
	GPIOC->CR1 &= ~(1<<4);
	EXTI_DeInit();
	EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOC, EXTI_SENSITIVITY_FALL_ONLY);
	EXTI_SetTLISensitivity(EXTI_TLISENSITIVITY_FALL_ONLY);
	
}

void send_data_nrf(datagram data, uint8_t depth){
	volatile addr addr_tx;
	volatile uint8_t status_contents = 0;	
	
	// Check recursion depth
	if(depth >= 10){
		// status register -- resetting the IRQ of the NRF
		nrf24_write_register(0x07, 0x40);
		nrf24_write_register(0x07, 0x30);
		
		// Config register
		nrf24_write_register(0x00, 0x0B);
		
		// Tun interrupts back on 
		enableInterrupts();
		return;
	}

	// Turn of interrupts to send data
	disableInterrupts();
	
	status_contents = nrf24_update_status();
	
	nrf24_CE_off();

	// Config register
	nrf24_write_register(0x00, 0x0A);

	// status register
	nrf24_write_register(0x07, 0x30);
	
	nrf24_flush_tx();
	
	nrf8_write_payload(data);
	
	nrf24_CE_on(); // This intiates the send
	
	while(GPIO_ReadInputPin(GPIOC, GPIO_PIN_4) != RESET){
		nop();
	}
	
	//for( status_contents = 0; status_contents < 200; status_contents++){
	//	nop();
	//}
	status_contents = nrf24_read_register(0x07);
	
	if(status_contents != 0x2E){
		send_data_nrf(data, depth + 1);
		return;
	}
	
	// status register -- resetting the IRQ of the NRF
	nrf24_write_register(0x07, 0x40);
	nrf24_write_register(0x07, 0x30);
	
	while(GPIO_ReadInputPin(GPIOC, GPIO_PIN_4) == RESET){
		nop();
	}
	

	
	// Config register
	nrf24_write_register(0x00, 0x0B);
	
	// Tun interrupts back on 
	enableInterrupts();
	return;
}