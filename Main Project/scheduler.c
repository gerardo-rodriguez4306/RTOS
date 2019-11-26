// int rtosScheduler()
// {
//     /*
//         Example: Tasks A, B, C have priorities 0, 1, 2. They will be processed as follows:
//         A | A B | A C | A B | A C | A B | A B C
//
//         Actual program behavior:
//         We have ten tasks with priorities as high as 0 and as low as 15. The tasks/threads, along with the priorities, are:
//
//         Important: 0       alias --> A
//         oneshot:   4       alias --> B
//         errant:    8       alias --> C
//         flash4Hz:  8       alias --> D
//         shell:     8       alias --> E
//         uncooperative: 10  alias --> F
//         LengthyFn: 12      alias --> G
//         ReadKeys:  12      alias --> H
//         Debouce:   12      alias --> I
//         idle:      15      alias --> J
//
//         A | A | A | A B | A | A | A | A B C D E | A | AF | A | A B G H I | A | A | A J | A B C D E |
//         A | A | A | A B F | A | A | A | A B C D E G H I | A | A | A | A B | A | A F | A | A B C D E |
//
//         will have to use modulus operator on timer that ticks every time SysTick fires
//     */
//     bool ok;
//     static uint8_t task = 0xFF;
//     ok = false;
//     while (!ok)
//     {
//         task++;
//         if (task >= MAX_TASKS)
//             task = 0;
//         ok = (tcb[task].state == STATE_READY || tcb[task].state == STATE_UNRUN);
//     }
//     return task;
// }
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#define MAX_TASKS 12
#define READY 1
#define UNRUN 1

/*
    Both run_task and run_task_2 work for our purposes. Integration of either is up to you
*/
void run_task_2()
{
  static uint32_t iterator = 0;
  static uint32_t overflow[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  printf("%d\n", iterator);
  char *tasks[10] = {"A", "B", "C","D","E","F","G","H","I", "J"};
  int priorities[10] =    {0, 4, 8, 8, 8, 10, 12, 12, 12, 15};
  int tasks_to_run[10] =  {0, 0, 0, 0, 0,  0,  0,  0,  0,  0};
  int i = 0;
  for (i = 0; i < sizeof(priorities)/sizeof(int); i++)
  {
    if (priorities[i] == 0)
    {
      tasks_to_run[i] = 1;
      continue;
    }
    if (priorities[i] - 1 == overflow[i])
    {
      tasks_to_run[i] = 1;
      overflow[i] = 0;
    }
    else
      overflow[i]++;
  }
  for (i = 0; i < sizeof(priorities)/sizeof(int); i++)
  {
    if (tasks_to_run[i])
      printf(" %s |", tasks[i]);
  }
  printf("\n");
  iterator+=1;
}
void run_task()
{
  static uint32_t iterator = 0;
  printf("%d\n", iterator);
  char *tasks[10] = {"A", "B", "C","D","E","F","G","H","I", "J"};
  int priorities[10] =    {0, 4, 8, 8, 8, 10, 12, 12, 12, 15};
  int tasks_to_run[10] =  {0, 0, 0, 0, 0,  0,  0,  0,  0,  0};
  int i = 0;
  for (i = 0; i < sizeof(priorities)/sizeof(int); i++)
  {
    if (priorities[i] == 0)
    {
      tasks_to_run[i] = 1;
      continue;
    }
    if (iterator % priorities[i] == 0)
      tasks_to_run[i] = 1;
  }
  for (i = 0; i < sizeof(priorities)/sizeof(int); i++)
  {
    if (tasks_to_run[i])
      printf(" %s |", tasks[i]);
  }
  printf("\n");
  iterator+=1;
}
int main()
{
  while(1)
  {
    run_task_2();
    sleep(1);
  }
  return 0;
}
