/* Program written by:                  Gerardo Rodriguez
 * Functions initHw(), putcUart0(), getcUart0(), and putsUart0() taken from: Dr. Jason Losh's Embedded Systems I code examples
 * Program written to meet the requirements of the Embedded Systems I course at the University of Texas at Arlington
 *
 * */

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"

#define MAX_CHARS 80

char strInput[MAX_CHARS];
char *temp_command;
char *temp_arg[10];
uint8_t argCount;

/* adding a putsUart0 function declaration so string functions know about it */
void putsUart0(const char* str);
/* String handling functions */

int strcmp(const char* str1, const char* str2)
{
    int result = 0;
    uint8_t i = 0;
    while (str1[i] != '\0' && str2[i] != '\0')
    {
        if ( str1[i] < str2[i] )
        {
            result = -1; break;
        }
        else if ( str1[i] > str2[i] )
        {
            result = 1;  break;
        }
        else
            i++;
    }
    return result;
}

uint8_t strlen(const char* str)
{
    uint8_t result = 0;
    while (str[result++] != '\0');
    return result;
}

int atoi(const char* str)
{
    uint8_t i = 0;
    int result = 0;
    uint8_t length_of_string = strlen(str);
    uint32_t power_of_ten = 1;
    while (i < length_of_string - 1)
    {
        power_of_ten *= 10;
        i++;
    }
    while (i < length_of_string)
    {
        if (str[i] > '9' || str[i] < '0')
        {
            putsUart0(str); putsUart0(" contains a character. Could not convert to integer.\n");
            break;
        }
        result += (str[i] - 48) * 10;

        power_of_ten /= 10;
        i++;
    }
    return result;
}
/* String handling functions */

uint8_t is_alphanumeric(char c)
{
    /*determines whether input is alphanumeric or not based on ASCII values*/
    uint8_t result[256] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
                            0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0,
                            };
    return result[c];
}
void tokenize_string(char* str)
{
    uint8_t i;
    uint8_t j = 0;
    uint8_t length = strlen(str);
    if (is_alphanumeric(str[0]))
    {
        temp_command = temp_arg[0] = &str[0];
        j++;
    }
    for (i = 0; i < length; i++)
    {
        if ( !(is_alphanumeric(str[i]) ) )
                str[i] = '\0';
        if ( is_alphanumeric(str[i]) && !(is_alphanumeric(str[i - 1])) && i > 0)
                temp_arg[j++] = &str[i];
    }
    if (j == 0) temp_command = temp_arg[0] = &str[0];
    temp_command = temp_arg[0];
    argCount = j - 1;
}
bool isCommand(char* cmd, uint8_t min)
{
  /*commands: calibrate, color, erase, periodic, delta, match, trigger, button
              led, test, ramp, rgb, light*/
  if (strcmp(temp_command, cmd) == 0 && min == 0)
      return true;

  else if (strcmp(temp_command,cmd) == 0 && min == 1)
    return true;

  else if (strcmp(temp_command,cmd) == 0 && min == 3)
    return true;

  else
    return false;
}
uint32_t getValue(uint16_t argNum)
{
    return atoi(temp_arg[argNum + 1]);
}
char* getString(uint8_t argNum)
{
    return temp_arg[argNum + 1];
}
void initHw()
{
    // Configure HW to work with 16 MHz XTAL, PLL enabled, system clock of 40 MHz
    // PWM is system clock / 2
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);

    // Set GPIO ports to use APB (not needed since default configuration -- for clarity)
    SYSCTL_GPIOHBCTL_R = 0;

    // Enable GPIO port B and E peripherals
    SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOA;

    // Configure UART0 pins
    SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R0;         // turn-on UART0, leave other uarts in same status
    GPIO_PORTA_DEN_R |= 3;                           // enable digital on UART0 pins: default, added for clarity
    GPIO_PORTA_AFSEL_R |= 3;                         // use peripheral to drive PA0, PA1: default, added for clarity
    GPIO_PORTA_PCTL_R = GPIO_PCTL_PA1_U0TX | GPIO_PCTL_PA0_U0RX;
                                                      //select UART0 to drive pins PA0 and PA1: default, added for clarity

     //Configure UART0 to 115200 baud, 8N1 format (must be 3 clocks from clock enable and config writes)
    UART0_CTL_R = 0;                                 // turn-off UART0 to allow safe programming
    UART0_CC_R = UART_CC_CS_SYSCLK;                  // use system clock (40 MHz)
    UART0_IBRD_R = 21;                               // r = 40 MHz / (Nx115.2kHz), set floor(r)=21, where N=16
    UART0_FBRD_R = 45;                               // round(fract(r)*64)=45
    UART0_LCRH_R = UART_LCRH_WLEN_8 | UART_LCRH_FEN; // configure for 8N1 w/ 16-level FIFO
    UART0_CTL_R = UART_CTL_TXE | UART_CTL_RXE | UART_CTL_UARTEN; // enable TX, RX, and module
}

