// ECE198 Project: Musical "Simon Says"
// N. Chung and Komal Vachhani
// November 25, 2021

#include <stdbool.h> // booleans, i.e. true and false
#include <stdio.h>   // sprintf() function
#include <stdlib.h>  // srand() and random() functions

#include "ece198.h"

int main(void)
{
    HAL_Init(); // initialize the Hardware Abstraction Layer

    __HAL_RCC_GPIOA_CLK_ENABLE(); // enable port A (for the on-board LED, for example)
    __HAL_RCC_GPIOB_CLK_ENABLE(); // enable port B (for the rotary encoder inputs, for example)
    __HAL_RCC_GPIOC_CLK_ENABLE(); // enable port C (for the on-board blue pushbutton, for example)
    InitializePin(GPIOA, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, 0);  // initialize color LED 1
    InitializePin(GPIOB, GPIO_PIN_6 | GPIO_PIN_10 | GPIO_PIN_4, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, 0);  // initialize color LED 2
    InitializePin(GPIOA, GPIO_PIN_10, GPIO_MODE_INPUT, GPIO_PULLUP, 0); // button 1
    InitializePin(GPIOB, GPIO_PIN_3, GPIO_MODE_INPUT, GPIO_PULLUP, 0); // button 2
    InitializePin(GPIOB, GPIO_PIN_5, GPIO_MODE_INPUT, GPIO_PULLUP, 0); // button 3

    SerialSetup(9600); // set up serial

    // GAME CODE

    // DEFINE note frequencies
    // 2x frequency of note (AKA period for PWM)
    int noteA = 880;
    int noteC = 1046;
    int noteE = 660;

    // set maximum level that must be attained to win the game
    uint16_t maxLevel = 7;
    
    // Randomized Pattern Generation (Must have user press button 7 times)
    // Create array of maxLevel # of indices
    int notesList[maxLevel];
    // Initialize random number variable
    int random;
    // Initialize counter to loop through
    int counter = 0;

    // While the counter has not reached the maxLevel # of indices
    while (counter < maxLevel) {

        // Wait for button press
        while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13));
        // HAL_GetTick()
        srand(HAL_GetTick());
        // Assign random variable the number generated from srand()
        random = rand();
        // Wait for button release
        while(!HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13));
        
        // Make random either 0, 1, or 2
        random = random % 3;
        // If the value is 0, print 0 and add noteA to the current index of the array
        if (random == 0) {
            SerialPutc('0');
            notesList[counter] = noteA;
        }

        // If the value is 1, print 1 and add noteC to the current index of the array
        else if (random == 1) {
            SerialPutc('1');
            notesList[counter] = noteC;
        }

        // If the value is 1, print 1 and add noteC to the current index of the array
        else if (random == 2) {
            SerialPutc('2');
            notesList[counter] = noteE;
        }

        // Increment counter to continue iterating through the array, assigning each index of the array a value
        ++counter;
    }

    // start the user at level 1
    uint16_t level = 1;
    // while the user has not won the game (beat the last level)
    while (level <= maxLevel) {

        // Start Pattern Output

        // for each index up to the current level
        for (uint16_t k = 0; k < level; k++) {

            // if the note in the pattern is an A
            if (notesList[k] == noteA) {

                // second LED goes RED
                SetLight2(0x04);

                // buzzer plays an A
                PlaySound(noteA);
                HAL_Delay(100);
            }

            // if the note in the pattern is a C
            else if (notesList[k] == noteC) {
                
                // second LED goes green
                SetLight2(0x02);

                // buzzer plays a C
                PlaySound(noteC);
                HAL_Delay(100);
            }

            // if the note in the pattern is an E
            else if (notesList[k] == noteE) {

                // second LED goes blue
                SetLight2(0x01);

                // buzzer plays an E
                PlaySound(noteE);
                HAL_Delay(100);
            }
        }
        // turn off LED
        SetLight2(0x00);

        // new array to hold user input
        int UserInput[level];                   // number of indices == current level number
        bool levelPassed = true;                // bool to track if user passed the level

        // Start of User Input
        // first LED holds white while waiting for user to input
        SetLight1(0x07);

        // for each input (# of user inputs = current level #)
        for (uint16_t m = 0; m < level; m++) {

            //  wait for User Input
            while(true) {

                // if user presses button 1
                if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10) == 0) {

                    // second LED goes RED
                    SetLight2(0x04);

                    // buzzer plays an A
                    PlaySound(noteA);

                    // user input array tracks what the input was
                    UserInput[m] = noteA;

                    // 
                    break;
                }

                // if user presses button 2
                else if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3) == 0) {
                    // second LED goes green
                    SetLight2(0x02);

                    // buzzer plays a C
                    PlaySound(noteC);

                    // user input array tracks what the input was
                    UserInput[m] = noteC;
                    break;
                }

                // if user presses button 3
                else if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == 0) {
                    // second LED goes blue
                    SetLight2(0x01);

                    // buzzer plays an E
                    PlaySound(noteE);

                    // user input array tracks what the input was
                    UserInput[m] = noteE;
                    break;
                }
            }

            // if the user input value does not match the intended value from the pattern
            if (UserInput[m] != notesList[m]) {

                // user has not passed the level
                levelPassed = false;
            }
        }
        // When user input time is over, turn the first LED off
        SetLight1(0x00);

        // if the user has passed the level (none of the inputs were incorrect)
        if (levelPassed == true) {
            // execute level won function
            LevelWon();

            // increment level by 1
            ++level;
        }

        // if the user has failed the level
        else {
            // execute level lost function
            LevelLost();
        }
    }

    // once level has incremented past max level, game is won
    if (level == (maxLevel + 1)) {
        // execute game won function
        GameWon(6000);
    }

    return 0;
}



