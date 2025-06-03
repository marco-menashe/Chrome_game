#ifndef INC_TIMER_H_
#define INC_TIMER_H_

#ifdef _TIMER_C
   #define SCOPE
#else
   #define SCOPE extern
#endif

#define NUMBER_OF_TIMERS 		4

#define KEY_SCAN_TIMER 			0
#define KEY_REPEAT_TIMER 		3
#define KEY_WAIT_REPEAT_TIMER 	2
#define LCD_SCROLL_TIMER 		1

#define LCD_SCROLL_TIME 		10000
#define KEY_SCAN_TIME 			10
#define KEY_REPEAT_TIME 		333
#define KEY_WAIT_REPEAT_TIME 	1000


SCOPE unsigned short sTimer[NUMBER_OF_TIMERS];

SCOPE void TIMER2_HANDLE(void);

#undef SCOPE
#endif /* INC_TIMER_H_ */