void putcUart0(char c)
{
  while (UART0_FR_R & UART_FR_TXFF);               // wait if uart0 tx fifo full
  UART0_DR_R = c;                                  // write character to fifo
}
void putsUart0(const char* str)
{
    uint8_t i;
    for (i = 0; i < strlen(str); i++)
      putcUart0(str[i]);
}
char getcUart0()
{
    while (UART0_FR_R & UART_FR_RXFE);               // wait if uart0 rx fifo empty
    return UART0_DR_R & 0xFF;                        // get character from fifo
}
void getsUart0(char* str, uint8_t maxChars)
{
    uint8_t count = 0;
    char c;
    while (count < maxChars)
    {
        c = getcUart0();
        //if you've reached the max amount of characters, break (80 chars max)
        if (count == maxChars)
        {
            str[count] = '\0';
            break;
        }
        if (c == 8)
        {
            if (count < 1) continue;
            count--; continue;
        }
        //if you've pressed enter, add a newline delimiter, a carriage return, and a null terminator
        if (c == 13)
        {
            str[count] = '\0';
            break;
        }
        //if an input is an uppercase letter, make it lowercase
        if (c >= 'A' && c <= 'Z')
        {
            c+=32;
            str[count] = c;
        }
        //put whatever is in the buffer onto the screen
        else
            str[count] = c;
        count++;
    }
}

int main(void)
{
    // Initialize hardware
    initHw();
    /* Continuously get and print user input
     * */
    char *menu = "Menu: \n\n"
                "calibrate:\t Calibrate according to white balance\n"
                "color N:\t store current color\n";

    char *menu1 = "erase N:\t erase reference color N\n"
                  "periodic T:\t send RGB triplet every 0.1 x T seconds\n"
                  "delta D:\t send RGB triplet when RMS average vs long-term average is greater than D\n";
    char *menu2 = "match E:\t send RGB triplet when error between sample and color reference is less than E\n"
                  "trigger:\t send RGB triplet immediately\n"
                  "button: \t send RGB triplet when PB is pressed\n";
    char *menu3 = "led off:\t disable green status LED\n"
                  "led on: \t enable green status LED\n"
                  "led sample:\t blink green status LED when sample is taken\n";
    char *menu4 = "test:  \t\t ramp up RGB separately, respectively, and output 12-bit light intensity\n"
                  "ramp:  \t\t ramps up RGB separately, respectively\n"
                  "rgb A B C:\t sends an RGB value; A B and C vary from 0 to 255\n\n";
    putsUart0("\nHello! Welcome to ColorimeterUTA! Please enter any input or 'menu' if you need help.\n");
    while (1)
    {
        getsUart0(strInput, MAX_CHARS);
        tokenize_string(strInput);

        /*tokenizing string, setting argCount, getting arguments, and determining command*/
        putsUart0("command is: ");
        putsUart0(temp_command);
        putsUart0("\n");
        if (isCommand("menu", argCount))
        {
            putsUart0(menu); putsUart0(menu1); putsUart0(menu2); putsUart0(menu3); putsUart0(menu4);
        }
        else if (isCommand("rgb", argCount))
        {
            putsUart0("Setting PWM signal\n");

        }
        else if (isCommand("calibrate", argCount))
            putsUart0("Calibrating...\n");
        else if (isCommand("color", argCount))
            putsUart0("Storing color...\n");
        else if (isCommand("erase", argCount))
            putsUart0("Erasing reference color...\n");
        else if (isCommand("periodic", argCount))
        {
            putsUart0("Sending RGB triplets in 8-bit calibrated format every ");
            putsUart0(temp_arg[0]); putsUart0(" * 0.1 seconds...\n");
        }
        else if (isCommand("delta", argCount))
        {
            putsUart0("sending RGB triplet when RMS > IIR based on ");
            putsUart0(temp_arg[0]); putsUart0(" or distance from value...\n");
        }
        else if (isCommand("match", argCount))
        {
            putsUart0("Configuring hardware to Euclidean distance: ");
            putsUart0(temp_arg[0]); putsUart0("...\n");
        }
        else if (isCommand("trigger", argCount) /* && system is calibrated*/)
        {
            putsUart0("Sending rgb triplet...\n");
        }
        else if (isCommand("button", argCount))
        {
            putsUart0("Push button when you desire to send rgb triplet...\n");
        }
        else if (isCommand("led", argCount))
        {
            if (strcmp(temp_arg[1], "off") == 0)
            {
                putsUart0("Turning green LED status light off...\n");
            }
            else if(strcmp(temp_arg[1], "on") == 0)
            {
                putsUart0("Turning green LED status light on...\n");
            }
            else if (strcmp(temp_arg[1], "sample") == 0)
            {
                putsUart0("Toggling green LED status light every time sample is taken...\n");
            }
            else
                putsUart0("That is an incorrect LED command");
        }
        else if (isCommand("test", argCount))
        {
            putsUart0("Testing...\n");
        }
        else
        {
            putsUart0(temp_command);
            putsUart0(" is not specified. You might be missing arguments.\n");
        }
        argCount = 0;
    }
}
