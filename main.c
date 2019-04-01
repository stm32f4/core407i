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

/* Private typedef -----------------------------------------------------------*/
GPIO_InitTypeDef GPIO_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;

/* Private variables ---------------------------------------------------------*/
static volatile uint32_t TimingDelay;
char* frequency[5];

/* Private function prototypes -----------------------------------------------*/
void Delay_ms(volatile uint32_t nCount);
void TimingDelay_Decrement(void);
void TestScroll();
void TestFill();
void HSVtoRGB(float *r, float *g, float *b, float h, float s, float v);

int main(void) {

	/* Set Systick to 1 ms */
	if (SysTick_Config(SystemCoreClock / 1000))
		while (1)
			;
	NVIC_InitStructure.NVIC_IRQChannel = SysTick_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* GPIOH Periph clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);
	/* Configure PH2 as output pushpull mode */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOH, &GPIO_InitStructure);

	Delay_ms(500);
	LCD_Init();
	LCD_SetBacklight(0xA0);
	LCD_Clear(Black);

#define _RED 0xD0
#define _GREEN 0xC0
#define _BLUE 0x00

	LCD_FillArea(0, 0, 319, 40, RGB565CONVERT(_RED ,_GREEN ,_BLUE));

	LCD_DrawLine(0, 0, 319, 0, 0xFFFF);
	LCD_DrawLine(0, 40, 319, 40, 0xFFFF);
	LCD_DrawLine(0, 0, 0, 40, 0xFFFF);
	LCD_DrawLine(319, 0, 319, 40, 0xFFFF);
	GUI_Text(10, 10, "3.5 TFT LCD with SSD1963   R(02/05/14)\0", Black,
			RGB565CONVERT(_RED ,_GREEN ,_BLUE));
	//GUI_Text(10, 26, "3.5 TFT LCD with SSD1963\0", White, Black);

	//TestScroll();
	TestFill();

	while (1) {
		Delay_ms(500);
		GPIO_ToggleBits(GPIOH, GPIO_Pin_2);
	}
}

void TestFill() {
	uint8_t count = 0;
	uint16_t index = 0;
	float red;
	float green;
	float blue;
	float sat = 1;
	float val = 0.8;

	while (1) {

		if (index == 360) {
			index = 0;
		}
		/* Wait VSync period */
		while (!GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4)) {
		}
		GPIO_SetBits(GPIOC, GPIO_Pin_3);

		HSVtoRGB(&red, &green, &blue, (float) index, sat, val);

		LCD_FillArea(0, 41, 319, 239,
				RGB565CONVERT((uint8_t)(red*255+0.5) ,(uint8_t)(green*255+0.5) ,(uint8_t)(blue*255+0.5) ));
//		LCD_FillArea(0, 61, 319, 80,
//				RGB565CONVERT(0x00+index,0xFF+index,0x00+index));
//		LCD_FillArea(0, 81, 319, 100,
//				RGB565CONVERT(0x00+index,0x00+index,0xFF+index));
//		LCD_FillArea(0, 101, 319, 120,
//				RGB565CONVERT(0xFF+index,0xFF+index,0x00+index));
//		LCD_FillArea(0, 121, 319, 140,
//				RGB565CONVERT(0xFF+index,0x00+index,0xFF+index));
//		LCD_FillArea(0, 141, 319, 160,
//				RGB565CONVERT(0x00+index,0xFF+index,0xFF+index));
//		LCD_FillArea(0, 161, 319, 180,
//				RGB565CONVERT(0xFF+index,0xFF+index,0xFF+index));
//		LCD_FillArea(0, 181, 319, 200,
//				RGB565CONVERT(0xFF+index,0xD0+index,0x80+index));
//		LCD_FillArea(0, 201, 319, 220,
//				RGB565CONVERT(0x80+index,0xD0+index,0xFF+index));
//		LCD_FillArea(0, 221, 319, 240,
//				RGB565CONVERT(0xD0+index,0xFF+index,0x80+index));

//Low pulse when processing is done
		GPIO_ResetBits(GPIOC, GPIO_Pin_3);
		for (count = 0; count < 255; count++) {
			RGB565CONVERT((uint8_t)(red*255+0.5), (uint8_t)(green*255+0.5),
					(uint8_t)(blue*255+0.5));
		}

		/* Wait end of Vsync */
		if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4)) {
			GPIO_SetBits(GPIOC, GPIO_Pin_3);
			while (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4)) {
			}
			GPIO_ResetBits(GPIOC, GPIO_Pin_3);
		}
		index++;
	}
}

void TestScroll() {
	uint16_t index;
	uint8_t line;
	uint8_t thick = 16;
	uint8_t colStep = 128 / thick;
	uint16_t colR = 0x7F;
	uint16_t colG = 0x00;
	uint16_t colB = 0x7F;
	uint16_t last = 0;
	uint16_t step = 1;
	uint16_t start = 41;
	uint16_t end = 239 - thick;
	index = start;
	while (1) {
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
		LCD_FillArea(0, last, 319, last + thick, Black);
		for (line = 0; line < thick + 1; line++) {
			LCD_FillArea(0, index + line, 319, index + line,
					RGB565CONVERT(colR + line * colStep ,colG + line * colStep ,colB + line * colStep));
		}

		// End of Data Processing
		GPIO_ResetBits(GPIOC, GPIO_Pin_3);

		//Add a low pulse at end of processing
		for (line = 0; line < 255; line++) {
			RGB565CONVERT(colR + line * colStep, colG + line * colStep,
					colB + line * colStep);
		}
		GPIO_SetBits(GPIOC, GPIO_Pin_3);

		//Wait next display period
		while (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4)) {
		}

		// Start of display period
		GPIO_ResetBits(GPIOC, GPIO_Pin_3);

		index += step;
		if (index == end || index == start) {
			step = -step;
		}
	}
}
/**
 * @brief  This function handles SysTick Handler.
 * @param  None
 * @retval None
 */
void SysTick_Handler(void) {
	TimingDelay_Decrement();
}

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

/************************END OF FILE*******************************/
