/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "LCD.h"
#include "TIMER.h"
#include "main.h"

#define Number_of_Keys 12
#define Number_of_Cols 3
#define PA0 0x0001
#define PA1 0x0002
#define PA4 0x0010
#define PB0 0x0001
#define PC1 0x0002
#define PC0 0x0001
#define PA10 0x0400
#define KeyDetect 0x0001
#define KeyLow2High 0x0002
#define KeyRepeat 0x0004  // after a key is pressed & held for one second
#define KeyToBeRepeated 0x0008
#define Key1Pushed 0x0001
#define Key2Pushed 0x0002
#define Key3Pushed 0x0004
#define Key4Pushed 0x0008
#define Key5Pushed 0x0010
#define Key6Pushed 0x0020
#define Key7Pushed 0x0040
#define Key8Pushed 0x0080
#define Key9Pushed 0x0100
#define Key0Pushed 0x0201

#define CS GPIO_PIN_6
#define EEPROM_START 0x01
#define EEPROM_EWEN 0x3F
#define EEPROM_WRITE 0x40
#define EEPROM_EWDS 0x0F
#define EEPROM_READ 0x80

#define NUMCELLS 		32
#define ROWLENGTH 		16
#define JUMPFRAMES 		5
#define BOT_OBSTACLE	'O'
#define TOP_OBSTACLE	'V'

#define FULL_IDX		0
#define BOT_HALF_IDX	1
#define TOP_HALF_IDX	2
static int lives = 2;

//I2C
#define IODIRA 0x00
#define IODIRB 0x01
#define MCP_GPIOA 0x12
#define MCP_GPIOB 0x13

char I2C_ADDRESS = 0x40;

#define LIFE1	0x01  //
#define LIFE2	0x02  //
#define LIFE3   0x04  //

#define GAME_END		0x01

unsigned short flags = 0x00;
uint8_t EE_Data[3];

typedef struct {
    unsigned short sKeyRead;
    unsigned short sKeyReadTempPos;
    unsigned short sKeySend;
    unsigned short sKeyCol;
    char KeyLetter;
    unsigned short sKeyCommand;
} Key_Contorl_struct_t;

typedef struct {
	unsigned short row;
	unsigned short column;
	char state;
	bool environmentOccupied;
} environment_struct_t;

typedef struct {
	bool row0Occupied;
	bool row1Occupied;
	bool jumping;
	unsigned short jumpFrame;
	unsigned short topState;
	unsigned short botState;
} character_struct_t;

