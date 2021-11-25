// Sample code for ECE 198

// Written by Bernie Roehl, August 2021

// This file contains code for a number of different examples.
// Each one is surrounded by an #ifdef ... #endif block inside of main().

// To run a particular example, you should remove the comment (//) in
// front of exactly ONE of the following lines:

// #define BUTTON_BLINK
// #define LIGHT_SCHEDULER
// #define TIME_RAND
// #define KEYPAD
// #define KEYPAD_CONTROL
// #define SEVEN_SEGMENT
// #define KEYPAD_SEVEN_SEGMENT
// #define COLOR_LED
// #define ROTARY_ENCODER
// #define ANALOG
// #define PWM

#include <stdbool.h> // booleans, i.e. true and false
#include <stdio.h>   // sprintf() function
#include <stdlib.h>  // srand() and random() functions

#include "ece198.h"

int main(void)
{
    HAL_Init(); // initialize the Hardware Abstraction Layer

    // Peripherals (including GPIOs) are disabled by default to save power, so we
    // use the Reset and Clock Control registers to enable the GPIO peripherals that we're using.

    __HAL_RCC_GPIOA_CLK_ENABLE(); // enable port A (for the on-board LED, for example)
    __HAL_RCC_GPIOB_CLK_ENABLE(); // enable port B (for the rotary encoder inputs, for example)
    __HAL_RCC_GPIOC_CLK_ENABLE(); // enable port C (for the on-board blue pushbutton, for example)
    InitializePin(GPIOA, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, 0);  // initialize color LED 1
    InitializePin(GPIOB, GPIO_PIN_6 | GPIO_PIN_10 | GPIO_PIN_4, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, 0);  // initialize color LED 2
    InitializePin(GPIOA, GPIO_PIN_10, GPIO_MODE_INPUT, GPIO_PULLUP, 0); // button 1
    InitializePin(GPIOB, GPIO_PIN_3, GPIO_MODE_INPUT, GPIO_PULLUP, 0); // button 2
    InitializePin(GPIOB, GPIO_PIN_5, GPIO_MODE_INPUT, GPIO_PULLUP, 0); // button 3

    // set up for serial communication to the host computer
    // (anything we write to the serial port will appear in the terminal (i.e. serial monitor) in VSCode)
    SerialSetup(9600);

    // GAME CODE

    // define note frequencies
    // 2x frequency of note (AKA period for PWM)
    int noteA = 880;
    int noteC = 1046;
    int noteE = 660;

    // set maximum level that must be attained to win the game
    uint16_t maxLevel = 7;
    // // initialize a list of the notes in the given pattern to match
    // int notesList[maxLevel];

    // // Hardcoded pattern
    // notesList[0] = noteA;
    // notesList[1] = noteC;
    // notesList[2] = noteA;
    // notesList[3] = noteE;
    // notesList[4] = noteA;
    // notesList[5] = noteC;
    // notesList[6] = noteE;
    
    // Randomized Pattern Generation (Must have user press button 7 times)
    int notesList[maxLevel];
    int random;
    int counter = 0;
    while (counter < maxLevel) {
        while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13));
        srand(HAL_GetTick());
        random = rand();
        while(!HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13));
        
        random = random % 3;
        if (random == 0) {
            SerialPutc('0');
            notesList[counter] = noteA;
        }
        else if (random == 1) {
            SerialPutc('1');
            notesList[counter] = noteC;
        }
        else if (random == 2) {
            SerialPutc('2');
            notesList[counter] = noteE;
        }
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





#ifdef BUTTON_BLINK
    // Wait for the user to push the blue button, then blink the LED.

    // wait for button press (active low)
    while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13))
    {
    }

    while (1) // loop forever, blinking the LED
    {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        HAL_Delay(250);  // 250 milliseconds == 1/4 second
    }
#endif

#ifdef LIGHT_SCHEDULER
    // Turn on the LED five seconds after reset, and turn it off again five seconds later.

    while (true) {
        uint32_t now = HAL_GetTick();
        if (now > 5000 && now < 10000)
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, true);   // turn on LED
        else
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, false);  // turn off LED
    }
