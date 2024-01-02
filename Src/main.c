/**
 * ST7735 TFT Display
 * LIS32DH Sensor per SPI interface read
 * ===========================
 *
 * Ansteuerung eines TFT Display ueber SPI.
 */

// Numeric display xyz values and calc of alpha and beta
// delete comment sign
//#define NumOut

#include <lis3dh_spi.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include <stm32f4xx.h>

#include <mcalSysTick.h>
#include <mcalGPIO.h>
#include <mcalSPI.h>

#include "hw_config.h"
#include "ST7735.h"
#include "tux_50_ad.h"



bool timerTrigger = false;


// Declaration  Timer1 = Main Prog
// 				ST7725_Timer delay Counter
uint32_t	Timer1 = 0UL;
uint32_t    ST7735_Timer = 0UL;
uint32_t	TaskTime = 300;							//tasktime default 300ms;

/* Private function prototypes -----------------------------------------------*/
void test_ascii_screen(void);
int8_t test_graphics(void);



int main(void)
{
	int testmode = 2;				// start with SPI and present test
	int stepSPI  = 1;				// first Status SPI
	int8_t ret;



	const float _pi=3.141;
#ifdef NumOut
	const float rad2deg = 57.295;
	char strOut[64];
#endif

	int16_t XYZ_raw[3];
	float X,Y,Z,alpha,beta;

	// grafic emulation
#define aDots 6							// Count of Dots for alpha line
#define bDots 12

	const int16_t a_midxyl[3] = {30,40,14};			// alpha circle xpos, ypos ,length
	const int16_t b_midxyl[3] = {100,40,38};		// beta circle xpos, ypos ,length
	const uint16_t aColor = ST7735_YELLOW;
	const uint16_t bColor = ST7735_MAGENTA; ;

	int16_t j, XYa[2][aDots], XYb[2][bDots];								// aDots Wertepaare für Winkeldarstellung alpha
	float xgrad, ygrad;

	// Variables for Oszi Function
	int16_t timepos = 0;
	const int16_t oszi[3] = {90,28,159};			//oszi ypos-Zero Level, y-amplitude, t_lenght
	const uint16_t osziColor = ST7735_GREY;

	//  	unsigned int r = 0;

    // Dies ist das Array, das die Adressen aller Timer-Variablen enthaelt.
       // Auch die Groesse des Arrays wird berechnet.
       uint32_t *timerList[] = { &Timer1, &ST7735_Timer /*, weitere Timer */ };
       size_t    arraySize = sizeof(timerList)/sizeof(timerList[0]);


    // Initialisiert den Systick-Timer
    systickInit(SYSTICK_1MS);
    systickSetMillis(&Timer1, 200);

    lcd7735_setup();
    LED_red_on;
    lcd7735_initR(INITR_REDTAB);
    lcd7735_setRotation(LANDSCAPE);
    //lcd7735_init_screen((uint8_t *)&SmallFont[0],ST7735_GREEN,ST7735_BLACK,LANDSCAPE); // not OK
    lcd7735_setFont((uint8_t *)&SmallFont[0]);
    LED_red_off;
    lcd7735_fillScreen(ST7735_BLACK);
    while (1)
    {
	   if (true == timerTrigger)
	   {
			systickUpdateTimerList((uint32_t *) timerList, arraySize);
	   }

	   if (isSystickExpired(Timer1))
	   {
		   LED_green_off;
		   systickSetTicktime(&Timer1, TaskTime);
		   switch (testmode)
		   {
		   	   case 1:
				{
					if (test_graphics() <= 0)
					{
						testmode = 2;
						TaskTime = 70;
					}
				break;
				}
		   	   case 2:
				{
					switch (stepSPI)
					{
						case 1:
						{
							ret = spiLIS3DH_init(0);
							if (ret > 0)										// no LIS3DH Sensor present
							{
								lcd7735_print("LIS3DH not Present ",0,0,0);
								TaskTime = 500;
								testmode = 1;
								stepSPI = 1;
							}
							if (ret == 0)										// LIS3DH init-procedure finished
							{
								lcd7735_print("(C)23Fl SPI LIS3DH ",0,0,0);
								TaskTime = 70;									// Tasktime for display 70ms
								stepSPI = 2;
							}
							break;

						}
						case 2:
						{
							LED_blue_on;
#ifdef NumOut
							int8_t Temp = spiLIS3DH_XYZ(XYZ_raw,1);
							//Temp = spiLIS3DH_Temp();
#else
							spiLIS3DH_XYZ(XYZ_raw,0);
#endif

							X = (float) XYZ_raw[0]/160;  //Skalierung 10mg/digit at +-2g
							Y = (float) XYZ_raw[1]/160;
							Z = (float) XYZ_raw[2]/160;
#ifdef NumOut
							sprintf(strOut, "%+2i X%+4i Y%+4i Z%+4i",Temp,(int)X,(int)Y,(int)Z);
							lcd7735_print((char *)strOut,0,110,0);
#endif
							alpha = atan(X/Z);
							if (Z<0)
							{
								if (X<0)  {alpha -=_pi;}
								else {alpha +=_pi;}
							}

							beta = atan(Y/Z);
							if (Z<0)
							{
								if (Y<0)  {beta -=_pi;}
								else {beta +=_pi;}
							}
#ifdef NumOut
							sprintf(strOut, "al:%+4i be:%+4i", (int)(alpha*rad2deg), (int)(beta*rad2deg));
							lcd7735_print((char *)strOut,0,80,0);
#else

							xgrad = cos(alpha)*a_midxyl[2];
							ygrad = sin(alpha)*a_midxyl[2];
							for (j = 0; j<aDots; j++ )
							{
								lcd7735_drawPixel(XYa[0][j],XYa[1][j],ST7735_BLACK);
								XYa[0][j] = lround(a_midxyl[0] + xgrad*(2*j - (float)aDots+1)/(aDots-1));			//(aDots-1) da j nur bis aDost -1 läuft
								XYa[1][j] = lround(a_midxyl[1] + ygrad*(2*j - (float)aDots+1)/(aDots-1));
								lcd7735_drawPixel(XYa[0][j],XYa[1][j],aColor);
							}
							//lcd7735_drawPixel(a_midxyl[0],a_midxyl[1],ST7735_RED);

							xgrad = cos(beta)*b_midxyl[2];
							ygrad = sin(beta)*b_midxyl[2];
							for (j = 0; j<bDots; j++ )
							{
								lcd7735_drawPixel(XYb[0][j],XYb[1][j],ST7735_BLACK);
								XYb[0][j] = lround(b_midxyl[0] + xgrad*(2*j - (float)bDots+1)/(bDots-1));			//(aDots-1) da j nur bis aDost -1 läuft
								XYb[1][j] = lround(b_midxyl[1] + ygrad*(2*j - (float)bDots+1)/(bDots-1));
								lcd7735_drawPixel(XYb[0][j],XYb[1][j],bColor);
							}



						// kleines Oszi als Zeitmitschrieb
							int16_t Ya = oszi[0] - (int16_t)(oszi[1] * alpha/_pi);			// - ST7735 y = 0 upper line inverter direct to y Scale
							int16_t Yb = oszi[0] - (int16_t)(oszi[1] * beta/_pi);
							int16_t osziHight = oszi[1]*2;
							lcd7735_drawFastVLine(timepos, (oszi[0]-oszi[1]), osziHight, osziColor);
							if (Ya == Yb)
							{
								lcd7735_drawPixel(timepos,Ya,ST7735_WHITE);
							}
							else
							{
								lcd7735_drawPixel(timepos,Ya,aColor);
								lcd7735_drawPixel(timepos,Yb,bColor);
							}
							if (++timepos > oszi[2] )
							{
								timepos = 0;
								//lcd7735_fillRect(0, oszi[0]-oszi[1], oszi[2]+1, 2*oszi[1], ST7735_GREY);
							}
							lcd7735_drawFastVLine(timepos, oszi[0], oszi[1]/2, ST7735_RED);

#endif
							LED_blue_off;


							//step = 3;
							break;
						}
						default:
						{
							stepSPI = 1;
						}
					}
				break;
				}
		   	    default:
				{
					testmode = 2;
				} break;
		   }
		   LED_green_on;
	   }

    }

    return 0;
}


