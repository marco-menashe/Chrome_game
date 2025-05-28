#define _KEY_C

#include "main.h"
#include "KEYPAD.h"
#include "Timer.h"
#include <string.h>
#include <stdio.h>

#define Number_of_Keys 12
#define Number_of_Cols  3
#define PA0 0x0001
#define PA1 0x0002
#define PA4 0x0010
#define PB0 0x0001
#define PC1 0x0002
#define PC0 0x0001
#define PA10 0x0400

//flag for main
#define POUND_DETECT 0x0008

#define KeyDetect 			0x0001
#define KeyLow2High 		0x0002
#define KeyRepeat			0x0004  // after a key is pressed & held for one second
#define KeyToBeRepeated 	0x0008

typedef struct
{
   unsigned short sKeyRead;
   unsigned short sKeyReadTempPos;
   unsigned short sKeySend;
   unsigned short sKeyCol;
   char KeyLetter;
   unsigned short sKeyCommand;
} Key_Contorl_struct_t;

typedef enum KeyName
{
  ONE_command,
  FOUR_command,
  SEVEN_command,
  STAR_command,
  TWO_command,
  FIVE_command,
  EIGHT_command,
  ZERO_command,
  THREE_command,
  SIX_command,
  NINE_command,
  POUND_command
} KeyName;

typedef enum TimeIDX {
	AM,
	PM
} TimeIDX;

/******** Structure *******/
Key_Contorl_struct_t sKeyControl[Number_of_Keys]
={
 {PA10,0x8,PA4,0,'1',ONE_command},     // PA10 (read), PA4 (send)
 {PC0,0x4,PA4,0,'4',FOUR_command},     // PC0 (read), PA4 (send)
 {PC1,0x2,PA4,0,'7',SEVEN_command},    // PC1 (read), PA4 (send)
 {PB0,0x1,PA4,0,'*',STAR_command},     // PB0 (read), PA4 (send)

 {PA10,0x8,PA1,1,'2',TWO_command},     // PA10 (read), PA1 (send)
 {PC0,0x4,PA1,1,'5',FIVE_command},     // PC0 (read), PA1 (send)
 {PC1,0x2,PA1,1,'8',EIGHT_command},    // PC1 (read), PA1 (send)
 {PB0,0x1,PA1,1,'0',ZERO_command},     // PB0 (read), PA1 (send)

 {PA10,0x8,PA0,2,'3',THREE_command},   // PA10 (read), PA0 (send)
 {PC0,0x4,PA0,2,'6',SIX_command},      // PC0 (read), PA0 (send)
 {PC1,0x2,PA0,2,'9',NINE_command},     // PC1 (read), PA0 (send)
 {PB0,0x1,PA0,2,'#',POUND_command}     // PB0 (read), PA0 (send)
};

unsigned short sKeyStatus;
unsigned short sKeyCurrentCol[Number_of_Cols];
unsigned short sKeyDebouncedCol[Number_of_Cols];
unsigned short sKeyIssued;
unsigned short sIndexCopy;

unsigned short sKeyPreviousCol[Number_of_Cols];
unsigned short sKeyLow2HighCol[Number_of_Cols];

volatile uint8_t seconds = 0;
volatile uint8_t minutes = 0;
volatile uint8_t hours = 0;
volatile TimeIDX timeIDX = AM;
char* times[] = {"AM", "PM"};

unsigned short key1();
unsigned short key2();
unsigned short key3();
unsigned short key4();
unsigned short key5();
unsigned short key6();
unsigned short key7();
unsigned short key8();
unsigned short key9();
unsigned short key0();
unsigned short keyS();
unsigned short keyP();

unsigned short key1R();
unsigned short key2R();
unsigned short key3R();
unsigned short key4R();
unsigned short key5R();
unsigned short key6R();
unsigned short key7R();
unsigned short key8R();
unsigned short key9R();
unsigned short key0R();
unsigned short keySR();
unsigned short keyPR();


void initKeypad() {
	// Clear all debounced records, Previous, Low2High
	unsigned short sIndex;
	for (sIndex=0; sIndex<Number_of_Cols; sIndex++)
	{
		sKeyDebouncedCol[sIndex] = 0x0000;
		sKeyPreviousCol[sIndex] = 0x0000;
		sKeyLow2HighCol[sIndex] = 0x0000;
	}
}

char* IncrementClock(char* buffer, int buflen) {
	seconds++;
	if (seconds >= 60) {
	    seconds = 0;
	    minutes++;
	    if (minutes >= 60) {
	        minutes = 0;
	        hours = (hours + 1) % 24;
	    }
	}
	snprintf(buffer, buflen-1, "%02d:%02d:%02d %s", hours, minutes, seconds, times[timeIDX]);
	return buffer;
}


