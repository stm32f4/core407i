/**
 *****************************************************************************
 * @title   main.c
 * @author  Goofie31
 * @date    14 April 2013
 * @brief
 *******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "main.h"
#include "GLCD.h"
#include "AD7846.h"
#include "stdio.h"

#define _RED 0xD0
#define _GREEN 0xC0
#define _BLUE 0x00

/* Private typedef -----------------------------------------------------------*/
GPIO_InitTypeDef GPIO_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;

/* Private variables ---------------------------------------------------------*/
static volatile uint32_t TimingDelay;
char* frequency[5];
u8 volatile touchedOn = 0;
u8 volatile touchedOff = 0;
u32 msCounter = 0;

/* Private function prototypes -----------------------------------------------*/
void Delay_ms(volatile uint32_t nCount);
void TimingDelay_Decrement(void);
void TestScroll();
void TestFill(uint16_t index);
void HSVtoRGB(float *r, float *g, float *b, float h, float s, float v);
void InitSystick();
void trackTouch(TP_EVENT e);
void InitTrace();

int main(void) {

	InitSystick();
	InitTrace();
	Delay_ms(500);
	LCD_Init();
	LCD_SetBacklight(0xA0);

	LCD_FillArea(0, 0, 319, 40, RGB565CONVERT(_RED, _GREEN, _BLUE));
	LCD_DrawLine(0, 0, 319, 0, 0xFFFF);
	LCD_DrawLine(0, 40, 319, 40, 0xFFFF);
	LCD_DrawLine(0, 0, 0, 40, 0xFFFF);
	LCD_DrawLine(319, 0, 319, 40, 0xFFFF);
	GUI_Text(10, 10, "3.5 TFT LCD with SSD1963   R(02/05/14)\0", Black, RGB565CONVERT(_RED, _GREEN, _BLUE));
	LCD_FillArea(0, 41, 319, 239, Black);
	TC_InitSPI();
	TC_SetTouchCallBack(trackTouch);

	u8 cnt = 20;
	GUI_Text(0, 120, "Touch the screen to start calibration.", Black, White);
	TC_set_interrupt(1);
	while (cnt > 0) {
		Delay_ms(100);
		if (touchedOn == 1) {
			Delay_ms(200);
			TS_Calibrate(320, 240);
			touchedOn = 0;
			break;
		}
		cnt--;
	}
	LCD_FillArea(0, 41, 319, 239, Black);

	char Text[15] = "         ";
	u16 index = 0;
	while (1) {
		if (touchedOn == 1) {
			if (Pen_Point.X > 0 && Pen_Point.Y > 0) {
				LCD_FillArea(0, 0, 319, 239, Black);
				sprintf(Text, "X : %1d   Y : %1d", Pen_Point.X0, Pen_Point.Y0);
				GUI_Text(50, 1, Text, White, Black);
				if (Pen_Point.X0 > 0 && Pen_Point.X0 < 320 && Pen_Point.Y0 > 0 && Pen_Point.Y0 < 240) {
					LCD_FillArea(Pen_Point.X0 - 5, Pen_Point.Y0 - 5, Pen_Point.X0 + 5, Pen_Point.Y0 + 5, White);
				} else {
					LCD_SetPoint(Pen_Point.X0, Pen_Point.Y0, White);
				}
			}
			touchedOn = 0;
		} else {
			//TestFill(index);
			TestScroll();
			index++;
			if (index == 360)
				index = 0;
		}
	}
}

void TestFill(uint16_t index) {
	float red;
	float green;
	float blue;
	float sat = 1;
	float val = 0.8;
	uint16_t width = 5;
	uint16_t start = 0;

	if (index == 360) {
		index = 0;
	}
	/* Wait VSync period */
	while (!GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4))
		;

	HSVtoRGB(&red, &green, &blue, (float) index, sat, val);

	LCD_FillArea(0, start, width, 239,
			RGB565CONVERT((uint8_t )(red * 255 + 0.5), (uint8_t )(green * 255 + 0.5), (uint8_t )(blue * 255 + 0.5)));
}

