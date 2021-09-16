#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define false 0
#define true 1
/*
void CleanLine(char str[])
{
    size_t correct_pointer=0, old_pointer=0;
    _Bool after_space = false;
    while(str[old_pointer]==' ') old_pointer++;
    while(str[old_pointer])
    {
        if(str[old_pointer]==' ' && !after_space)
        {
            after_space = true;
        }
        else if(str[old_pointer]==' ' && after_space)
        {
            old_pointer++;
            continue;
        }
        str[correct_pointer] = str[old_pointer];
        old_pointer++;
        correct_pointer++;
    }
    str[correct_pointer]='\0';
    strlwr(str);
}*/

int main()
{
    char buff1[256],buff2[256],buff3[256];
    while(true)
    {
        printf("Enter a string: ");
        scanf("%s %s %s", buff1, buff2, buff3);
        printf("---\n%s\n%s\n%s\n---", buff1, buff2, buff3);
        /*CleanLine(buff);
        printf("%s\n", buff);
        if(strstr(buff, "db")==&buff[0])
        {
            printf("This is Database!");
            if(strstr(buff, "exit")==&buff[2])
            {
                printf("This is Exit!");
                break;
            }
        }*/
    }
    return 0;
}

