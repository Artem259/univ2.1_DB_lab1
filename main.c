#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include "structures.h"

_Bool get_free_index(char file_name[], size_t *result)
{
    FILE *old_free = fopen(file_name,"rb+");
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
    FILE *obs_file = fopen("OBSERVATORIES.bin","rb+");
    FILE *index_file = fopen("index.bin","rb+");
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
    if(get_free_index("free_OBS.bin",&index))
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
    FILE *index_file = fopen("index.bin","rb+");
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
    FILE *obs_file = fopen("OBSERVATORIES.bin","rb+");
    fseek(obs_file, (long)(curr_struct.index*sizeof(struct observatory)), SEEK_SET);
    fread(output_struct, sizeof(struct observatory), 1, obs_file);
    fclose(obs_file);
    *output_index=curr_struct.index;
    return 1;
}
_Bool insert_s(size_t id, struct telescope *input)
{
    (*input).next_telescope = 0;
    size_t obs_index, tel_index;
    struct observatory obs_struct;
    if(!get_m(id, &obs_index, &obs_struct)) // нет обсерватории - некуда добавлять телескоп
    {
        return 0; //ошибка добавления
    }
    FILE *tel_file = fopen("TELESCOPES.bin","rb+");
    //-----------------------------------------------------------------------------------//
    // получение индекса свободной ячейки в TELESCOPES.bin, установка указателя на нем
    if(get_free_index("free_TEL.bin",&tel_index))
    {
        // свободная ячейка есть - установка указателя на ней в TELESCOPES.bin
        fseek(tel_file, (long)(tel_index*sizeof(struct telescope)), SEEK_SET);
    }
    else
    {
        // свободных ячеек нет - запись в конец
        fseek(tel_file, 0, SEEK_END);
        tel_index = filelength(fileno(tel_file)) / sizeof(struct telescope);
    }
    //-----------------------------------------------------------------------------------//
    if(obs_struct.telescopes == 0) // добавляемый телескоп - первый
    {
        obs_struct.telescopes = 1;
        obs_struct.telescope_index = tel_index;
        (*input).id = 0;
    }
    else
    {
        FILE *tel_file_help = fopen("TELESCOPES.bin","rb+");
        struct telescope curr_tel;
        size_t curr_tel_index = obs_struct.telescope_index;
        //-----------------------------------------------------------------------------------//
        // поиск последнего телескопа в цепочке
        for(size_t i=0 ; i<obs_struct.telescopes ; i++)
        {
            fseek(tel_file_help, (long)(curr_tel_index*sizeof(struct telescope)), SEEK_SET);
            fread(&curr_tel, sizeof(struct telescope), 1, tel_file_help);
            curr_tel_index = curr_tel.next_telescope;
        }
        curr_tel.next_telescope = tel_index; // закрепление индекса нового телескопа в последнем
        //-----------------------------------------------------------------------------------//
        // перезапись последнего телескопа на то же место в TELESCOPES.bin
        fseek(tel_file_help, (long)(-sizeof(struct telescope)), SEEK_CUR);
        fwrite(&curr_tel, sizeof(struct telescope), 1, tel_file_help);
        fclose(tel_file_help);
        //-----------------------------------------------------------------------------------//
        (*input).id = curr_tel.id+1; // id нового телескопа = id прошлого + 1
        obs_struct.telescopes++; // в обсерватории на 1 телескоп больше
    }
    fwrite(input, sizeof(struct telescope), 1, tel_file); // запись нового телескопа в TELESCOPES.bin
    fclose(tel_file);
    //-----------------------------------------------------------------------------------//
    // перезапись обсерватории в OBSERVATORIES.bin
    FILE *obs_file = fopen("OBSERVATORIES.bin","rb+");
    fseek(obs_file, (long)(obs_index*sizeof(struct observatory)), SEEK_SET);
    fwrite(&obs_struct, sizeof(struct observatory), 1, obs_file);
    fclose(obs_file);
    //-----------------------------------------------------------------------------------//
    return 1;
}

