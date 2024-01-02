/**
 * LIS3DH
 *
 *  Created on: 28.12.2023
 *      Author: Flämig, Tobias
 *       Email: flaemig@dhbw-stuttgart.de
 */

// XYZopt used the BLE switch High and low Byte order for a faster reading process using one readBurst
#define XYZopt

#include <mcalSysTick.h>
#include <mcalGPIO.h>
#include <mcalSPI.h>
#include "lis3dh_spi.h"


	// LIS3DH
	bool LIS3DH_spi = false;


	#define StatusReg 	0x27
	#define AcclReg 	0x28
	#define ADC1Reg 	0x08
	#define ADC3Temp 	0x0C
	#define	DummyReg 	0x0F			// check value at present dummyVal (0x33)
	#define ADCConfReg 	0x1F
	#define CtrlReg1 	0x20
	#define CtrlReg4	0x23
	#define CtrlReg5 	0x24
	#define StatusReg 	0x27
	#define OUTxyz 		0x28
	#define dummyVal	0x33


//SPI specific declaration
	#define spiCont 	0x40
    #define spiRead 	0x80
	#define spiOUTxyz 	0xE8
static SPI_TypeDef  *spi  = SPI1;


void LIS3DH_init_spi(void)
{
	gpioInitPort(LIS3DH_CS_PORT);
	gpioSelectPinMode(LIS3DH_CS_PORT,LIS3DH_CS, OUTPUT);        // CS
	gpioSelectPushPullMode(LIS3DH_CS_PORT, LIS3DH_CS, PULLUP);
	LIS3DH_CS1;

	gpioInitPort(LIS3DH_SPI_PORT);
	gpioSelectPinMode(LIS3DH_SPI_PORT, LIS3DH_CLK, ALTFUNC);     // SPI1 Clock
	gpioSelectAltFunc(LIS3DH_SPI_PORT, LIS3DH_CLK, AF5);
	gpioSelectPinMode(LIS3DH_SPI_PORT, LIS3DH_MOSI, ALTFUNC);    // SPI1 MOSI
	gpioSelectAltFunc(LIS3DH_SPI_PORT, LIS3DH_MOSI, AF5);
	gpioSelectPinMode(LIS3DH_SPI_PORT, LIS3DH_MISO, ALTFUNC);    // SPI1 MISO
	gpioSelectAltFunc(LIS3DH_SPI_PORT, LIS3DH_MISO, AF5);
	// Init SPI1
	spiSelectSPI(spi);
	spiInitSPI(spi, CLK_DIV_16, SPI_DATA_8_BIT, SSM_ON, SSI_LVL_HIGH, MASTER, SPI_PHASE_EDGE_2, SPI_IDLE_HIGH);

}


// Send byte via SPI to controller
void spiLIS3DH_senddata(uint8_t reg, uint8_t data)
{
	spiSendData(spi, LIS3DH_CS_PORT, LIS3DH_CS, reg, data);
}


/* does not work

uint8_t spiLIS3DH_getRegData(uint8_t reg)
{
	uint8_t h = spiReadRegByte(spi, LIS3DH_CS_PORT, LIS3DH_CS, (uint8_t) (reg|spiRead));
	return h;
}

uint16_t spiLIS3DH_getRegWord(uint8_t reg)
{
	uint16_t h;
	spiReadRegWord(spi, LIS3DH_CS_PORT, LIS3DH_CS, (uint8_t) (reg|spiRead|spiCont), &h);
	return h;
}

*/

int8_t LIS3DH_presCheck()
{
	uint8_t ret[2];
	//ret[1] = spiLIS3DH_getRegData(DummyReg|spiRead))

	spiReadRegBurst(spi, LIS3DH_CS_PORT, LIS3DH_CS, (DummyReg|spiRead), ret, 2);

	if (dummyVal == ret[1])
	{return 1;}
	else
	{return 0;}

}


