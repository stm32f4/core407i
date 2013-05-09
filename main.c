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
#include "5110.h"
#include "main.h"
#include "ascii.h"

/* Private typedef -----------------------------------------------------------*/
GPIO_InitTypeDef GPIO_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;

/* Private variables ---------------------------------------------------------*/
static volatile uint32_t TimingDelay;
char* frequency[5];

/* Private function prototypes -----------------------------------------------*/
void Delay(volatile uint32_t nCount);
void TimingDelay_Decrement(void);
void Animate();
void Animate2();

/**
 * @brief  Nokia 5110 LCD demo
 * @param  None
 * @retval None
 */
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

	/* GPIOA Periph clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	/* Configure PA1 as output pushpull mode */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Init LCD */
	LCD5110_init();
	LCD5110_Led(0);

	/* Display sample text */
	LCD5110_set_XY(0, 0);
	LCD5110_write_string("Core 407 12/04");
	LCD5110_write_string("              ");
	LCD5110_write_string("Nokia 5110 LCD");
	LCD5110_write_string("              ");
	getDecimalFromShort((char*) frequency, (u16) (SystemCoreClock / 1000000));
	LCD5110_write_string(" ");
	LCD5110_write_string((char*) frequency);
	LCD5110_write_string(" Mhz    ");

	/* Start animation bar */
	while (1) {
		Animate2();
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
void Delay(volatile uint32_t nTime) {
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

void Animate() {
	u8 i = 0;
	u8 j = 0;
	u8 val = 0;
	for (i = 3; i < 8; i++) {
		val = 0x01 << i;
		Delay(150);
		for (j = 0; j < 84; j++) {
			LCD5110_set_pos(j, 5);
			LCD5110_write_byte(val);
		}
	}
	for (i = 6; i > 0; i--) {
		val = 0x01 << i;
		Delay(150);
		for (j = 0; j < 84; j++) {
			LCD5110_set_pos(j, 5);
			LCD5110_write_byte(val);
		}
	}
	for (i = 0; i < 4; i++) {
		val = 0x01 << i;
		Delay(150);
		for (j = 0; j < 84; j++) {
			LCD5110_set_pos(j, 5);
			LCD5110_write_byte(val);
		}
	}
}

void Animate2() {
	u8 i = 0;
	u8 on = 0b11110000;
	u8 off = 0b00000000;
	for (i = 0; i < 83; i++) {
		LCD5110_set_pos(i, 5);
		LCD5110_write_byte(on);
		Delay(15);
	}
	for (i = 0; i < 83; i++) {
		LCD5110_set_pos(i, 5);
		LCD5110_write_byte(off);
		Delay(15);
	}
	for (i = 83; i > 0; i--) {
		LCD5110_set_pos(i, 5);
		LCD5110_write_byte(on);
		Delay(15);
	}
	for (i = 83; i > 0; i--) {
		LCD5110_set_pos(i, 5);
		LCD5110_write_byte(off);
		Delay(15);
	}
}
/************************END OF FILE*******************************/