// This function is called by the HAL once every millisecond
void SysTick_Handler(void)
{
    HAL_IncTick(); // tell HAL that a new tick has happened
    // we can do other things in here too if we need to, but be careful
}



// Turn on first RGB LED
void SetLight1 (int color) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, color & 0x01);  // blue  (hex 1 == 0001 binary)
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, color & 0x02);  // green (hex 2 == 0010 binary)
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, color & 0x04);  // red   (hex 4 == 0100 binary)
}

// Turn on second RGB LED
void SetLight2 (int color) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, color & 0x01);  // blue  (hex 1 == 0001 binary)
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, color & 0x02);  // green (hex 2 == 0010 binary)
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, color & 0x04);  // red   (hex 4 == 0100 binary)
}

// Play a sound from the buzzer for 1 second
void PlaySound (uint16_t period) {      // takes 2x the frequency of the note
    __TIM1_CLK_ENABLE();    // enable timer 2
    TIM_HandleTypeDef pwmTimerInstance1;    // this variable stores an instance of the timer
    InitializePWMTimer(&pwmTimerInstance1, TIM1, period, 16);   // initialize the timer instance
    InitializePWMChannel(&pwmTimerInstance1, TIM_CHANNEL_2);            // initialize one channel (can use others for motors, etc)

    InitializePin(GPIOA, GPIO_PIN_9, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF1_TIM1); // connect the buzzer to the timer output

    SetPWMDutyCycle(&pwmTimerInstance1, TIM_CHANNEL_2, period/2);
    HAL_Delay(1000);
    SetPWMDutyCycle(&pwmTimerInstance1, TIM_CHANNEL_2, 0);
}

// Play a sound from the buzzer for a specific duration
void PlayNote (int duration, uint16_t period) {           // takes time, and 2x the frequency of the note
    __TIM1_CLK_ENABLE();    // enable timer 2
    TIM_HandleTypeDef pwmTimerInstance1;    // this variable stores an instance of the timer
    InitializePWMTimer(&pwmTimerInstance1, TIM1, period, 16);   // initialize the timer instance
    InitializePWMChannel(&pwmTimerInstance1, TIM_CHANNEL_2);            // initialize one channel (can use others for motors, etc)

    InitializePin(GPIOA, GPIO_PIN_9, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF1_TIM1); // connect the buzzer to the timer output

    SetPWMDutyCycle(&pwmTimerInstance1, TIM_CHANNEL_2, period/2);
    HAL_Delay(duration);
    SetPWMDutyCycle(&pwmTimerInstance1, TIM_CHANNEL_2, 0);
}

// Flash green lights 3x if level has been won
void LevelWon(){
    for (uint16_t m = 0; m < 3; m++) {
        SetLight1(0x02);
        SetLight2(0x02);
        PlayNote(200, 784);
        SetLight1(0x00);
        SetLight2(0x00);
        HAL_Delay(100);
    }
    // Wait five seconds before resuming to next level
    HAL_Delay(500);
}

// Flash red lights 3x if level has been lost
void LevelLost(){
    for (uint16_t m = 0; m < 3; m++) {
        SetLight1(0x04);
        SetLight2(0x04);
        PlayNote(200, 1564);
        SetLight1(0x00);
        SetLight2(0x00);
        HAL_Delay(100);
    }
    HAL_Delay(500);
}

// Flash white lights 3x if game has been won
// Play a nice tune and flash red and green lights
void GameWon(int duration) {
    for (uint16_t m = 0; m < 3; m++) {
        SetLight1(0x07);
        SetLight2(0x07);
        HAL_Delay(300);
        SetLight1(0x00);
        SetLight2(0x00);
        HAL_Delay(300);
    }
    HAL_Delay(500);
    int repeat = duration / 1000;
    for (uint16_t k = 0; k < repeat; k++) {
        SetLight2(0x02);
        HAL_Delay(125);
        SetLight1(0x04);
        HAL_Delay(125);
        SetLight2(0x04);
        HAL_Delay(125);
        SetLight1(0x02);
        HAL_Delay(125);
        SetLight2(0x02);
        HAL_Delay(125);
        SetLight1(0x04);
        HAL_Delay(125);
        SetLight2(0x04);
        HAL_Delay(125);
        SetLight1(0x02);
        HAL_Delay(125);
    }
    SetLight1(0x02);
    SetLight2(0x02);
}