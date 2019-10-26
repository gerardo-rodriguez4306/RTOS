#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#define BLUE_LED       (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 2*4))) // on-board blue LED
#define RED_LED        (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 1*4))) // off-board red LED
#define GREEN_LED      (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 4*4))) // off-board green LED
#define YELLOW_LED     (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 3*4))) // off-board yellow LED
#define ORANGE_LED     (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 2*4))) // off-board orange LED

#define PUSH_BUTTON_0  (*((volatile uint32_t *)(0x42000000 + (0x400043FC-0x40000000)*32 + 2*4)))
#define PUSH_BUTTON_1  (*((volatile uint32_t *)(0x42000000 + (0x400043FC-0x40000000)*32 + 3*4)))
#define PUSH_BUTTON_2  (*((volatile uint32_t *)(0x42000000 + (0x400043FC-0x40000000)*32 + 4*4)))
#define PUSH_BUTTON_3  (*((volatile uint32_t *)(0x42000000 + (0x400043FC-0x40000000)*32 + 5*4)))
#define PUSH_BUTTON_4  (*((volatile uint32_t *)(0x42000000 + (0x400043FC-0x40000000)*32 + 6*4)))
#define PUSH_BUTTON_5  (*((volatile uint32_t *)(0x42000000 + (0x400043FC-0x40000000)*32 + 7*4)))

#define BLUE_LED_MASK 4
#define RED_LED_MASK 2
#define GREEN_LED_MASK 16
#define YELLOW_LED_MASK 8
#define ORANGE_LED_MASK 4

#define PUSH_BUTTON_0_MASK 4
#define PUSH_BUTTON_1_MASK 8
#define PUSH_BUTTON_2_MASK 16
#define PUSH_BUTTON_3_MASK 32
#define PUSH_BUTTON_4_MASK 64
#define PUSH_BUTTON_5_MASK 128
void initHw()
{
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);

    // Set GPIO ports to use APB (not needed since default configuration -- for clarity)
    SYSCTL_GPIOHBCTL_R = 0;

    // Enable GPIO port B and E peripherals
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA | SYSCTL_RCGC2_GPIOE | SYSCTL_RCGC2_GPIOF;

    // Configuring GPIO Port F
    GPIO_PORTF_DIR_R |= BLUE_LED_MASK;
    GPIO_PORTF_DR2R_R |= BLUE_LED_MASK;
    GPIO_PORTF_DEN_R |= BLUE_LED_MASK;
    // Configuring GPIO PORT E
    GPIO_PORTE_DIR_R |= RED_LED_MASK | GREEN_LED_MASK | YELLOW_LED_MASK | ORANGE_LED_MASK;
    GPIO_PORTE_DR2R_R |= RED_LED_MASK | GREEN_LED_MASK | YELLOW_LED_MASK | ORANGE_LED_MASK;
    GPIO_PORTE_DEN_R |= RED_LED_MASK | GREEN_LED_MASK | YELLOW_LED_MASK | ORANGE_LED_MASK;
    // Configuring GPIO PORT A for PBs
    GPIO_PORTA_DIR_R |= 0; //default -> all pbs will be inputs
    GPIO_PORTA_DEN_R |= PUSH_BUTTON_0_MASK | PUSH_BUTTON_1_MASK | PUSH_BUTTON_2_MASK | PUSH_BUTTON_3_MASK | PUSH_BUTTON_4_MASK | PUSH_BUTTON_5_MASK;
    GPIO_PORTA_PUR_R |= PUSH_BUTTON_0_MASK | PUSH_BUTTON_1_MASK | PUSH_BUTTON_2_MASK | PUSH_BUTTON_3_MASK | PUSH_BUTTON_4_MASK | PUSH_BUTTON_5_MASK;
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
void waitMicrosecond(uint32_t us)
{
                                              // Approx clocks per us
    __asm("WMS_LOOP0:   MOV  R1, #6");          // 1
    __asm("WMS_LOOP1:   SUB  R1, #1");          // 6
    __asm("             CBZ  R1, WMS_DONE1");   // 5+1*3
    __asm("             NOP");                  // 5
    __asm("             B    WMS_LOOP1");       // 5*3
    __asm("WMS_DONE1:   SUB  R0, #1");          // 1
    __asm("             CBZ  R0, WMS_DONE0");   // 1
    __asm("             B    WMS_LOOP0");       // 1*3
    __asm("WMS_DONE0:");                        // ---
                                                // 40 clocks/us + error
}
int main(void)
{
    initHw();
    RED_LED = GREEN_LED = ORANGE_LED = YELLOW_LED = BLUE_LED = 0;
    while(1)
    {
        waitMicrosecond(100000);
        if (!PUSH_BUTTON_0)
            RED_LED ^= 1;
        if (!PUSH_BUTTON_1)
            GREEN_LED ^= 1;
        if (!PUSH_BUTTON_2)
            BLUE_LED ^= 1;
        if (!PUSH_BUTTON_3)
            YELLOW_LED ^= 1;
        if (!PUSH_BUTTON_4)
            ORANGE_LED ^= 1;
        if (!PUSH_BUTTON_5)
            RED_LED = GREEN_LED = ORANGE_LED = YELLOW_LED = BLUE_LED ^= 1;
    }
	return 0;
}
