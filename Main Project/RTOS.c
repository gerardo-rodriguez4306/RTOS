// RTOS Framework - Fall 2019
// J Losh

// Student Name:
// TO DO: Add your name on this line.  Do not include your ID number in the file.

// Add xx_ prefix to all files in your project
// xx_rtos.c
// xx_tm4c123gh6pm_startup_ccs.c
// xx_other files
// (xx is a unique number that will be issued in class)
// Please do not change any function name in this code or the thread priorities

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL Evaluation Board
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// 6 Pushbuttons and 5 LEDs, UART
// LEDS on these pins:
// Blue:   PF2 (on-board)
// Red:    PE1
// Orange: PE2
// Yellow: PE3
// Green:  PE4
// PBs on these pins
// PB0:    PA2
// PB1:    PA3
// PB2:    PA4
// PB3:    PA5
// PB4:    PA6
// PB5:    PA7

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "tm4c123gh6pm.h"
#include "hex.h"

extern void loadR0();
extern uint32_t getR0();
extern uint32_t getR1();
extern uint32_t getR2();
extern uint32_t getR3();
extern uint32_t getR12();
extern uint32_t getSP();
extern uint32_t getPC();
extern uint32_t getLR();
extern uint32_t getPSR();
extern uint32_t getPSP();
extern void PushR4Thru11();
extern void PopR4Thru11();

// REQUIRED: correct these bitbanding references for the off-board LEDs
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

//-----------------------------------------------------------------------------
// RTOS Defines and Kernel Variables
//-----------------------------------------------------------------------------

// function pointer
typedef void (*_fn)();

// semaphore
#define MAX_SEMAPHORES 5
#define MAX_QUEUE_SIZE 5
#define ALIGN1024(s)         (((((s) - 1) >> 10) << 10) + 1024)
struct semaphore
{
    uint16_t count;
    uint16_t queueSize;
    uint32_t processQueue[MAX_QUEUE_SIZE]; // store task index here
} semaphores[MAX_SEMAPHORES];
uint8_t semaphoreCount = 0;

struct semaphore *keyPressed, *keyReleased, *flashReq, *resource;

// task
#define STATE_INVALID    0 // no task
#define STATE_UNRUN      1 // task has never been run
#define STATE_READY      2 // has run, can resume at any time
#define STATE_DELAYED    3 // has run, but now awaiting timer
#define STATE_BLOCKED    4 // has run, but now blocked by semaphore

#define MAX_TASKS 12       // maximum number of valid tasks
uint8_t taskCurrent = 0;   // index of last dispatched task
uint8_t taskCount = 0;     // total number of valid tasks
char str[40];

// REQUIRED: add store and management for the memory used by the thread stacks
//           thread stacks must start on 1 kiB boundaries so mpu can work correctly

struct _tcb
{
    uint8_t state;                 // see STATE_ values above
    void *pid;                     // used to uniquely identify thread
    void *spInit;                  // location of original stack pointer
    void *sp;                      // location of stack pointer for thread
    int8_t priority;               // 0=highest to 15=lowest
    int8_t currentPriority;        // used for priority inheritance
    uint32_t ticks;                // ticks until sleep complete
    char name[16];                 // name of task used in ps command
    void *semaphore;               // pointer to the semaphore that is blocking the thread
} tcb[MAX_TASKS];

//uint32_t ticka = tcb[1].ticks;

//-----------------------------------------------------------------------------
// RTOS Kernel Functions
//-----------------------------------------------------------------------------

// REQUIRED: initialize systick for 1ms system timer ---> initHw()
void initRtos()
{
    uint8_t i;
    // no tasks running
    taskCount = 0;
    // clear out tcb records
    for (i = 0; i < MAX_TASKS; i++)
    {
        tcb[i].state = STATE_INVALID;
        tcb[i].pid = 0;
    }
}

// REQUIRED: Implement prioritization to 16 levels ---> 
int rtosScheduler()
{
    /*
        Example: Tasks A, B, C have priorities 0, 1, 2. They will be processed as follows:
        A | A B | A C | A B C | A | A B | A C | A B C
        
        Actual program behavior:
        We have ten tasks with priorities as high as 0 and as low as 15. The tasks/threads, along with the priorities, are:
        
        Important: 0       alias --> A
        oneshot:   4       alias --> B
        errant:    8       alias --> C
        flash4Hz:  8       alias --> D
        shell:     8       alias --> E
        uncooperative: 10  alias --> F
        LengthyFn: 12      alias --> G
        ReadKeys:  12      alias --> H
        Debouce:   12      alias --> I
        idle:      15      alias --> J
        
        A | A | A | A B | A | A | A | A B C C C | A | AF | A | A B G H I | A | A | A J | A A B C C C |
        A | A | A | A B F | A | A | A | A B C C C | A | A | A | A B | A | A F | A | A B C C C | 
        
        will have to use modulus operator on timer that ticks every time SysTick fires
    */
    bool ok;
    static uint8_t task = 0xFF;
    ok = false;
    while (!ok)
    {
        task++;
        if (task >= MAX_TASKS)
            task = 0;
        ok = (tcb[task].state == STATE_READY || tcb[task].state == STATE_UNRUN);
    }
    return task;
}

