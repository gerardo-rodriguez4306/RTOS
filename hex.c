#include <stdio.h>
#include <stdint.h>
void itoh(int number, char* output)
{
     int i = 9;
     uint32_t temp = number;
     while (i>2 && number > 0)
     {    
          if (number % 16 < 10)
               output[i] = (number % 16) + '0';
          else
               output[i] = (number % 16) + 87;
          i--;
          number = number/16;
     }
}
int main()
{
     int i = 0;
     char input[11] = {'0','x','0','0','0','0','0','0','0','0','\0'};
     while (i < 100000)
     {
          itoh(i, input);
          printf("%d is %s in hex\n", i, input);
          i+=2;
     }
     return 0;
}