int8_t test_graphics(void)
{

#define GrafikTests 21
static int8_t testcount = GrafikTests;
switch (testcount--)
	{
		case 21:
		{
			lcd7735_setRotation(LANDSCAPE);
			lcd7735_setFont((uint8_t *)&SmallFont[0]);
			lcd7735_print("Hi the 1st output",0,0,0);
		break;
		}
		case 20:
		{
			lcd7735_setFont((uint8_t *)&BigFont[0]);
			lcd7735_print("no SPI",10,20,0);
		break;
		}
		case 19:
		{
			lcd7735_setFont((uint8_t *)&SevenSegNumFont[0]);
			lcd7735_print("01234",0,60,0);
		break;
		}
		case 18:
		{
			lcd7735_setFont((uint8_t *)&BigFont[0]);
			lcd7735_fillScreen(ST7735_MAGENTA);
			lcd7735_print("no SPI",30,10,0);
		break;
		}
		case 17:	lcd7735_print("37deg noSPI",10,5,37);break;
		case 16:	lcd7735_drawBitmap(0,0,50,52,(bitmapdatatype)tux_50_ad,1);break;
		case 15:	lcd7735_drawBitmap(55,0,50,52,(bitmapdatatype)tux_50_ad,2);break;
		case 14:
		{
			lcd7735_setRotation(PORTRAIT);
			lcd7735_drawBitmap(0,0,50,52,(bitmapdatatype)tux_50_ad,1);
		break;
		}
		case 13:	lcd7735_invertDisplay(INVERT_ON);break;
		case 12:	lcd7735_invertDisplay(INVERT_OFF);break;
		case 11:	lcd7735_fillScreen(ST7735_RED);break;
		case 10:	lcd7735_fillScreen(ST7735_GREEN);break;
		case  9:	lcd7735_fillScreen(ST7735_BLUE);break;
		case  8:	lcd7735_fillScreen(ST7735_MAGENTA);break;
		case  7:	lcd7735_fillScreen(ST7735_BLACK); break;
		case  6:	lcd7735_drawRect(20,15,40,35,ST7735_BLUE);break;
		case  5:	lcd7735_fillCircle(70,70,30,ST7735_YELLOW);break;
		case  4:	lcd7735_drawRect(10,20,90,100,ST7735_MAGENTA);break;
		case  3:	lcd7735_drawCircle(60,120,35,ST7735_CYAN);break;
		case  2:	lcd7735_drawFastLine(10,5,110,100,ST7735_WHITE);break;
		case  1:	{ lcd7735_setRotation(LANDSCAPE);
					lcd7735_invertDisplay(INVERT_OFF);
					lcd7735_setFont((uint8_t *)&SmallFont[0]);}
					break;
		case  0:	lcd7735_invertDisplay(INVERT_OFF);break;
		default:
		{
			testcount = GrafikTests;
			lcd7735_fillScreen(ST7735_BLACK);
			break;
		}

	}
return  testcount;
}