bool createThread(_fn fn, const char name[], uint8_t priority, uint32_t stackBytes)
{

    stackBytes = ALIGN1024(stackBytes);
    bool ok = false;
    uint8_t i = 0;
    int j;
    bool found = false;
    // REQUIRED: store the thread name
    // add task if room in task list
    // allocate stack space for a thread and assign to sp below
    if (taskCount < MAX_TASKS)
    {
        // make sure fn not already in list (prevent reentrancy)
        while (!found && (i < MAX_TASKS))
        {
            found = (tcb[i++].pid ==  fn);
        }
        if (!found)
        {
            // find first available tcb record
            i = 0;
            while (tcb[i].state != STATE_INVALID) {i++;}
            tcb[i].state = STATE_UNRUN;
            tcb[i].pid = fn;
            //need to implement
            for(j = 0; j < sizeof(name); j++)
                tcb[i].name[j] = name[j];
            if(i)
                tcb[i].spInit = tcb[i-1].spInit - stackBytes;
            else
                tcb[i].spInit = 0x20007C00;
            tcb[i].sp = tcb[i].spInit;
            tcb[i].priority = priority;
            tcb[i].currentPriority = priority;
            // increment task count
            taskCount++;
            ok = true;
        }
    }
    // REQUIRED: allow tasks switches again
    return ok;
}

// REQUIRED: modify this function to restart a thread
void restartThread(_fn fn)
{
}

// REQUIRED: modify this function to destroy a thread
// REQUIRED: remove any pending semaphore waiting
// NOTE: see notes in class for strategies on whether stack is freed or not
void destroyThread(_fn fn)
{
}

// REQUIRED: modify this function to set a thread priority
void setThreadPriority(_fn fn, uint8_t priority)
{
}

struct semaphore* createSemaphore(uint8_t count)
{
    struct semaphore *pSemaphore = 0;
    if (semaphoreCount < MAX_SEMAPHORES)
    {
        pSemaphore = &semaphores[semaphoreCount++];
        pSemaphore->count = count;
    }
    return pSemaphore;
}

// REQUIRED: modify this function to start the operating system, using all created tasks
void startRtos()
{
    static int a;
    a = rtosScheduler();
    int *pid, spInit;
    spInit = tcb[a].spInit;
    int * firstTask = 0x20002000;
    *firstTask = spInit;
    loadR0();
    asm("  LDR R0, [R0]");
    asm("  MSR PSP, R0");
    //set Stack Pointer (SP) to Process Stack Pointer (PSP) instead of Main Stack Pointer (MSP)
    __asm("  MOV R0, #0x02");
    __asm("  MSR CONTROL, R0");
    pid = tcb[a].pid;
    //idle();
    firstTask = 0x20002000;
    *firstTask = pid;
    loadR0();
    asm("  LDR PC, [R0]");
    //set SP to spInit
    //set PC to pid
}

// REQUIRED: modify this function to yield execution back to scheduler using pendsv
// push registers, call scheduler, pop registers, return to new function
void yield()
{
    asm("  SVC #1");

}

// REQUIRED: modify this function to support 1ms system timer
// execution yielded back to scheduler until time elapses using pendsv
// push registers, set state to delayed, store timeout, call scheduler, pop registers,
// return to new function (separate unrun or ready processing)
void sleep(uint32_t tick)
{
    tcb[taskCurrent].ticks = tick;
    tcb[taskCurrent].state = STATE_DELAYED;
    NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;
}

// REQUIRED: modify this function to wait a semaphore with priority inheritance
// return if avail (separate unrun or ready processing), else yield to scheduler using pendsv
void wait(struct semaphore *pSemaphore)
{
}

// REQUIRED: modify this function to signal a semaphore is available using pendsv
void post(struct semaphore *pSemaphore)
{
}

