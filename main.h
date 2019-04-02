#ifndef __MAIN_H
#define __MAIN_H

void Delay_ms(volatile uint32_t nTime);
void TimingDelay_Decrement(void);
void LCD_DrawCross(u16 X, u16 Y, u8 show);
u32 getTimeMs();
void SetTrace();
void ResetTrace();

/*
 * TP IRQ
 * TP SCK
 * TP SO
 * TP SI
 * TP CS
 */

#endif
