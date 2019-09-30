/* Program written by:                  Gerardo Rodriguez
 * Functions initHw(), putcUart0(), getcUart0(), and putsUart0() taken from: Dr. Jason Losh's Embedded Systems I code examples
 * Program written to meet the requirements of the Embedded Systems I course at the University of Texas at Arlington
 *
 * */

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"

#define MAX_CHARS 80
#define RED_LED (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 1*4)))

/* Please make these variables local to the shell function */

#define RED_LED_MASK 2
char strInput[MAX_CHARS];
char *temp_command;
char *temp_arg[10];
uint8_t argCount;


/* Please make these variables local to the shell function */

/* adding a putsUart0 function declaration so string functions know about it */
void putsUart0(const char* str);
/* String handling functions */

uint8_t strlen(const char* str)
{
    uint8_t result = 0;
    while (str[result++] != '\0');
    return result;
}
int strcmp(const char* str1, const char* str2)
{
    uint8_t i = 0;
    if (strlen(str1) != strlen(str2)) return -1;
    while (str1[i] != '\0' && str2[i] != '\0')
    {
        if ( str1[i] < str2[i] )
            return -1;
        else if ( str1[i] > str2[i] )
            return 1;
        else
            i++;
    }
    return 0;
}
int atoi(const char* str)
{
    uint8_t i = 0;
    int result = 0;
    uint8_t length_of_string = strlen(str);
    int power_of_ten;
    if (str[0] == '-')
    {
        power_of_ten = -1;
        i++;
    }
    else
        power_of_ten = 1;
    while (i < length_of_string - 1)
    {
        power_of_ten *= 10;
        i++;
    }
    while (i < length_of_string)
    {
//        if (str[i] > '9' || str[i] < '0')
//        {
//            putsUart0(str); putsUart0(" contains a character. Could not convert to integer.\n");
//            break;
//        }
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
                            0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
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
    SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOA | SYSCTL_RCGC2_GPIOF;

    // Configuring GPIO Port F
    GPIO_PORTF_DIR_R |= RED_LED_MASK;
    GPIO_PORTF_DR2R_R |= RED_LED_MASK;
    GPIO_PORTF_DEN_R |= RED_LED_MASK;
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
        if (c == 8 || c == 127)
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
void ps()
{
    putsUart0("PS called");
}
void ipcs()
{
    putsUart0("IPCS called");
}
void kill(int pid)
{
    putsUart0("# killed");
}
void pi(bool on)
{
    if (on)
        putsUart0("PI on");
    else
        putsUart0("PI off");
}
void preempt(bool on)
{
    if (on)
        putsUart0("preempt on");
    else
        putsUart0("preempt off");
}
void sched(bool prio_on)
{
    if (prio_on)
        putsUart0("sched prio");
    else
        putsUart0("sched rr");
}
void pidof(char name[])
{
    putsUart0(name);
    putsUart0(" launched");
}
void shell(void)
{
    bool _on_;
    char *menu =  "help menu: \n"
                  "help:\t\t displays help menu\n"
                  "reboot:\t\t reboots the microcontroller.\n"
                  "ipcs:\t\t displays the inter-process (thread) communication state.\n";
    char *menu1 = "ps:\t\t displays the process (thread) information.\n"
                  "kill PID:\t kills the process (thread) with the matching PID.\n"
                  "pi ON|OFF:\t turns priority inheritance on or off. \n";
    char *menu2 = "preempt ON|OFF:\t turns preemption on or off.\n"
                  "sched PRIO | RR: selected priority or round-robin scheduling.\n"
                  "pidof proc_name: displays the PID of the process (thread).\n"
                  "proc_name &: \t runs the selected program in the background.\n";
    while (true)
    {
        putsUart0("\nrtos-shell-0.1~ ");
        getsUart0(strInput, MAX_CHARS);
        tokenize_string(strInput);

        /*tokenizing string, setting argCount, getting arguments, and determining command*/
        if (temp_arg[1][0] == '&')
            RED_LED ^= 1;
        else if (isCommand("help", argCount))
        {
            putsUart0(menu); putsUart0(menu1); putsUart0(menu2);
        }
        else if (isCommand("reboot", argCount))
        {
            putsUart0("System rebooting...\n");
            break;
        }
        else if (isCommand("ps", argCount))
            ps();
        else if (isCommand("ipcs", argCount))
            ipcs();
        else if (isCommand("kill", argCount))
            kill(atoi(temp_arg[1]));
        else if (isCommand("pi", argCount))
        {
            if (strcmp("on", temp_arg[1]) == 0)
            {
                _on_ = true;
                pi(_on_);
            }
            else if (strcmp("off", temp_arg[1]) == 0)
            {
                _on_ = false;
                pi(_on_);
            }
            else
                putsUart0("Argument supplied was not \"on\" or \"off\". Try again.");
        }
        else if (isCommand("preempt", argCount))
        {
            if (strcmp("on", temp_arg[1]) == 0)
            {
                _on_ = true;
                preempt(_on_);
            }
            else if (strcmp("off", temp_arg[1]) == 0)
            {
                _on_ = false;
                preempt(_on_);
            }
            else
                putsUart0("Argument supplied was not \"on\" or \"off\". Try again.");
        }
        else if (isCommand("sched", argCount))
        {
            if (strcmp("prio", temp_arg[1]) == 0)
            {
                _on_ = true;
                sched(_on_);
            }
            else if (strcmp("rr", temp_arg[1]) == 0)
            {
                _on_ = false;
                sched(_on_);
            }
            else
                putsUart0("Argument supplied was not \"prio\" or \"rr\". Try again.");
        }
        else if (isCommand("pidof", argCount))
            pidof(temp_arg[1]);
        else
        {
            putsUart0(temp_command);
            putsUart0(" is not specified. You might be missing arguments.");
        }
        argCount = 0;
        // reboot_debug++;
    }
    return;
}
int main(void)
{
    // Initialize hardware
    initHw();
    // begin shell
    shell();
    return 0;
}