// REQUIRED: modify this function to add support for the system timer
// REQUIRED: in preemptive code, add code to request task switch
void systickIsr()
{
    int i;
    //putsUart0("Test\n\r");
    for(i = 0; i < 12; i++)
    {
        if(tcb[i].state == STATE_DELAYED)
        {
            tcb[i].ticks -= 1;
            if(tcb[i].ticks == 0)
                tcb[i].state = STATE_READY;
            //sprintf(str, "\r\ntcb[%d].ticks : %d", taskCurrent, tcb[taskCurrent].ticks);
            //putsUart0(str);
        }
        /*if(tcb[taskCurrent].ticks == 0)
        {
            putsUart0("3\n\r");
            tcb[taskCurrent].state = STATE_READY;
            putsUart0("4\n\r");
            //sprintf(str, "\r\ntcb[%d].state : %d", taskCurrent, tcb[taskCurrent].state);
            //putsUart0(str);
        }*/
    }
}

// REQUIRED: in coop and preemptive, modify this function to add support for task switching
// REQUIRED: process UNRUN and READY tasks differently
// total of 23 lines of code, (including braces and comments etc), calls some functions along the way)
void pendSvIsr()
{
    //SAVING CONTEXT
    //must push R4-11 onto PSP, everything that wasn't saved before
    PushR4Thru11();
    //can't use push command, puts it on MSP. Have to use STR commands. Must predecrement and store.
    //tcb[task_current].sp = PSP
    tcb[taskCurrent].sp = getPSP();

    //need to get the next task
    //set PSP = tcb[task_current].sp
    taskCurrent = rtosScheduler();
    int * firstTask = 0x20002000;
    *firstTask = tcb[taskCurrent].sp;
    loadR0();
    asm("  LDR R0, [R0]");
    asm("  MSR PSP, R0");
    if(tcb[taskCurrent].state == STATE_READY)
        PopR4Thru11();
    else if (tcb[taskCurrent].state == STATE_UNRUN)
        pushP();

    //if(STATE = ready) pop R4-R11 from PSP
    //else push values to PSP to make it look like it had run before, random values*/
}

void pushP()
{
    int *temp = 0x20002000;
    *temp = 0x01000000;
    loadR0();
    asm("  LDR R0, [R0]");
    asm("  MRS R1, PSP");
    asm("  SUB R1, R1, #4");
    asm("  STR R0, [R1]"); //pushing PSR onto process stack
    *temp = tcb[taskCurrent].pid;
    loadR0();
    asm("  LDR R0, [R0]");
    asm("  MRS R1, PSP");
    asm("  SUB R1, R1, #8");
    asm("  STR R0, [R1]"); //pushing PC onto process stack
    asm("  SUB R1, R1, #24");
    asm("  MSR PSP, R1");

}

uint8_t getServiceNumber()
{
    asm("  MRS R0, PSP");
    asm("  LDR R0, [R0,#24]"); //Loading value of PC that was pushed on to Process Stack
    asm("  LDR R0, [R0,#-2]"); //Stepping back one instruction from PC
    uint8_t i = getR0();
    return i;
}
// REQUIRED: modify this function to add support for the service call
// REQUIRED: in preemptive code, add code to handle synchronization primitives
void svCallIsr()
{
    asm("  MRS R0, PSP");
    asm("  LDR R0, [R0,#24]");
    asm("  LDR R0, [R0,#-2]");
    uint8_t i = getServiceNumber();
    switch (i)
    {
        case 1:
            NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;
            break;
    }
}

// REQUIRED: code this function
void mpuFaultIsr()
{
}

// REQUIRED: code this function
void hardFaultIsr()
{
}

// REQUIRED: code this function
void busFaultIsr()
{
}

// REQUIRED: code this function
void usageFaultIsr()
{
}

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
// REQUIRED: Add initialization for blue, orange, red, green, and yellow LEDs
//           5 pushbuttons, and uart
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

    NVIC_ST_CTRL_R = 0;
    NVIC_ST_RELOAD_R = 0x9C3F; //0x2625A00; //0x9C3F
    NVIC_ST_CURRENT_R = 1;
    NVIC_ST_CTRL_R = 0x7;
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
// Approximate busy waiting (in units of microseconds), given a 40 MHz system clock
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

// REQUIRED: add code to return a value from 0-31 indicating which of 5 PBs are pressed
uint8_t readPbs()
{
    return 0;
}

//-----------------------------------------------------------------------------
// YOUR UNIQUE CODE
// REQUIRED: add any custom code in this space
//-----------------------------------------------------------------------------

// ------------------------------------------------------------------------------
//  Task functions
// ------------------------------------------------------------------------------

