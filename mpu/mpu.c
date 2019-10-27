/* Program written by:                  Gerardo Rodriguez
 * Functions initHw(), putcUart0(), getcUart0(), and putsUart0() taken from: Dr. Jason Losh's Embedded Systems I code examples
 * Program written to meet the requirements of the Embedded Systems I course at the University of Texas at Arlington
 *
 * */

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "hex.h"
#define MAX_CHARS 80
typedef struct user_input
{
    char strInput[MAX_CHARS];
    char *temp_command;
    char *temp_arg[10];
    uint8_t argCount;

}user_input;
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
        result += (str[i] - 48) * 10;
        power_of_ten /= 10;
        i++;
    }
    return result;
}
uint8_t is_alphanumeric(char c)
{
    /*determines whether input is alphanumeric or not based on ASCII values*/
    uint8_t result[128] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
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
                            1, 1, 1, 0, 0, 0, 0, 0,
                            };
    return result[c];
}
void initHw()
{
    // Configure HW to work with 16 MHz XTAL, PLL enabled, system clock of 40 MHz
    // PWM is system clock / 2
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);

    // Set GPIO ports to use APB (not needed since default configuration -- for clarity)
    SYSCTL_GPIOHBCTL_R = 0;

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
void getsUart0(user_input *temp, uint8_t maxChars)
{
    uint8_t count = 0;
    char c;
    while (count < maxChars)
    {
        c = getcUart0();
        //if you've reached the max amount of characters, break (80 chars max)
        if (count == maxChars)
        {
            temp->strInput[count] = '\0';
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
            temp->strInput[count] = '\0';
            break;
        }
        //if an input is an uppercase letter, make it lowercase
        if (c >= 'A' && c <= 'Z')
        {
            c+=32;
            temp->strInput[count] = c;
        }
        //put whatever is in the buffer onto the screen
        else
            temp->strInput[count] = c;
        count++;
    }
}
void initMpu()
{
    NVIC_SYS_HND_CTRL_R = NVIC_SYS_HND_CTRL_MEM | NVIC_SYS_HND_CTRL_USAGE | NVIC_SYS_HND_CTRL_BUS;
    NVIC_MPU_CTRL_R = NVIC_MPU_CTRL_ENABLE | NVIC_MPU_CTRL_PRIVDEFEN;

}

void FaultISR(void)
{
    //
    // Enter an infinite loop.
    //
    putsUart0("hard fault\n");
    while(1)
    {
    }
}
void BusFaultISR(void)
{
    putsUart0("bus fault\n");
}

void UsageFaultISR(void)
{
    putsUart0("usage fault\n");
}

void MpuISR()
{
    putsUart0("MPU fault in process \n");
    while(1);
}
/* Adds an MPU region for Flash. It's main purpose is to give read, right, execute access to Flash. Second lowest priority (MPU # 1). Size (256 kb)
 * All subregions enabled. Nonshareable, noncacheable, nonbufferable.
 */
void addFlashRegion()
{
    NVIC_SYS_HND_CTRL_R = NVIC_SYS_HND_CTRL_MEM | NVIC_SYS_HND_CTRL_USAGE | NVIC_SYS_HND_CTRL_BUS;
    NVIC_MPU_CTRL_R = NVIC_MPU_CTRL_ENABLE | NVIC_MPU_CTRL_PRIVDEFEN;
    NVIC_MPU_NUMBER_R = 0x1;
    NVIC_MPU_BASE_R = 0x0;
    NVIC_MPU_ATTR_R = 0x302003F; //RES: 000 XN: 0 RES: 0 AP: 011 RES: 00 TEX: 000 SCB: 010 SRD: 00000000 RES: 00 SIZE: 10001 ENABLE: 1
	
    // TBD: R/W to Flash for testing
}

/* Adds an MPU region to the entire memory map (Size : 4gb). It's main purpose is to give read right access (no execute) to SRAM and peripherals. Has lowest priority (MPU # 0) so
 * it doesn't affect other MPU regions. All subregions enabled. Nonshareable, noncacheable, nonbufferable(?) 
 */
void addSRAMRegion()
{
    NVIC_SYS_HND_CTRL_R = NVIC_SYS_HND_CTRL_MEM | NVIC_SYS_HND_CTRL_USAGE | NVIC_SYS_HND_CTRL_BUS;
    NVIC_MPU_CTRL_R = NVIC_MPU_CTRL_ENABLE | NVIC_MPU_CTRL_PRIVDEFEN;
    NVIC_MPU_NUMBER_R = 0x0;
    NVIC_MPU_BASE_R = 0x0;
    NVIC_MPU_ATTR_R = 0b00010011000000000000000000111111; // XN: 1 AP: 011 SIZE: 11111: ENABLE: 1
	
    // TBD: R/W to SRAM and Peripherals for testing
}


int main(void)
{
    initHw();
    initMpu();
    __asm("  MOV r0, #0x1");
    __asm("  MSR CONTROL, r0");
	return 0;
}