typedef enum KeyName {
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

void key1(void);
void key2(void);
void key3(void);
void key4(void);
void key5(void);
void key6(void);
void key7(void);
void key8(void);
void key9(void);
void key0(void);
void keyS(void);
void keyP(void);
void key1R(void);
void key2R(void);
void key3R(void);
void key4R(void);
void key5R(void);
void key6R(void);
void key7R(void);
void key8R(void);
void key9R(void);
void key0R(void);
void keySR(void);
void keyPR(void);

/******** Structure *******/
Key_Contorl_struct_t sKeyControl[Number_of_Keys] = {
    {PA10, 0x8, PA4, 0, '1', ONE_command},    // PA10 (read), PA4 (send)
    {PC0, 0x4, PA4, 0, '4', FOUR_command},    // PC0 (read), PA4 (send)
    {PC1, 0x2, PA4, 0, '7', SEVEN_command},   // PC1 (read), PA4 (send)
    {PB0, 0x1, PA4, 0, '*', STAR_command},    // PB0 (read), PA4 (send)
    {PA10, 0x8, PA1, 1, '2', TWO_command},    // PA10 (read), PA1 (send)
    {PC0, 0x4, PA1, 1, '5', FIVE_command},    // PC0 (read), PA1 (send)
    {PC1, 0x2, PA1, 1, '8', EIGHT_command},   // PC1 (read), PA1 (send)
    {PB0, 0x1, PA1, 1, '0', ZERO_command},    // PB0 (read), PA1 (send)
    {PA10, 0x8, PA0, 2, '3', THREE_command},  // PA10 (read), PA0 (send)
    {PC0, 0x4, PA0, 2, '6', SIX_command},     // PC0 (read), PA0 (send)
    {PC1, 0x2, PA0, 2, '9', NINE_command},    // PC1 (read), PA0 (send)
    {PB0, 0x1, PA0, 2, '#', POUND_command}    // PB0 (read), PA0 (send)
};


/****** Environment ******/
environment_struct_t envCells[NUMCELLS] = {
	{0, 0, ' ', false},
	{0, 1, ' ', false},
	{0, 2, ' ', false},
	{0, 3, ' ', false},
	{0, 4, ' ', false},
	{0, 5, ' ', false},
	{0, 6, ' ', false},
	{0, 7, ' ', false},
	{0, 8, ' ', false},
	{0, 9, ' ', false},
	{0, 10, ' ', false},
	{0, 11, ' ', false},
	{0, 12, ' ', false},
	{0, 13, ' ', false},
	{0, 14, ' ', false},
	{0, 15, ' ', false},
	{1, 0, ' ', false},
	{1, 1, ' ', false},
	{1, 2, ' ', false},
	{1, 3, ' ', false},
	{1, 4, ' ', false},
	{1, 5, ' ', false},
	{1, 6, ' ', false},
	{1, 7, ' ', false},
	{1, 8, ' ', false},
	{1, 9, ' ', false},
	{1, 10, ' ', false},
	{1, 11, ' ', false},
	{1, 12, ' ', false},
	{1, 13, ' ', false},
	{1, 14, ' ', false},
	{1, 15, ' ', false},
};


/****** Character ********/
character_struct_t character = {
	false,
	true,
	false,
	0,
	(unsigned short)' ',
	FULL_IDX,
};


// arrays for custom LCD characters:
uint8_t bottom_half[8] = {
	0b11111,
	0b11111,
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b00000
};

uint8_t top_half[8] = {
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b11111,
	0b11111
};

uint8_t full_rect[8] = {
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111
};


unsigned short sKeyStatus;
unsigned short sKeyCurrentCol[Number_of_Cols];
unsigned short sKeyDebouncedCol[Number_of_Cols];
unsigned short sKeyIssued;
unsigned short sIndexCopy;
unsigned short sKeyPushedRecord;
unsigned short sKeyPreviousCol[Number_of_Cols];
unsigned short sKeyLow2HighCol[Number_of_Cols];
char Txt[1];



I2C_HandleTypeDef hi2c1;
SPI_HandleTypeDef hspi1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim5;
ADC_HandleTypeDef hadc1;


void print_time(void);
void Keypadscan(void);
void KeyProcess(void);
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM5_Init(void);
static void MX_I2C1_Init(void);
static void MX_ADC1_Init(void);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) { TIMER2_HANDLE(); }

void Delay_1_plus_us(void);
char *EEPROM_READ_PASS(unsigned short);
void EEPROM_SEND(char EE_Addr, char EE_Data1, char EE_Data2);
void EEPROM_READ_FUN(uint8_t EE_Addr);

void updateCellsEnv(void);
void updatePlayerPos(void);
void updateLifeLEDs(int lives);
void I2CSend(char, char);

void resetHighScore(void);
uint32_t get_adc_seed(void);