void Keypadscan()
{
    unsigned short sIndex;
    unsigned short Temp;

    // Clear all key records
    for (sIndex=0; sIndex<Number_of_Cols; sIndex++)
    {
      sKeyCurrentCol[sIndex] = 0x00;
    }

    // Read all 3 column
    for (sIndex=0; sIndex<Number_of_Keys; sIndex++)
    {
      GPIOA->ODR &=~(PA4 | PA1 | PA0);
      GPIOA->ODR |= sKeyControl[sIndex].sKeySend;
      HAL_Delay(0.5);

      switch (sKeyControl[sIndex].sKeyCommand)
	  {
      	  case ONE_command:
      	  case TWO_command:
      	  case THREE_command:
      		if (GPIOA->IDR & sKeyControl[sIndex].sKeyRead)
      		  sKeyCurrentCol[sKeyControl[sIndex].sKeyCol]= sKeyControl[sIndex].sKeyReadTempPos;
      		break;

      	  case FOUR_command:
      	  case FIVE_command:
      	  case SIX_command:
      	  case SEVEN_command:
      	  case EIGHT_command:
      	  case NINE_command:
        	if (GPIOC->IDR & sKeyControl[sIndex].sKeyRead)
        	  sKeyCurrentCol[sKeyControl[sIndex].sKeyCol] = sKeyControl[sIndex].sKeyReadTempPos;
      	    break;

      	  case STAR_command:
      	  case ZERO_command:
      	  case POUND_command:
      		if (GPIOB->IDR & sKeyControl[sIndex].sKeyRead)
      		  sKeyCurrentCol[sKeyControl[sIndex].sKeyCol] = sKeyControl[sIndex].sKeyReadTempPos;
	  }
    }

    // Check if a key is steadily read
    for (sIndex=0; sIndex<Number_of_Cols; sIndex++)
    {
      if ((sKeyCurrentCol[sIndex] == sKeyDebouncedCol[sIndex]) && (sKeyCurrentCol[sIndex] != 0x0000))
        break;
    }

    if (sIndex <Number_of_Cols)
    {
    	// Check for push on/ push off (Low To High)
    	for (sIndex=0; sIndex<Number_of_Cols; sIndex++)
    	{
    		Temp = sKeyCurrentCol[sIndex] ^ sKeyPreviousCol[sIndex];
    		sKeyLow2HighCol[sIndex] = (sKeyCurrentCol[sIndex] & Temp);
    	}

    	// Update Previous records
    	for (sIndex=0; sIndex<Number_of_Cols; sIndex++)
    	{
    	    sKeyPreviousCol[sIndex] = sKeyCurrentCol[sIndex];
    	}

      // Find which key is JUST depressed (Low To High) or KeyRepeat detected
       for (sIndex=0 ; sIndex<Number_of_Keys; sIndex++)
       {
         if (sKeyLow2HighCol[sKeyControl[sIndex].sKeyCol] & sKeyControl[sIndex].sKeyReadTempPos)
         {
           sKeyIssued = sKeyControl[sIndex].sKeyCommand;
           sKeyStatus |= (KeyDetect | KeyLow2High);
           sTimer[KEY_WAIT_REPEAT_TIMER] = KEY_WAIT_REPEAT_TIME;
           sKeyStatus |= KeyRepeat;		// a new key comes in, set the repeat flag
           sIndexCopy = sIndex;			// save a copy of sIndex for push & held use
           break;
         }
         else if ((sKeyStatus & KeyRepeat) && (sTimer[KEY_WAIT_REPEAT_TIMER]==0))
         {
           if (sTimer[KEY_REPEAT_TIMER] == 0)
           {
              sKeyIssued = sKeyControl[sIndexCopy].sKeyCommand;
              sKeyStatus |= (KeyDetect | KeyToBeRepeated);
              sTimer[KEY_REPEAT_TIMER] = KEY_REPEAT_TIME;
           }
         }
         else
         sKeyIssued = 0xFFFF;
       }
    }
    else
    {
      sKeyStatus &= ~(KeyDetect | KeyLow2High | KeyToBeRepeated | KeyRepeat);
      sTimer[KEY_REPEAT_TIMER] = 0;  // Reset repeat timer if no key

      for (sIndex=0; sIndex<Number_of_Cols; sIndex++)
       	  sKeyPreviousCol[sIndex] = 0;
    }


    // Transfer Current reading to debounced reading
    for (sIndex=0; sIndex<Number_of_Cols; sIndex++)
    {
      sKeyDebouncedCol[sIndex] = sKeyCurrentCol[sIndex];
      sKeyLow2HighCol[sIndex] = 0;
    }
}