void TestScroll() {
	static uint16_t index;
	static uint16_t step = 1;
	uint8_t line;
	uint8_t thick = 16;
	uint8_t colStep = 128 / thick;
	uint16_t colR = 0x7F;
	uint16_t colG = 0x00;
	uint16_t colB = 0x7F;
	uint16_t last = 0;
	uint16_t start = 0;
	uint16_t width = 5;
	uint16_t end = 239 - thick;
	// Wait if in display period
	while (!GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4)) {
	}

	// Start of non display period
	GPIO_SetBits(GPIOC, GPIO_Pin_3);
	if (index == start) {
		last = end;
	} else {
		last = index - step;
	}
	LCD_FillArea(0, last, width, last + thick, Black);
	for (line = 0; line < thick + 1; line++) {
		LCD_FillArea(0, index + line, width, index + line,
				RGB565CONVERT(colR + line * colStep, colG + line * colStep, colB + line * colStep));
	}

	for (line = 0; line < 255; line++) {
		RGB565CONVERT(colR + line * colStep, colG + line * colStep, colB + line * colStep);
	}

	//Wait next display period
	while (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4)) {
	}

	index += step;
	if (index == end || index == start) {
		step = -step;
	}
}
/**
 * @brief  This function handles SysTick Handler.
 * @param  None
 * @retval None
 */
//void SysTick_Handler(void) {
//	TimingDelay_Decrement();
//}
/**
 * @brief  Inserts a delay time.
 * @param  nTime: specifies the delay time length, in milliseconds.
 * @retval None
 */
void Delay_ms(volatile uint32_t nTime) {
	TimingDelay = nTime;
	while (TimingDelay != 0)
		;
}

/**
 * @brief  Decrements the TimingDelay variable.
 * @param  None
 * @retval None
 */
void TimingDelay_Decrement(void) {
	if (TimingDelay > 0x00) {
		TimingDelay--;
	}
	msCounter++;
}

void HSVtoRGB(float *r, float *g, float *b, float h, float s, float v) {
	int i;
	float f, p, q, t;
	if (s == 0) {
		// achromatic (grey)
		*r = *g = *b = v;
		return;
	}
	h /= 60; // sector 0 to 5
	i = floor(h);
	f = h - i; // factorial part of h
	p = v * (1 - s);
	q = v * (1 - s * f);
	t = v * (1 - s * (1 - f));
	switch (i) {
	case 0:
		*r = v;
		*g = t;
		*b = p;
		break;
	case 1:
		*r = q;
		*g = v;
		*b = p;
		break;
	case 2:
		*r = p;
		*g = v;
		*b = t;
		break;
	case 3:
		*r = p;
		*g = q;
		*b = v;
		break;
	case 4:
		*r = t;
		*g = p;
		*b = v;
		break;
	default: // case 5:
		*r = v;
		*g = p;
		*b = q;
		break;
	}
}

void trackTouch(TP_EVENT e) {
	if (e == on) {
		touchedOn = 1;
	} else if (e == off) {
		touchedOff = 1;
	} else {
		touchedOn = 0;
		touchedOff = 0;
	}
}

void LCD_DrawCross(u16 X, u16 Y, u8 show) {
	LCD_Clear(White);
	if (show == 1) {
		LCD_DrawLine(X - 3, Y, X + 3, Y, Black);
		LCD_DrawLine(X, Y - 3, X, Y + 3, Black);
	}
	return;
}

void InitSystick() {
	/* Set Systick to 1 ms */
	if (SysTick_Config(SystemCoreClock / 1000))
		while (1)
			;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	NVIC_InitStructure.NVIC_IRQChannel = SysTick_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;	// Not used
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	NVIC_SetPriorityGrouping(0x03);
	NVIC_SetPriority(SysTick_IRQn, 0);
}

u32 getTimeMs() {
	return msCounter;
}

void InitTrace() {
	GPIO_InitTypeDef GPIO_InitStructure;

	// Enable GPIE
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	ResetTrace();
}

void SetTrace() {
	GPIO_SetBits(GPIOE, GPIO_Pin_3);
}

void ResetTrace() {
	GPIO_ResetBits(GPIOE, GPIO_Pin_3);
}
/************************END OF FILE*******************************/