int main(void) {
	HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_GPIO_Init();
    MX_TIM2_Init();
    MX_SPI1_Init();
    MX_TIM5_Init();
    MX_I2C1_Init();
    MX_ADC1_Init();

	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Base_Start(&htim5);

	char highScoreString[4];
	strncpy(highScoreString, EEPROM_READ_PASS(0x0C), sizeof(highScoreString) - 1);
	highScoreString[3] = '\0';  // Ensure null termination

	int highScore = atoi(highScoreString);
	int currentScore = 0;


	unsigned short sIndex;


    // Clear all debounced records, Previous, Low2High
    for (sIndex = 0; sIndex < Number_of_Cols; sIndex++) {
        sKeyDebouncedCol[sIndex] = 0x0000;
        sKeyPreviousCol[sIndex] = 0x0000;
        sKeyLow2HighCol[sIndex] = 0x0000;
    }

    LcdInit(); 				// LcdPutS("KeyPad LCD:");
    LcdWriteCmd(0x000C);  	// CURSOR OFF    LcdCreateChar(FULL_IDX, full_rect);
    LcdCreateChar(TOP_HALF_IDX, top_half);
    LcdCreateChar(BOT_HALF_IDX, bottom_half);
    LcdCreateChar(FULL_IDX, full_rect);

    // Reset and initialize I2C interface
    HAL_I2C_DeInit(&hi2c1);
    HAL_Delay(10);
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        // Display error message on LCD if I2C initialization failed
        LcdGoto(0, 0);
        LcdPutS("I2C Init Error");
        HAL_Delay(1000); // Give time to read the message
    }

    // configure i2c
    I2CSend(IODIRB, 0x00);      	// Make IO port_B, bit 0 to 7 all output
    I2CSend(MCP_GPIOB, (LIFE1|LIFE2|LIFE3));


    char topbuff[17] = "";
    char bottombuff[17] = "";

    char endMessage[17] = "--- Game Over --";
    char endScore[17];

    if (highScore < 0 || highScore > 99999)
    	highScore = 0;

    // Initialize life LEDs based on starting lives
    updateLifeLEDs(lives);

    // set random seed
    srand(get_adc_seed());

    while (1) {
    	// check for game end condition
    	if (flags & GAME_END) {
    		if(highScore < currentScore){
    		    char currentScoreString[11];
    		    sprintf(currentScoreString, "%d", currentScore);
    			EEPROM_SEND(0x0C,currentScoreString[0],currentScoreString[1]);					// Address at 0x0C is randomly chosen.
    			EEPROM_SEND(0x0E,currentScoreString[2],currentScoreString[3]);
    			EEPROM_SEND(0x010,currentScoreString[4],currentScoreString[5]);
    			EEPROM_SEND(0x012,'\0','\0');
    			highScore = currentScore;
    		}

    		LcdClear();
    		LcdGoto(0, 0);
    		LcdPutS(endMessage);
    		LcdGoto(1, 0);
    		snprintf(endScore, sizeof(endScore), "H: %d C: %d", highScore,  currentScore);
			LcdPutS(endScore);

    		return 0;
    	}

        // Check if need to scan and process keys
        if ((sTimer[KEY_SCAN_TIMER] == 0) && !(flags & GAME_END) ) {
            Keypadscan();
            KeyProcess();
            sTimer[KEY_SCAN_TIMER] = KEY_SCAN_TIME;
        }

        if ((sTimer[SCORE_TIMER] == 0)){
        	currentScore++;
        	sTimer[SCORE_TIMER] = SCORE_TIME;
        }

        // frame logic goes here
        if ((sTimer[LCD_SCROLL_TIMER] == 0) && !(flags & GAME_END)) {
        	// update the environment cells first
        	updateCellsEnv();

        	// update player position based on logic
        	updatePlayerPos();

        	// check for game end
        	if(character.row0Occupied && envCells[0].environmentOccupied)
        	{
                // lose one life
                lives--;
                // Update the life LEDs
                updateLifeLEDs(lives);
                if (lives == 1) {
                    // first death -> turn OFF the second LED (PA3)
                }
                else if (lives <= 0) {
                    // no more lives -> game over
                    flags |= GAME_END;
                }
                // clear that obstacle so we do not immediately collide again
                envCells[0].environmentOccupied = false;
                envCells[0].state = ' ';
                continue;
        	} else if (character.row1Occupied && envCells[ROWLENGTH].environmentOccupied) {
                // lose one life
                lives--;
                // Update the life LEDs
                updateLifeLEDs(lives);
                if (lives == 1) {
                    // first death -> turn OFF the second LED (PA3)
                }
                else if (lives <= 0) {
                    // no more lives -> game over
                    flags |= GAME_END;
                }
                // clear that obstacle so we do not immediately collide again
                envCells[ROWLENGTH].environmentOccupied = false;
                envCells[ROWLENGTH].state = ' ';
                continue;
        	}

        	// game hasn't ended so we update player on screen
        	// fill 0 with character if present otherwise with env
        	if (character.row0Occupied) {
        		LcdGoto(0,0);
        		LcdPutCh(character.topState);
        	} else {
        		LcdGoto(0,0);
        		LcdPutCh(envCells[0].state);
        	}
        	if (character.row1Occupied) {
        		LcdGoto(1,0);
        		LcdPutCh(character.botState);
        	} else {
        		LcdGoto(1,0);
        		LcdPutCh(envCells[ROWLENGTH].state);
        	}

            // now fill buffers with rest of environment and display
        	for (int i = 0; i < ROWLENGTH-1; i++) {
        		topbuff[i] = envCells[i+1].state;
        		bottombuff[i] = envCells[ROWLENGTH+i+1].state;
        	}

        	// end with null
        	topbuff[ROWLENGTH-1] = '\0';
        	bottombuff[ROWLENGTH-1] = '\0';

            HAL_Delay(1);
            LcdGoto(0, 1);
            LcdPutS(topbuff);

            LcdGoto(1, 1);
            LcdPutS(bottombuff);

            // Reset the LCD_SCROLL_TIMER to control the game speed
            sTimer[LCD_SCROLL_TIMER] = LCD_SCROLL_TIME;
        }
	}
}


