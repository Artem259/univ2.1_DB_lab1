#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include "structures.h"

_Bool get_free_index(char file_name[], size_t *result)
{
    FILE *old_free = fopen(file_name,"ab+");
    size_t current;
    if(!fread(result, sizeof(*result), 1, old_free))
    {
        return 0;
    }
    FILE *new_free = fopen("TEMP_free.bin","ab+");
    while(fread(&current, sizeof(current), 1, old_free))
    {
        fwrite(&current, sizeof(current), 1, new_free);
    }
    fclose(old_free);
    fclose(new_free);
    remove(file_name);
    rename("TEMP_free.bin", file_name);
    return 1;
}

void insert_m(struct observatory *input)
{
    (*input).telescopes = 0; // при создании обсерватории, в ней нет телескопов
    FILE *obs_file = fopen("OBSERVATORIES.bin","ab+");
    FILE *index_file = fopen("index.bin","ab+");
    //-----------------------------------------------------------------------------------//
    // установка указателя на начало последней записи в index.bin, получение id последнего элемента
    if(!fseek(index_file, -(long)sizeof(struct index_structure), SEEK_END))
    {
        fread(&(*input).id, sizeof((*input).id), 1, index_file);
        (*input).id++; // увеличение id нового элемента на 1
    }
    else // предыдущих записей нет - id=0
    {
        (*input).id = 0;
    }
    //-----------------------------------------------------------------------------------//
    // получение индекса свободной ячейки в OBSERVATORIES.bin, установка указателя на нем
    size_t index;
    if(get_free_index("free_OBS",&index))
    {
        // свободная ячейка есть - установка указателя на ней в OBSERVATORIES.bin
        fseek(obs_file, (long)(index*sizeof(struct observatory)), SEEK_SET);
    }
    else
    {
        // свободных ячеек нет - запись в конец
        fseek(obs_file, 0, SEEK_END);
        index = filelength(fileno(obs_file)) / sizeof(struct observatory);
    }
    //-----------------------------------------------------------------------------------//
    // запись данных в OBSERVATORIES.bin
    fwrite(input ,sizeof(struct observatory), 1, obs_file);
    //-----------------------------------------------------------------------------------//
    // установка указателя в конце index.bin, запись данных о новом элементе
    struct index_structure curr_index = {(*input).id, index, 0};
    fseek(index_file, 0, SEEK_END);
    fwrite(&curr_index, sizeof(struct index_structure), 1, index_file);
    //-----------------------------------------------------------------------------------//
    fclose(obs_file);
    fclose(index_file);
}

_Bool get_m(size_t id, size_t *output_index, struct observatory *output_struct)
{
    struct index_structure curr_struct;
    FILE *index_file = fopen("index.bin","ab+");
    size_t left = 0;
    size_t right = filelength(fileno(index_file)) / sizeof(struct index_structure);
    size_t current;
    //-----------------------------------------------------------------------------------//
    // бинарный поиск по index.bin
    while(1)
    {
        current = (left+right)/2;
        fseek(index_file, (long)(current*sizeof(struct index_structure)), SEEK_SET);
        fread(&curr_struct, sizeof(struct index_structure), 1, index_file);
        if(curr_struct.id != id)
        {
            if(left == right)
            {
                fclose(index_file);
                return 0; //не найдено
            }
            if(curr_struct.id > id)
            {
                right = current;
            }
            else
            {
                if(left+1 == right)
                {
                    fclose(index_file);
                    return 0; //не найдено
                }
                left = current;
            }
        }
        else
        {
            break; // найдено
        }
    }
    //-----------------------------------------------------------------------------------//
    if(curr_struct.is_removed) return 0; // запись была удалена
    //-----------------------------------------------------------------------------------//
    // получение и возврат нужной записи из OBSERVATORIES.bin (output_struct), а также ее индекса в файле (output_index)
    FILE *obs_file = fopen("OBSERVATORIES.bin","ab+");
    fseek(obs_file, (long)(curr_struct.index*sizeof(struct observatory)), SEEK_SET);
    fread(output_struct, sizeof(struct observatory), 1, obs_file);
    fclose(obs_file);
    *output_index=curr_struct.index;
    return 1;
}

int DB_Terminal()
{
    printf("\nNow you are in the Database terminal.\nEnter \"db: help\" for a list of commands.\n\n");
    char s1[128], s2[128], s3[128], s4[128], s5[128], s6[128];
    int buff;
    while(1)
    {
        printf("db << ");
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
        struct observatory obs0 = {100, "Obs1", 10,10};
        struct observatory obs1 = {100, "Obs2", 10,10};
        struct observatory obs2 = {100, "Obs3", 10,10};
        struct observatory obs3 = {100, "Obs4", 10,10};
        struct observatory obs4 = {100, "Obs5", 10,10};
        struct observatory obs5 = {100, "Obs6", 10,10};
    insert_m(&obs0);
    insert_m(&obs1);
    insert_m(&obs2);
    insert_m(&obs3);
    insert_m(&obs4);
    insert_m(&obs5);

        size_t output_index;
        struct observatory output_struct;
    printf("0: %i(1)\n",get_m(0, &output_index, &output_struct));
    printf("   index: %zu\n   name: %s\n", output_index, output_struct.name);
    printf("1: %i(1)\n",get_m(1, &output_index, &output_struct));
    printf("   index: %zu\n   name: %s\n", output_index, output_struct.name);
    printf("2: %i(1)\n",get_m(2, &output_index, &output_struct));
    printf("   index: %zu\n   name: %s\n", output_index, output_struct.name);
    printf("3: %i(1)\n",get_m(3, &output_index, &output_struct));
    printf("   index: %zu\n   name: %s\n", output_index, output_struct.name);
    printf("4: %i(1)\n",get_m(4, &output_index, &output_struct));
    printf("   index: %zu\n   name: %s\n", output_index, output_struct.name);
    printf("5: %i(1)\n",get_m(5, &output_index, &output_struct));
    printf("   index: %zu\n   name: %s\n", output_index, output_struct.name);
    printf("6: %i(0)\n",get_m(6, &output_index, &output_struct));
    printf("   index: %zu\n   name: %s\n", output_index, output_struct.name);
    printf("100500: %i(0)\n",get_m(100500, &output_index, &output_struct));
    system("pause");
    return 0;
}

