/*
* The MIT License (MIT)
* 
* Copyright (c) 2015 David Ogilvy (MetalPhreak)
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include "driver/ir_blaster.h"


#define SPI_DEV HSPI

////////////////////////////////////////////////////////////////////////////////
//
// Function Name: ir_blaster_INIT
//   Description: Initialises SPI hardware for IR blasting operation.
//                SPI clock signal becomes the carrier wave (connect to input of mosfet/transistor).
//                MOSI connected to mosfet gate/transistor base controls carrier wave output 
//    Parameters: frequency - carrier frequency F36KHZ etc (see ir_blaster_register.h)
//                duty_cycle - duty cycle of carrier in percent DCP33 for 33% etc
//				 
////////////////////////////////////////////////////////////////////////////////

void ir_blaster_INIT(uint8 frequency, uint8 duty_cycle){
	
	//Init MOSI and SCLK pins
	WRITE_PERI_REG(PERIPHS_IO_MUX, 0x105); //Set bit 9 if 80MHz sysclock required
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, 2); //GPIO13 is HSPI MOSI pin (Master Data Out)
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, 2); //GPIO14 is HSPI CLK pin (Clock)
	spi_tx_byte_order(SPI_DEV, SPI_BYTE_ORDER_HIGH_TO_LOW);
	SET_PERI_REG_MASK(SPI_USER(SPI_DEV), SPI_USR_MOSI);
	CLEAR_PERI_REG_MASK(SPI_USER(SPI_DEV), SPI_USER_COMMAND|SPI_USER_ADDR|SPI_USR_DUMMY|SPI_USR_MISO|SPI_CS_SETUP|SPI_CS_HOLD|SPI_FLASH_MODE);


	uint16 prediv = 0;
	uint8 cntdiv  = 0;
	uint8 dutycnt = 0;

	//get prediv and cntdiv
	switch(frequency) {
		case F36KHZ: 
			prediv = 10; //8 MHz
			cntdiv = 222; //36.036 KHz
			break;

		case F38KHZ:
			prediv = 10; //8 MHZ
			cntdiv = 210; //38.09 KHz
			break;

		case F40KHZ:
			prediv = 10; //8 MHZ
			cntdiv = 200; //40 KHz
			break;

		case F56KHZ:
			prediv = 10; //8 MHZ
			cntdiv = 143; //55.94 KHz
			break;
		default: //36KHz
			prediv = 10; //8 MHz
			cntdiv = 222; //36.036 KHz
			break;		
	}

	//get duty cycle
	switch(duty_cycle) {
		case DCP25:
			dutycnt = cntdiv>>2;
			break;

		case DCP33:
			dutycnt = cntdiv/3;
			break;

		case DCP50:
			dutycnt = cntdiv>>1;
			break;

		case DCP100:
			dutycnt = cntdiv; //not sure if this works as intended...
			break;

		default:
			dutycnt = cntdiv>>2;
			break;
	}


	//set clock and clock duty cycle
	WRITE_PERI_REG(SPI_CLOCK(SPI_DEV), 
			(((prediv-1)&SPI_CLKDIV_PRE)<<SPI_CLKDIV_PRE_S)|
			(((cntdiv-1)&SPI_CLKCNT_N)<<SPI_CLKCNT_N_S)|
			(((cntdiv>>1)&SPI_CLKCNT_H)<<SPI_CLKCNT_H_S)|
			((0&SPI_CLKCNT_L)<<SPI_CLKCNT_L_S));
	//done
}

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
// Function Name: func
//   Description: 
//    Parameters: 
//				 
////////////////////////////////////////////////////////////////////////////////

void RC5(uint32 command){
	uint8 cmdlen = 14; //hardcode for now
	uint32 cmd = command;

	ir_blaster_INIT(F36KHZ, DCP25);

	while(cmdlen){
		
		uint32 cmdbit = (cmd>>(cmdlen-1) & 1;
		if(cmdbit){
			//send "1" bit
			spi_transaction(SPI_DEV, 0, 0, 0, 0, 32, 0x0000, 0, 0); //low 32 cycles
			spi_transaction(SPI_DEV, 0, 0, 0, 0, 32, 0xFFFF, 0, 0); //high 32 cycles
		} else {
			//send "0" bit
			spi_transaction(SPI_DEV, 0, 0, 0, 0, 32, 0xFFFF, 0, 0); //high 32 cycles
			spi_transaction(SPI_DEV, 0, 0, 0, 0, 32, 0x0000, 0, 0); //low 32 cycles
		}
		cmdlen--;
	}
}

////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//
// Function Name: func
//   Description: 
//    Parameters: 
//				 
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////