void updatePlayerPos() {
	if (character.jumping) {

		// If we have reached the last frame of animation set back to ground and ready to jump again
		if (character.jumpFrame >= JUMPFRAMES) {
			character.jumping = false;
			character.jumpFrame = 0;
			character.topState = (unsigned short)' ';
			character.botState = FULL_IDX;
			character.row0Occupied = false;
			character.row1Occupied = true;
			return;
		}

		switch (character.jumpFrame) {
			case 0:
				// we are now in takeoff
				character.jumpFrame += 1;
				character.topState = TOP_HALF_IDX;
				character.botState = BOT_HALF_IDX;
				character.row0Occupied = true;
				character.row1Occupied = true;
				break;
			case 1:
				// we are now in flight
				character.jumpFrame += 1;
				character.topState = FULL_IDX;
				character.botState = (unsigned short)' ';
				character.row0Occupied = true;
				character.row1Occupied = false;
				break;
			case 2:
				// we are now in flight
				character.jumpFrame += 1;
				character.topState = BOT_HALF_IDX;
				character.botState = (unsigned short)' ';
				character.row0Occupied = true;
				character.row1Occupied = false;
				break;
			case 3:
				// we are now in flight
				character.jumpFrame += 1;
				character.topState = FULL_IDX;
				character.botState = (unsigned short)' ';
				character.row0Occupied = true;
				character.row1Occupied = false;
				break;
			case 4:
				// we are now in landing
				character.jumpFrame += 1;
				character.topState = TOP_HALF_IDX;
				character.botState = BOT_HALF_IDX;
				character.row0Occupied = true;
				character.row1Occupied = true;
				break;
			default:
				// we are now on the ground
				character.jumping = false;
				character.jumpFrame = 0;
				character.topState = (unsigned short)' ';
				character.botState = FULL_IDX;
				character.row0Occupied = false;
				character.row1Occupied = true;
				break;
		}

	}
}

char *EEPROM_READ_PASS(unsigned short addr)
{
	char temp[20];
	static char xyz[20];

	uint8_t i;

	// Read two bytes. Check separately until null is found
	for (i=0; i<10; i++)
	{
	  EEPROM_READ_FUN(addr + (2 * i));		// read two bytes

	  if (EE_Data[0] != '\0')
		temp[2*i] = EE_Data[0];
	  else
	  {
		temp[2*i] = '\0';
		break;
	  }

	  if (EE_Data[1] != '\0')
	  	temp[2*i+1] = EE_Data[1];
	  else
	  {
	    temp[2*i+1] = '\0';
	  	break;
	  }
	}

	strcpy(xyz,temp);
	return xyz;
}

