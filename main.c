#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include "structures.h"

_Bool get_free_index(char file_name[], size_t *result)
{
    FILE *old_free = fopen(file_name,"ab+");
    FILE *new_free = fopen("TEMP_free.bin","ab+");
    size_t current;
    if(!fread(result, sizeof(*result), 1, old_free))
    {
        return 0;
    }
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
    while(left <= right)
    {
        current = (left+right)/2;
        fseek(index_file, (long)(current*sizeof(struct index_structure)), SEEK_SET);
        fread(&curr_struct, sizeof(struct index_structure), 1, index_file);
        if(curr_struct.id > id)
        {
            right = current;
        }
        else if(curr_struct.id < id)
        {
            left = current;
        }
        else
        {
            // найдено
            break;
        }
    }
    fclose(index_file);
    if(left>right) return 0; //не найдено
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
    DB_Terminal();
    return 0;
}