#endif

#ifdef TIME_RAND
    // This illustrates the use of HAL_GetTick() to get the current time,
    // plus the use of random() for random number generation.
    
    // Note that you must have "#include <stdlib.h>"" at the top of your main.c
    // in order to use the srand() and random() functions.

    // while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13));  // wait for button press
    // srand(HAL_GetTick());  // set the random seed to be the time in ms that it took to press the button
    // if the line above is commented out, your program will get the same sequence of random numbers
    // every time you run it (which may be useful in some cases)

    while (true) // loop forever
    {
        while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13));  // wait for button press

        // Display the time in milliseconds along with a random number.
        // We use the sprintf() function to put the formatted output into a buffer;
        // see https://www.tutorialspoint.com/c_standard_library/c_function_sprintf.htm for more
        // information about this function
        char buff[100];
        sprintf(buff, "Time: %lu ms   Random = %ld\r\n", HAL_GetTick(), random());
        // lu == "long unsigned", ld = "long decimal", where "long" is 32 bit and "decimal" implies signed
        SerialPuts(buff); // transmit the buffer to the host computer's serial monitor in VSCode/PlatformIO

        while (!HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13));  // wait for button to be released
    }
#endif

#ifdef KEYPAD
    // Read buttons on the keypad and display them on the console.

    // this string contains the symbols on the external keypad
    // (they may be different for different keypads)
    char *keypad_symbols = "123A456B789C*0#D";
    // note that they're numbered from left to right and top to bottom, like reading words on a page

    InitializeKeypad();
    while (true)
    {
        while (ReadKeypad() < 0);   // wait for a valid key
        SerialPutc(keypad_symbols[ReadKeypad()]);  // look up its ASCII symbol and send it to the hsot
        while (ReadKeypad() >= 0);  // wait until key is released
    }
#endif

#ifdef KEYPAD_CONTROL
    // Use top-right button on 4x4 keypad (typically 'A') to toggle LED.

    InitializeKeypad();
    while (true)
    {
        while (ReadKeypad() < 0);   // wait for a valid key
        int key = ReadKeypad();
        if (key == 3)  // top-right key in a 4x4 keypad, usually 'A'
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);   // toggle LED on or off
         while (ReadKeypad() >= 0);  // wait until key is released
    }
#endif

#ifdef SEVEN_SEGMENT
    // Display the numbers 0 to 9 inclusive on the 7-segment display, pausing for a second between each one.
    // (remember that the GND connection on the display must go through a 220 ohm current-limiting resistor!)
    
    Initialize7Segment();
    while (true)
        for (int i = 0; i < 10; ++i)
        {
            Display7Segment(i);
            HAL_Delay(1000);  // 1000 milliseconds == 1 second
        }
#endif

#ifdef KEYPAD_SEVEN_SEGMENT
    // Combines the previous two examples, displaying numbers from the keypad on the 7-segment display.

    // this string contains the symbols on the external keypad
    // (they may be different for different keypads)
    char *keypad_symbols = "123A456B789C*0#D";
    // note that they're numbered from left to right and top to bottom, like reading words on a page

    InitializeKeypad();
    Initialize7Segment();
    while (true)
    {
        int key = ReadKeypad();
        if (key >= 0)
            Display7Segment(keypad_symbols[key]-'0');  // tricky code to convert ASCII digit to a number
    }
#endif

#ifdef COLOR_LED
    // Cycle through all 8 possible colors (including off and white) as the on-board button is pressed.
    // This example assumes that the color LED is connected to pins D11, D12 and D13.

    // Remember that each of those three pins must go through a 220 ohm current-limiting resistor!
    // Also remember that the longest pin on the LED should be hooked up to GND.

    InitializePin(GPIOA, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, 0);  // initialize color LED output pins
    while (true) {
        for (int color = 0; color < 8; ++color) {
            // bottom three bits indicate which of the three LEDs should be on (eight possible combinations)
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, color & 0x01);  // blue  (hex 1 == 0001 binary)
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, color & 0x02);  // green (hex 2 == 0010 binary)
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, color & 0x04);  // red   (hex 4 == 0100 binary)

            while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13));   // wait for button press 
            while (!HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13));  // wait for button release
        }
    }