void EEPROM_READ_FUN(uint8_t EE_Addr)
{
    uint8_t buf[2];

    HAL_GPIO_WritePin(GPIOB, CS, GPIO_PIN_SET);         // Chip Select Enable
    buf[0] = EEPROM_START;
    buf[1] = (EEPROM_READ | (EE_Addr & 0x3F));
    HAL_SPI_Transmit(&hspi1, buf, 2, 100);

    HAL_SPI_Receive(&hspi1, EE_Data, 3, 100);
    HAL_GPIO_WritePin(GPIOB, CS, GPIO_PIN_RESET);           // Chip Select disable
    Delay_1_plus_us();

    EE_Data[0] <<= 1;
    if (EE_Data[1] & 0x80)
    	EE_Data[0] |= 0x01;

    EE_Data[1] <<= 1;
        if (EE_Data[2] & 0x80)
        EE_Data[1] |= 0x01;
}


void EEPROM_SEND(char EE_Addr, char EE_Data1, char EE_Data2)
{
    uint8_t buf[4];

    HAL_GPIO_WritePin(GPIOB, CS, GPIO_PIN_SET);         // Chip Select Enable
    buf[0] = EEPROM_START;
    buf[1] = EEPROM_EWEN; //EEPROM_EWEN;
    HAL_SPI_Transmit(&hspi1, buf, 2, 100);
    HAL_GPIO_WritePin(GPIOB, CS, GPIO_PIN_RESET);       // Chip Select disable

    Delay_1_plus_us();

    HAL_GPIO_WritePin(GPIOB, CS, GPIO_PIN_SET);         // Chip Select Enable
    buf[0] = EEPROM_START;
    buf[1] = (0x40 | (EE_Addr & 0x03F));        // WRITE command OR with 6-bit address
    buf[2] = EE_Data1;
    buf[3] = EE_Data2;
    HAL_SPI_Transmit(&hspi1, buf, 4, 100);
    HAL_GPIO_WritePin(GPIOB, CS, GPIO_PIN_RESET);       // Chip Select disable

    HAL_Delay(20);

    HAL_GPIO_WritePin(GPIOB, CS, GPIO_PIN_SET);         // Chip Select Enable
    buf[0] = EEPROM_START;
    buf[1] = EEPROM_EWDS;
    HAL_SPI_Transmit(&hspi1, buf, 2, 100);
    HAL_GPIO_WritePin(GPIOB, CS, GPIO_PIN_RESET);           // Chip Select disable

    Delay_1_plus_us();

}


void Delay_1_plus_us()
{
    uint32_t time5now = htim5.Instance->CNT;
    while (htim5.Instance->CNT == time5now)
        ; ;
    time5now = htim5.Instance->CNT;

    while (htim5.Instance->CNT == time5now)
        ; ;
}



