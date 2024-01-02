/**
 * LIS3DH
 *
 *  Created on: 28.12.2023
 *      Author: Flämig, Tobias
 *       Email: flaemig@dhbw-stuttgart.de
 */

#ifndef LIS3DH_H_
#define LIS3DH_H_

#include <stm32f4xx.h>


/****
 *
 * CS  - ChipSelect		PA8
 * MOSI -				PA7
 * MISO					PA6
 * CLK					PA5
 *
 */

#define LIS3DH_SPI_PORT		GPIOA
#define LIS3DH_CLK			PIN5				// Arduino D13
#define LIS3DH_MOSI			PIN7				// Arduino D11
#define LIS3DH_MISO			PIN6				// Arduino D12

//EXT_CS   PBA
#define LIS3DH_CS_PORT		GPIOA
#define LIS3DH_CS			PIN8				// Arduino D7
#define LIS3DH_CS1   gpioSetPin(GPIOA, PIN8)	// CS disable
#define LIS3DH_CS0   gpioResetPin(GPIOA, PIN8)


extern int8_t spiLIS3DH_init(int8_t restart);
extern uint8_t spiLIS3DH_getRegData(uint8_t reg);
extern uint16_t spiLIS3DH_getRegWord(uint8_t reg);
extern int8_t spiLIS3DH_XYZ(int16_t *xyz, int8_t enable_Temp);

// Value is not the absolut Temp, it is only the delta from temp at calibration process
extern int8_t spiLIS3DH_Temp(void);



#define MAX7219_CS_PORT     (GPIOA)
#define MAX7219_CS          (PIN8)
#define MAX7219_MOSI        (PIN7)
#define MAX7219_CLK         (PIN5)

typedef enum
{
    // Use this when using 8-bit transfer
    REG_NOOP        =  0,
    REG_DIG0,
    REG_DIG1,
    REG_DIG2,
    REG_DIG3,
    REG_DIG4,
    REG_DIG5,
    REG_DIG6,
    REG_DIG7,
    REG_DECODE,
    REG_INTENSITY,
    REG_SCAN_LIMIT,
    REG_SHUTDOWN,
    REG_DISPTEST    = 15,

    // Use this when using 16-bit transfer
    // Addresses are then left-shifted by 8
    REG_NOOP_SL8        =  (0 << 8),
    REG_DIG0_SL8        =  (1 << 8),
    REG_DIG1_SL8        =  (2 << 8),
    REG_DIG2_SL8        =  (3 << 8),
    REG_DIG3_SL8        =  (4 << 8),
    REG_DIG4_SL8        =  (5 << 8),
    REG_DIG5_SL8        =  (6 << 8),
    REG_DIG6_SL8        =  (7 << 8),
    REG_DIG7_SL8        =  (8 << 8),
    REG_DECODE_SL8      =  (9 << 8),
    REG_INTENS_SL8      = (10 << 8),
    REG_SCAN_LIMIT_SL8  = (11 << 8),
    REG_SHUTDOWN_SL8    = (12 << 8),
    REG_DISPTEST_SL8    = (15 << 8)

} MAX7219_REG_t;

typedef enum
{
    INT_MIN         = 0,
    INT_1,
    INT_2,
    INT_3,
    INT_4,
    INT_5,
    INT_6,
    INT_7,
    INT_8,
    INT_9,
    INT_10,
    INT_11,
    INT_12,
    INT_13,
    INT_14,
    INT_MAX
} MAX7219_INTENSITY_t;

typedef enum
{
    TEST_OFF        = 0,
    TEST_ON
} MAX7219_TESTMODE_t;

typedef enum
{
    SHUTDOWN = 0,
    NORMAL_OP
} MAX7219_OPMODE_t;

typedef enum
{
    NO_DECODE           = 0,
    DECODE_DIG0         = 1,
    DECODE_DIG0_TO_DIG3 = 0x0F,
    DECODE_ALL          = 0xFF
} MAX7219_DECODE_t;

//extern void max7219Init(SPI_TypeDef *spi);
extern void max7219Test(SPI_TypeDef *spi, MAX7219_TESTMODE_t testMode);
extern void max7219Shutdown(SPI_TypeDef *spi);
extern void max7219TurnOn(SPI_TypeDef *spi);
extern void max7219SetDecodeMode(SPI_TypeDef *spi, MAX7219_DECODE_t mode);
extern void max7219TestDigits(SPI_TypeDef *spi, uint8_t value);
extern void max7219ResetDigits(SPI_TypeDef *spi, uint8_t value);
extern void max7219Clear(SPI_TypeDef *spi);

#endif /* LIS3DH_H_ */
