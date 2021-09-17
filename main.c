#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct observatory
{
    size_t id; //KEY
    char name[64];
    float latitude; //широта (-90 south; 90 north)
    float longitude; //долгота (-180 west; 180 east)

    unsigned first_telescope;
};

struct index_structure
{
    size_t index;
    size_t id;

    _Bool is_removed;
};

struct telescope
{
    size_t id; //KEY
    char name[64];

    size_t next_telescope;
};

int

int DB_Terminal()
{
    printf("\nNow you are in the Database terminal.\nEnter \"db: help\" for a list of commands.\n\n");
    char s1[128], s2[128], s3[128], s4[128], s5[128], s6[128];
    int buff;
    while(1)
    {
        printf("db: ");
        scanf("%s %s %s %s %s", s1, s2, s3, s4, s5);
        strlwr(s1); strlwr(s2); strlwr(s3); strlwr(s4); strlwr(s5);
        while ((buff = getchar()) != '\n' && buff != EOF) { }
        if(!strcmp(s1, "help"))
        {
            printf("\nHELP\n");
        }
        else if(!strcmp(s1, "close"))
        {
            return 0;
        }
    }
}


int main()
{
    DB_Terminal();
    return 0;
}