#endif

#ifdef ROTARY_ENCODER
    // Read values from the rotary encoder and update a count, which is displayed in the console.

    InitializePin(GPIOB, GPIO_PIN_5, GPIO_MODE_INPUT, GPIO_PULLUP, 0);   // initialize CLK pin
    InitializePin(GPIOB, GPIO_PIN_4, GPIO_MODE_INPUT, GPIO_PULLUP, 0);   // initialize DT pin
    InitializePin(GPIOB, GPIO_PIN_10, GPIO_MODE_INPUT, GPIO_PULLUP, 0);  // initialize SW pin
    
    bool previousClk = false;  // needed by ReadEncoder() to store the previous state of the CLK pin
    int count = 0;             // this gets incremented or decremented as we rotate the encoder

    while (true)
    {
        int delta = ReadEncoder(GPIOB, GPIO_PIN_5, GPIOB, GPIO_PIN_4, &previousClk);  // update the count by -1, 0 or +1
        if (delta != 0) {
            count += delta;
            char buff[100];
            sprintf(buff, "%d  \r", count);
            SerialPuts(buff);
        }
        bool sw = !HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10);  // read the push-switch on the encoder (active low, so we invert it using !)
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, sw);  // turn on LED when encoder switch is pushed in
    }
#endif

#ifdef ANALOG
    // Use the ADC (Analog to Digital Converter) to read voltage values from two pins.

    __HAL_RCC_ADC1_CLK_ENABLE();        // enable ADC 1
    ADC_HandleTypeDef adcInstance;      // this variable stores an instance of the ADC
    InitializeADC(&adcInstance, ADC1);  // initialize the ADC instance
    // Enables the input pins
    // (on this board, pin A0 is connected to channel 0 of ADC1, and A1 is connected to channel 1 of ADC1)
    InitializePin(GPIOA, GPIO_PIN_0 | GPIO_PIN_1, GPIO_MODE_ANALOG, GPIO_NOPULL, 0);   
    while (true)
    {
        // read the ADC values (0 -> 0V, 2^12 -> 3.3V)
        uint16_t raw0 = ReadADC(&adcInstance, ADC_CHANNEL_0);
        uint16_t raw1 = ReadADC(&adcInstance, ADC_CHANNEL_1);

        // print the ADC values
        char buff[100];
        sprintf(buff, "Channel0: %hu, Channel1: %hu\r\n", raw0, raw1);  // hu == "unsigned short" (16 bit)
        SerialPuts(buff);
    }
#endif

#ifdef PWM
    // Use Pulse Width Modulation
    // uint16_t periodA4 = 880, prescaleA4 = 16;
    // uint16_t periodC5 = 1046, prescaleC5 = 16;
    uint16_t periodE5 = 1318, prescaleE5 = 16;

    __TIM1_CLK_ENABLE();    // enable timer 2
    //instance1 is Pin A0: Timer 2, channel 1.
    TIM_HandleTypeDef pwmTimerInstance1;    // this variable stores an instance of the timer
    InitializePWMTimer(&pwmTimerInstance1, TIM1, periodE5, prescaleE5);   // initialize the timer instance
    InitializePWMChannel(&pwmTimerInstance1, TIM_CHANNEL_2);            // initialize one channel (can use others for motors, etc)

    InitializePin(GPIOA, GPIO_PIN_9, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_AF1_TIM1); // connect the buzzer to the timer output

    SetPWMDutyCycle(&pwmTimerInstance1, TIM_CHANNEL_2, periodE5/2);
    HAL_Delay(2000);
    SetPWMDutyCycle(&pwmTimerInstance1, TIM_CHANNEL_2, 0);

#endif
    return 0;
}

// This function is called by the HAL once every millisecond
void SysTick_Handler(void)
{
    HAL_IncTick(); // tell HAL that a new tick has happened
    // we can do other things in here too if we need to, but be careful
}