void print_observatories()
{
    FILE *obs_file = fopen("OBSERVATORIES.bin","rb+");
    FILE *index_file = fopen("index.bin","rb+");
    struct observatory curr_obs;
    struct index_structure curr_index;
    printf("+----------+----------------------------------------+----------+-----------+----------+------------+\n");
    printf("|    id    |                  Name                  | Latitude | Longitude | Altitude | Telescopes |\n");
    printf("+----------+----------------------------------------+----------+-----------+----------+------------+\n");
    while(fread(&curr_index, sizeof(struct index_structure), 1, index_file))
    {
        if(curr_index.is_removed) continue;
        fseek(obs_file, (long)(curr_index.index*sizeof(struct observatory)), SEEK_SET);
        fread(&curr_obs, sizeof(struct observatory), 1, obs_file);
        printf("|%10zu|%40.40s|%10.4f|%11.4f|%10.1f|%12zu|\n",
               curr_obs.id, curr_obs.name, curr_obs.latitude, curr_obs.longitude, curr_obs.altitude, curr_obs.telescopes);
    }
    printf("+----------+----------------------------------------+----------+-----------+----------+------------+\n");
    fclose(obs_file);
    fclose(index_file);
}
void print_telescopes(size_t id)
{
    size_t obs_index;
    struct observatory obs_struct;
    //-----------------------------------------------------------------------------------//
    // проверка особых случаев
    if(!get_m(id, &obs_index, &obs_struct)) // проверка на существование обсерватории
    {
        printf("There is no %zu observatory in database.\n", id); // такой обсерватории уже/еще нет
        return;
    }
    if(obs_struct.telescopes == 0)
    {
        printf("Observatory %zu has no telescopes yet.\n", id); // в обсерватории нет телескопов
        return;
    }
    //-----------------------------------------------------------------------------------//
    // вывод таблицы
    FILE *tel_file = fopen("TELESCOPES.bin","rb+");
    struct telescope curr_tel;
    size_t tel_index = obs_struct.telescope_index;
    printf("Observatory: %-10zu\n", id);
    printf("+----------+----------------------------------------+----------+--------------+\n");
    printf("|    id    |                  Name                  | Diameter | Focal length |\n");
    printf("+----------+----------------------------------------+----------+--------------+\n");
    for(size_t i=0 ; i<obs_struct.telescopes ; i++)
    {
        fseek(tel_file, (long)(tel_index*sizeof(struct telescope)), SEEK_SET);
        fread(&curr_tel, sizeof(struct telescope), 1, tel_file);
        printf("|%10zu|%40.40s|%10.3f|%14.3f|\n",
               curr_tel.id, curr_tel.name, curr_tel.diameter, curr_tel.focal_length);
        tel_index = curr_tel.next_telescope;
    }
    printf("+----------+----------------------------------------+----------+--------------+\n");
    fclose(tel_file);
    //-----------------------------------------------------------------------------------//
}

void Database()
{
    //-----------------------------------------------------------------------------------//
    // создание всех рабочих файлов для последующего открытия через rb+
    FILE *file;
    file = fopen("OBSERVATORIES.bin","ab+"); fclose(file);
    file = fopen("TELESCOPES.bin","ab+"); fclose(file);
    file = fopen("index.bin","ab+"); fclose(file);
    file = fopen("free_OBS.bin","ab+"); fclose(file);
    file = fopen("free_TEL.bin","ab+"); fclose(file);
    //-----------------------------------------------------------------------------------//
    char str[64] = "";
    printf("~~~ Database terminal ~~~\n");
    printf("Use \"help\" to see the list of commands.\n\n");
    while(1)
    {
        printf(" >> ");
        scanf("%s", str);
        if(!strcmp(str, "close"))
        {
            break;
        }
        else if(!strcmp(str, "insert-m"))
        {
            printf(" >> ")
        }
        else
        {
            printf(" >> Unknown command. See \"help\".\n");
        }
    }
}

int main()
{
    Database();
    /*
    FILE *file;
    file = fopen("OBSERVATORIES.bin","ab+"); fclose(file);
    file = fopen("TELESCOPES.bin","ab+"); fclose(file);
    file = fopen("index.bin","ab+"); fclose(file);
    file = fopen("free_OBS.bin","ab+"); fclose(file);
    file = fopen("free_TEL.bin","ab+"); fclose(file);

    struct observatory obs0 = {100, "Obs1", 10,10, 10};
    struct observatory obs1 = {100, "Obs2", 10,10, 10.2};
    struct observatory obs2 = {100, "Obs3", 10,10, 10};
    struct observatory obs3 = {100, "Obs4", 10,10, 10};
    struct observatory obs4 = {100, "Obs5", 10,10, 10};
    struct observatory obs5 = {100, "Obs6", 10,10, 10};
    insert_m(&obs0);
    insert_m(&obs1);
    insert_m(&obs2);
    insert_m(&obs3);
    insert_m(&obs4);
    insert_m(&obs5);
    insert_m(&obs0);
    struct telescope tel={100500, "tel", (float)2.222222,(float)2.222222,100500};
    struct telescope tel2={100500, "tel2", (float)2000,(float)2,100500};
    insert_s(0, &tel);
    insert_s(2, &tel);
    insert_s(2, &tel);
    insert_s(2, &tel);
    insert_s(2, &tel2);
    insert_s(2, &tel);
    insert_s(6, &tel);


    print_observatories();
    printf("\n\n\n");
    print_telescopes(0);
    printf("\n");
    print_telescopes(2);
    printf("\n");
    print_telescopes(1);
    printf("\n");
    print_telescopes(100500);

    insert_s(20, &tel);
    print_telescopes(20);
    printf("\n");*/

    //Database();
    /*
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

    struct telescope tel={158, "tel1", 2,2,2};
    printf("0: %i(1)\n", insert_s(0, &tel));
    printf("1: %i(1)\n", insert_s(2, &tel));
    printf("2: %i(1)\n", insert_s(2, &tel));
    printf("3: %i(1)\n", insert_s(2, &tel));
    printf("4: %i(1)\n", insert_s(2, &tel));
    printf("5: %i(1)\n", insert_s(2, &tel));
    printf("6: %i(0)\n", insert_s(6, &tel));
    printf("100500: %i(0)\n\n", insert_s(100500, &tel));
    */
    system("pause");
    return 0;
}