int8_t spiLIS3DH_init(int8_t restart)
{
	#define stepStart -7
	static int8_t step = stepStart ;

	if ((restart != 0) && (step == stepStart))  {step = stepStart+1;}
	switch (step)
	{
		case -7:
		{
			LIS3DH_init_spi();
			step = -6;
			break;
		}
		case -6:
		{
			//ret = LIS3DH_presCheck();
			if (1 == LIS3DH_presCheck())
			{
				step = -5;
			}
			else
			{
				return 1;		//1 = failure
			}
			break;
		}
		case -5:
		{
			spiLIS3DH_senddata(CtrlReg5,   0b10000000); // reboot memory content
			step = -4;
			break;
		}
		case -4:
		{
			spiLIS3DH_senddata(CtrlReg4,   0b10001100); // BDU BLE FS1 FS0 HR ST1 ST0 SIM
			step = -3;
			break;
		}
		case -3:
		{
			spiLIS3DH_senddata(CtrlReg1,  (uint8_t) 0b10010111); //
			step = -2;
			break;
		}
		case -2:
		{
			uint8_t LIS3DH_Reg4 = 0b10001000;			// BDU BLE FS1 FS0 HR ST1 ST0 SIM
#ifdef XYZopt
			LIS3DH_Reg4 |=        0b01000000;			// enable to switch High and Low Byte order. special Hint: High Byte first for a fast read sequence to array of int16
#endif
			spiLIS3DH_senddata(CtrlReg4,  LIS3DH_Reg4); // BDU BLE FS1 FS0 HR ST1 ST0 SIM
			step = -1;
			break;
		}
		case -1:
		{
			spiLIS3DH_senddata(ADCConfReg,(uint8_t) 0b11000000); // ADC Temp 0 0 0 0 0 0

			step = 0;
			break;
		}
		default:
		{
			step = -7;
		}
	}
	return step;
}


#ifdef XYZopt
int8_t spiLIS3DH_XYZ(int16_t *xyz, int8_t enable_Temp)
{
	int8_t Temp[2] = {0,0};
	if (enable_Temp)
	{
		spiReadRegBurst(spi, LIS3DH_CS_PORT, LIS3DH_CS, (ADC3Temp|spiRead), (uint8_t *) Temp, 2);
	}
	spiReadRegBurst(spi, LIS3DH_CS_PORT, LIS3DH_CS, spiOUTxyz,(uint8_t *) xyz, 6);

	return Temp[1];
}

#else
int8_t spiLIS3DH_XYZ(int16_t *xyz, int8_t enable_Temp)
{
	int8_t Temp = 0;
	uint16_t *pvalue;
	pvalue = (int16_t *)xyz;
	Temp = (int8_t) spiLIS3DH_getRegData(ADC3Temp+1);
		*pvalue++ = spiLIS3DH_getRegWord(spiOUTxyz);
		*pvalue++ = spiLIS3DH_getRegWord(spiOUTxyz+2);
		*pvalue   = spiLIS3DH_getRegWord(spiOUTxyz+4);

	return Temp;
}
#endif

/**
- enable the bits 6 and 7 in TEMP_CFG_REG(1Fh): enable aux ADC and enable temperature sensor.
- enable Block Data Update, BDU, feature. CTRL_REG4(0x23) , BDU (bit#7)=1.
- read both the ADC output 3 registers (because of BDU enabled): OUT_ADC_3_L(0x0C) and OUT_ADC_3_H(0x0D).
Useful bits: 8, left aligned, hence useful data in OUT_ADC_3_H.
Temperature sensor output change vs temperature: 1digit/degrCelsius
*/
int8_t spiLIS3DH_Temp(void)
{
	int8_t Temp[2];
	spiReadRegBurst(spi, LIS3DH_CS_PORT, LIS3DH_CS, (ADC3Temp|spiRead), (uint8_t *) Temp, 2); //work only at the second read
	//spiWriteByte(spi, LIS3DH_CS_PORT, LIS3DH_CS, (ADC3Temp|spiRead)); //work fine
	//Temp[1]=spiReadByte(spi, LIS3DH_CS_PORT, LIS3DH_CS);  don't work
	//Temp[1] = spiLIS3DH_getRegData(spiADC3Temp);
	return Temp[1];
}