void updateCellsEnv() {
	// update the frames by moving each to the left
	// '0' is leftmost index
	for (int i = 0; i < ROWLENGTH-1; i++) {
		envCells[i].state = envCells[i + 1].state;
		envCells[i].environmentOccupied = envCells[i + 1].environmentOccupied;
		envCells[ROWLENGTH + i].state = envCells[ROWLENGTH + i + 1].state;
		envCells[ROWLENGTH + i].environmentOccupied = envCells[ROWLENGTH + i + 1].environmentOccupied;
	}

	// now update the last cell randomly:
	// first check if it's ok for an obstacle to appear
	if (envCells[ROWLENGTH - 2].environmentOccupied ||
		envCells[ROWLENGTH - 3].environmentOccupied ||
		envCells[ROWLENGTH - 4].environmentOccupied ||
		envCells[ROWLENGTH - 5].environmentOccupied ||
		envCells[NUMCELLS  - 2].environmentOccupied ||
		envCells[NUMCELLS  - 3].environmentOccupied ||
		envCells[NUMCELLS  - 4].environmentOccupied ||
		envCells[NUMCELLS  - 5].environmentOccupied) {
		envCells[ROWLENGTH - 1].state = ' ';
		envCells[ROWLENGTH - 1].environmentOccupied = false;
		envCells[NUMCELLS  - 1].state = ' ';
		envCells[NUMCELLS  - 1].environmentOccupied = false;
	} else {
		// random for bottom cell obstacle
		if (rand() < (0.3 * RAND_MAX)) {
			envCells[ROWLENGTH - 1].state = ' ';
			envCells[ROWLENGTH - 1].environmentOccupied = false;
			envCells[NUMCELLS  - 1].state = BOT_OBSTACLE;
			envCells[NUMCELLS  - 1].environmentOccupied = true;
		} else if (rand() < (0.1 * RAND_MAX)) {
			envCells[ROWLENGTH - 1].state = TOP_OBSTACLE;
			envCells[ROWLENGTH - 1].environmentOccupied = true;
			envCells[NUMCELLS  - 1].state = ' ';
			envCells[NUMCELLS  - 1].environmentOccupied = false;
		} else {
			envCells[ROWLENGTH - 1].state = ' ';
			envCells[ROWLENGTH - 1].environmentOccupied = false;
			envCells[NUMCELLS  - 1].state = ' ';
			envCells[NUMCELLS  - 1].environmentOccupied = false;
		}
	}
}




