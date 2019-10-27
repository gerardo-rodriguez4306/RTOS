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
int main()
{
     int i = 0;
     char input[11] = {'0','x','0','0','0','0','0','0','0','0','\0'};
     while (i < 1000)
     {
          char * c;
          c = itoh(i, input);
          printf("%d is %s in hex\n", i, c);
          i+=2;
     }
     return 0;
}
