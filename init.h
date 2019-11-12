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