void Keypadscan() {
    unsigned short sIndex;
    unsigned short Temp;
    // Clear all key records
    for (sIndex = 0; sIndex < Number_of_Cols; sIndex++) {
        sKeyCurrentCol[sIndex] = 0x00;
    }
    // Read all 3 column
    for (sIndex = 0; sIndex < Number_of_Keys; sIndex++) {
        GPIOA->ODR &= ~(PA4 | PA1 | PA0);
        GPIOA->ODR |= sKeyControl[sIndex].sKeySend;
        HAL_Delay(0.5);
        switch (sKeyControl[sIndex].sKeyCommand) {
            case ONE_command:
            case TWO_command:
            case THREE_command:
                if (GPIOA->IDR & sKeyControl[sIndex].sKeyRead)
                    sKeyCurrentCol[sKeyControl[sIndex].sKeyCol] =
                        sKeyControl[sIndex].sKeyReadTempPos;
                break;
            case FOUR_command:
            case FIVE_command:
            case SIX_command:
            case SEVEN_command:
            case EIGHT_command:
            case NINE_command:
                if (GPIOC->IDR & sKeyControl[sIndex].sKeyRead)
                    sKeyCurrentCol[sKeyControl[sIndex].sKeyCol] =
                        sKeyControl[sIndex].sKeyReadTempPos;
                break;
            case STAR_command:
            case ZERO_command:
            case POUND_command:
                if (GPIOB->IDR & sKeyControl[sIndex].sKeyRead)
                    sKeyCurrentCol[sKeyControl[sIndex].sKeyCol] =
                        sKeyControl[sIndex].sKeyReadTempPos;
        }
    }
    // Check if a key is steadily read
    for (sIndex = 0; sIndex < Number_of_Cols; sIndex++) {
        if ((sKeyCurrentCol[sIndex] == sKeyDebouncedCol[sIndex]) &&
            (sKeyCurrentCol[sIndex] != 0x0000))
            break;
    }
    if (sIndex < Number_of_Cols) {
        // Check for push on/ push off (Low To High)
        for (sIndex = 0; sIndex < Number_of_Cols; sIndex++) {
            Temp = sKeyCurrentCol[sIndex] ^ sKeyPreviousCol[sIndex];
            sKeyLow2HighCol[sIndex] = (sKeyCurrentCol[sIndex] & Temp);
        }
        // Update Previous records
        for (sIndex = 0; sIndex < Number_of_Cols; sIndex++) {
            sKeyPreviousCol[sIndex] = sKeyCurrentCol[sIndex];
        }
        // Find which key is JUST depressed (Low To High) or KeyRepeat detected
        for (sIndex = 0; sIndex < Number_of_Keys; sIndex++) {
            if (sKeyLow2HighCol[sKeyControl[sIndex].sKeyCol] &
                sKeyControl[sIndex].sKeyReadTempPos) {
                sKeyIssued = sKeyControl[sIndex].sKeyCommand;
                sKeyStatus |= (KeyDetect | KeyLow2High);
                sTimer[KEY_WAIT_REPEAT_TIMER] = KEY_WAIT_REPEAT_TIME;
                sKeyStatus |= KeyRepeat;   // a new key comes in, set the repeat flag
                sIndexCopy = sIndex;  // save a copy of sIndex for push & held use
				break;
            } else if ((sKeyStatus & KeyRepeat) &&
                       (sTimer[KEY_WAIT_REPEAT_TIMER] == 0)) {
                if (sTimer[KEY_REPEAT_TIMER] == 0) {
                    sKeyIssued = sKeyControl[sIndexCopy].sKeyCommand;
                    sKeyStatus |= (KeyDetect | KeyToBeRepeated);
                    sTimer[KEY_REPEAT_TIMER] = KEY_REPEAT_TIME;
                }
            } else
                sKeyIssued = 0xFFFF;
        }
    } else {
        sKeyStatus &= ~(KeyDetect | KeyLow2High | KeyToBeRepeated | KeyRepeat);
        sTimer[KEY_REPEAT_TIMER] = 0;  // Reset repeat timer if no key
        for (sIndex = 0; sIndex < Number_of_Cols; sIndex++)
            sKeyPreviousCol[sIndex] = 0;
    }
    // Transfer Current reading to debounced reading
    for (sIndex = 0; sIndex < Number_of_Cols; sIndex++) {
        sKeyDebouncedCol[sIndex] = sKeyCurrentCol[sIndex];
        sKeyLow2HighCol[sIndex] = 0;
    }
}
void KeyProcess() {
    uint16_t sIndex;
    if ((sKeyStatus & KeyDetect) && (sKeyIssued != 0xFFFF)) {
        switch (sKeyIssued) {
            case ONE_command: {
                if (sKeyStatus & KeyLow2High)
                    key1();
                else if (sKeyStatus & KeyToBeRepeated)
                    key1R();
                break;
            }
            case FOUR_command: {
                if (sKeyStatus & KeyLow2High)
                    key4();
                else if (sKeyStatus & KeyToBeRepeated)
                    key4R();
                break;
            }
            case SEVEN_command: {
                if (sKeyStatus & KeyLow2High)
                    key7();
                else if (sKeyStatus & KeyToBeRepeated)
                    key7R();
                break;
            }
            case STAR_command: {
                if (sKeyStatus & KeyLow2High)
                    keyS();
                else if (sKeyStatus & KeyToBeRepeated)
                    keySR();
                break;
            }
            case TWO_command: {
                if (sKeyStatus & KeyLow2High)
                    key2();
                else if (sKeyStatus & KeyToBeRepeated)
                    key2R();
                break;
            }
            case FIVE_command: {
                if (sKeyStatus & KeyLow2High)
                    key5();
                else if (sKeyStatus & KeyToBeRepeated)
                    key5R();
                break;
            }
            case EIGHT_command: {
                if (sKeyStatus & KeyLow2High)
                    key8();
                else if (sKeyStatus & KeyToBeRepeated)
                    key8R();
                break;
            }
            case ZERO_command: {
                if (sKeyStatus & KeyLow2High)
                    key0();
                else if (sKeyStatus & KeyToBeRepeated)
                    key0R();
                break;
            }
            case THREE_command: {
                if (sKeyStatus & KeyLow2High)
                    key3();
                else if (sKeyStatus & KeyToBeRepeated)
                    key3R();
                break;
            }
            case SIX_command: {
                if (sKeyStatus & KeyLow2High)
                    key6();
                else if (sKeyStatus & KeyToBeRepeated)
                    key6R();
                break;
            }
            case NINE_command: {
                if (sKeyStatus & KeyLow2High)
                    key9();
                else if (sKeyStatus & KeyToBeRepeated)
                    key9R();
                break;
            }
            case POUND_command: {
                if (sKeyStatus & KeyLow2High)
                    keyP();
                else if (sKeyStatus & KeyToBeRepeated)
                    keyPR();
                break;
            }
            default:
                break;
        }
        sKeyStatus &= ~(KeyDetect | KeyLow2High | KeyToBeRepeated);
        // Clear all Low-2-High and High-2-Low records
        for (sIndex = 0; sIndex < Number_of_Cols; sIndex++)
            sKeyLow2HighCol[sIndex] = 0x0000;
    }
}