unsigned short KeyProcess()
{
	uint16_t sIndex;

	if ((sKeyStatus & KeyDetect) && (sKeyIssued != 0xFFFF))
	{
		switch (sKeyIssued)
	    {
	    	case ONE_command:
	    	{
	    		if (sKeyStatus & KeyLow2High) {
	    			return key1();
	    		}
	    		else if (sKeyStatus & KeyToBeRepeated)
	    			return key1R();

	    		break;
	    	}
	        case FOUR_command:
	    	{
	    		if (sKeyStatus & KeyLow2High)
	    			return key4();
	    		else if (sKeyStatus & KeyToBeRepeated)
	    			return key4R();
	    		break;
	    	}

	        case SEVEN_command:
	    	{
	    		if (sKeyStatus & KeyLow2High)
	    			return key7();
	    		else if (sKeyStatus & KeyToBeRepeated)
	    			return key7R();
	    		break;
	    	}

	        case STAR_command:
	    	{
	    		if (sKeyStatus & KeyLow2High)
	    			return keyS();
	    		else if (sKeyStatus & KeyToBeRepeated)
	    			return keySR();
	    		break;
	    	}

	        case TWO_command:
	    	{
	    		if (sKeyStatus & KeyLow2High)
	    			return key2();
	    		else if (sKeyStatus & KeyToBeRepeated)
	    			return key2R();
	    		break;
	    	}

	        case FIVE_command:
	    	{
	    		if (sKeyStatus & KeyLow2High)
	    			return key5();
	    		else if (sKeyStatus & KeyToBeRepeated)
	    			return key5R();
	    		break;
	    	}

	        case EIGHT_command:
	    	{
	    		if (sKeyStatus & KeyLow2High)
	    			return key8();
	    		else if (sKeyStatus & KeyToBeRepeated)
	    			return key8R();
	    		break;
	    	}

	        case ZERO_command:
	    	{
	    		if (sKeyStatus & KeyLow2High)
	    			return key0();
	    		else if (sKeyStatus & KeyToBeRepeated)
	    			return key0R();
	    		break;
	    	}

	        case THREE_command:
	    	{
	    		if (sKeyStatus & KeyLow2High)
	    			return key3();
	    		else if (sKeyStatus & KeyToBeRepeated)
	    			return key3R();
	    		break;
	    	}

	        case SIX_command:
	    	{
	    		if (sKeyStatus & KeyLow2High)
	    			return key6();
	    		else if (sKeyStatus & KeyToBeRepeated)
	    			return key6R();
	    		break;
	    	}

	        case NINE_command:
	    	{
	    		if (sKeyStatus & KeyLow2High)
	    			return key9();
	    		else if (sKeyStatus & KeyToBeRepeated)
	    			return key9R();
	    		break;
	    	}

	        case POUND_command:
	        {
	        	if (sKeyStatus & KeyLow2High)
	        		return keyP();
	        	else if (sKeyStatus & KeyToBeRepeated)
	        		return keyPR();
	        	break;
	        }

            default:
            	return 0x0000;
        }

		sKeyStatus &= ~(KeyDetect | KeyLow2High | KeyToBeRepeated);

		// Clear all Low-2-High and High-2-Low records
		for (sIndex=0; sIndex<Number_of_Cols; sIndex++)
		  sKeyLow2HighCol[sIndex] = 0x0000;
	}
	return 0x0000;
}


unsigned short key1()
{
	// do nothing
	return 0x0000;
}

unsigned short key2()
{
	// do nothing
	return 0x0000;
}

unsigned short key3()
{
	// do nothing
	return 0x0000;
}

unsigned short key4()
{
	// do nothing
	return 0x0000;
}

unsigned short key5()
{
	// do nothing
	return 0x0000;
}

unsigned short key6()
{
	// do nothing
	return 0x0000;
}

unsigned short key7()
{
	// do nothing
	return 0x0000;
}

unsigned short key8()
{
	// do nothing
	return 0x0000;
}

unsigned short key9()
{
	return 0x0000;
}

unsigned short key0()
{
	return 0x0000;
}

unsigned short keyS()
{
	return 0x0000;
}

unsigned short keyP()
{
	return POUND_DETECT;
}

unsigned short key1R()
{
	return 0x0000;
}

unsigned short key2R()
{
	return 0x0000;
}

unsigned short key3R()
{
	return 0x0000;
}

unsigned short key4R()
{
	return 0x0000;
}

unsigned short key5R()
{
	return 0x0000;
}

unsigned short key6R()
{
	return 0x0000;
}

unsigned short key7R()
{
	return 0x0000;
}

unsigned short key8R()
{
	return 0x0000;
}

unsigned short key9R()
{
	return 0x0000;
}

unsigned short key0R()
{
	return 0x0000;
}

unsigned short keySR()
{
	return 0x0000;
}

unsigned short keyPR()
{
	return 0x0000;
}