// one task must be ready at all times or the scheduler will fail
// the idle task is implemented for this purpose
void idle()
{
    tcb[taskCurrent].state = STATE_READY;
    while(true)
    {
//        ORANGE_LED = 1;
//        waitMicrosecond(1000);
//        ORANGE_LED = 0;
        //putsUart0("IDLIN\r\n");
        yield();
    }
}

void idle2()
{
    tcb[taskCurrent].state = STATE_READY;
    while(true)
    {
//        ORANGE_LED = 1;
//        waitMicrosecond(1000);
//        ORANGE_LED = 0;
        putsUart0("IDLIN2\r\n");
        yield();
    }
}

void flash4Hz()
{
    tcb[taskCurrent].state = STATE_READY;
    while(true)
    {
        GREEN_LED ^= 1;
        //putsUart0("FLASH\r\n");
        sleep(125);
    }
}

void oneshot()
{
    while(true)
    {
        wait(flashReq);
        YELLOW_LED = 1;
        sleep(1000);
        YELLOW_LED = 0;
    }
}

void partOfLengthyFn()
{
    // represent some lengthy operation
    waitMicrosecond(990);
    // give another process a chance to run
    yield();
}

void lengthyFn()
{
    uint16_t i;
    while(true)
    {
        wait(resource);
        for (i = 0; i < 5000; i++)
        {
            partOfLengthyFn();
        }
        RED_LED ^= 1;
        post(resource);
    }
}

void readKeys()
{
    uint8_t buttons;
    while(true)
    {
        wait(keyReleased);
        buttons = 0;
        while (buttons == 0)
        {
            buttons = readPbs();
            yield();
        }
        post(keyPressed);
        if ((buttons & 1) != 0)
        {
            YELLOW_LED ^= 1;
            RED_LED = 1;
        }
        if ((buttons & 2) != 0)
        {
            post(flashReq);
            RED_LED = 0;
        }
        if ((buttons & 4) != 0)
        {
            restartThread(flash4Hz);
        }
        if ((buttons & 8) != 0)
        {
            destroyThread(flash4Hz);
        }
        if ((buttons & 16) != 0)
        {
            setThreadPriority(lengthyFn, 4);
        }
        yield();
    }
}

void debounce()
{
    uint8_t count;
    while(true)
    {
        wait(keyPressed);
        count = 10;
        while (count != 0)
        {
            sleep(10);
            if (readPbs() == 0)
                count--;
            else
                count = 10;
        }
        post(keyReleased);
    }
}

void uncooperative()
{
    while(true)
    {
        while (readPbs() == 8)
        {
        }
        yield();
    }
}

void errant()
{
    uint32_t* p = (uint32_t*)0x20000000;
    while(true)
    {
        while (readPbs() == 32)
        {
            *p = 0;
        }
        yield();
    }
}

void important()
{
    while(true)
    {
        wait(resource);
        BLUE_LED = 1;
        sleep(1000);
        BLUE_LED = 0;
        post(resource);
    }
}

// REQUIRED: add processing for the shell commands through the UART here
void shell()
{
    while (true)
    {
    }
}

void unPrivilegedMode()
{
    __asm("  MOV R0, #0x01");
    __asm("  MSR  CONTROL, R0");
}


//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(void)
{
    bool ok;

    // Initialize hardware
    initHw();
    initRtos();

    // Power-up flash
    GREEN_LED = 1;
    waitMicrosecond(250000);
    GREEN_LED = 0;
    waitMicrosecond(250000);

    // Initialize semaphores
    keyPressed = createSemaphore(1);
    keyReleased = createSemaphore(0);
    flashReq = createSemaphore(5);
    resource = createSemaphore(1);



    // Add required idle process at lowest priority
    ok =  createThread(idle, "Idle", 15, 1024);
    //ok =  createThread(idle2, "Idle2", 15, 1024);

    // Add other processes

 // ok &= createThread(lengthyFn, "LengthyFn", 12, 1024);
    ok &= createThread(flash4Hz, "Flash4Hz", 8, 1024);
 /* ok &= createThread(oneshot, "OneShot", 4, 1024);
    ok &= createThread(readKeys, "ReadKeys", 12, 1024);
    ok &= createThread(debounce, "Debounce", 12, 1024);
    ok &= createThread(important, "Important", 0, 1024);
    ok &= createThread(uncooperative, "Uncoop", 10, 1024);
    ok &= createThread(errant, "Errant", 8, 1024);
    ok &= createThread(shell, "Shell", 8, 1024);
*/
    // Start up RTOS
    if (ok)
        startRtos(); // never returns
    else
        RED_LED = 1;

    return 0;
}
