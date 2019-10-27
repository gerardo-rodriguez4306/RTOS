#include <stdio.h>
#include <stdint.h>
char * itoh(int number, char* output)
{
     int i = 9;
     uint32_t temp = number;
     while (i>2 && number > 0)
     {    
          if (number % 16 < 10)
               output[i] = (number % 16) + '0';
          else
               output[i] = (number % 16) + 87; //'a' + 10 = 87
          i--;
          number = number/16;
     }
     return output;
}
