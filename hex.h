#include <stdio.h>
#include <stdint.h>

/*converts int into a character string as a hexidecimal number*/
char * itoh(uint32_t number, char* output)
{
     int i = 9;
     while (i>1 && number > 0)
     {
          if (number % 16 < 10)
               output[i] = (number % 16) + '0';
          else
               output[i] = (number % 16) + 'A' - 10;
          i--;
          number = number/16;
     }
     return output;
}
/*sets last 8 bits of input string to 0 so it is of the form 0x00000000, resetting it for later use*/
void flush(char* input)
{
    int i = 2;
    for(i; i < 10; i++)
        input[i] = '0';
}
