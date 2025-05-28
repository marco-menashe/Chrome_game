#ifndef INC_KEY_H_
#define INC_KEY_H_

#ifdef _KEY_C
   #define SCOPE
#else
   #define SCOPE extern
#endif

SCOPE void initKeypad(void);
SCOPE unsigned short KeyProcess();
SCOPE void Keypadscan();
SCOPE char* IncrementClock(char*, int);

#undef SCOPE
#endif /* INC_LCD_H_ */