void key1() {

}
void key2() {

}
void key3() {

}
void key4() {

}
void key5() {

}
void key6() {


}
void key7() {

}
void key8() {

}
void key9() {

}
void key0() {
	// if character is not jumping set jumping to true and frame to 0
	if (!character.jumping) {
		character.jumping = true;
		character.jumpFrame = 0;
	}

}
void keyS() {

}
void keyP() {
	resetHighScore();
}




void key1R() {
}
void key2R() {
}
void key3R() {
}
void key4R() {
}
void key5R() {
}
void key6R() {
}
void key7R() {

}
void key8R() {

}
void key9R() {

}
void key0R() {
	// if character is not jumping set jumping to true and frame to 0
	if (!character.jumping) {
		character.jumping = true;
		character.jumpFrame = 0;
	}
}
void keySR() {

}
void keyPR() {

}

void resetHighScore(void) {
	EEPROM_SEND(0xC, '0', '\0');
	EEPROM_SEND(0xE, '0', '\0');
	EEPROM_SEND(0x10, '0', '\0');
	EEPROM_SEND(0x12, '0', '\0');
}

void I2CSend(char port, char data)
{
	uint8_t Buf[2];
	HAL_StatusTypeDef status;

	Buf[0] = port;
	Buf[1] = data;

	// Try to transmit - add error handling
	status = HAL_I2C_Master_Transmit(&hi2c1, I2C_ADDRESS, Buf, 2, 1000);

	// If there's an error, try re-initializing I2C and sending again
	if (status != HAL_OK) {
		HAL_I2C_DeInit(&hi2c1);
		HAL_Delay(10);
		HAL_I2C_Init(&hi2c1);
		HAL_Delay(10);
		HAL_I2C_Master_Transmit(&hi2c1, I2C_ADDRESS, Buf, 2, 1000);
	}
}

// Function to update life LEDs
void updateLifeLEDs(int lives) {
    uint8_t led_status = 0;

    // Set the appropriate bits based on number of lives
    if (lives >= 1) led_status |= LIFE1;
    if (lives >= 2) led_status |= LIFE2;
    if (lives >= 3) led_status |= LIFE3;

    // Send to the MCP23008
    I2CSend(MCP_GPIOB, led_status);
}


uint32_t get_adc_seed() {
    HAL_ADC_Start(&hadc1); // Start conversion
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY); // Wait for conversion
    uint32_t val = HAL_ADC_GetValue(&hadc1); // Read value
    HAL_ADC_Stop(&hadc1);
    return val;
}


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_7;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10D19CE4;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 3999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_DOWN;
  htim2.Init.Period = 19;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV2;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM5_Init(void)
{

  /* USER CODE BEGIN TIM5_Init 0 */

  /* USER CODE END TIM5_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM5_Init 1 */

  /* USER CODE END TIM5_Init 1 */
  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 79;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 4294967295;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV2;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM5_Init 2 */

  /* USER CODE END TIM5_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_8
                          |GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC0 PC1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA4 PA8
                           PA9 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_8
                          |GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB10 PB3 PB4 PB5
                           PB6 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
