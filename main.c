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
    // получение и возврат нужной записи из OBSERVATORIES.bin (output_struct), а также ее индекса в файле (output_index)
    FILE *obs_file = fopen("OBSERVATORIES.bin","rb+");
    fseek(obs_file, (long)(curr_struct.index*sizeof(struct observatory)), SEEK_SET);
    fread(output_struct, sizeof(struct observatory), 1, obs_file);
    fclose(obs_file);
    *output_index=curr_struct.index;
    //-----------------------------------------------------------------------------------//
    if((*output_struct).is_removed) return 0; // запись была удалена
    //-----------------------------------------------------------------------------------//
    return 1;
}
int get_s(size_t obs_id, size_t tel_id, size_t *output_index, struct telescope *output_struct)
{
    size_t obs_index;
    struct observatory obs_struct;
    if(!get_m(obs_id, &obs_index, &obs_struct)) // нет обсерватории - нет телескопа
    {
        return -1;
    }
    //-----------------------------------------------------------------------------------//
    // поиск телескопа по tel_id
    FILE *tel_file = fopen("TELESCOPES.bin","rb+");
    struct telescope curr_tel;
    size_t curr_tel_index = obs_struct.telescope_index;
    for(size_t i=0 ; i<obs_struct.telescopes ; i++)
    {
        fseek(tel_file, (long)(curr_tel_index*sizeof(struct telescope)), SEEK_SET);
        fread(&curr_tel, sizeof(struct telescope), 1, tel_file);
        if(curr_tel.id == tel_id) // телескоп найден
        {
            *output_index = curr_tel_index;
            *output_struct = curr_tel;
            fclose(tel_file);
            return 1;
        }
        curr_tel_index = curr_tel.next_telescope;
    }
    //-----------------------------------------------------------------------------------//
    // телескоп не найден
    fclose(tel_file);
    return 0;
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
    struct index_structure curr_index = {(*input).id, index};
    fseek(index_file, 0, SEEK_END);
    fwrite(&curr_index, sizeof(struct index_structure), 1, index_file);
    //-----------------------------------------------------------------------------------//
    fclose(obs_file);
    fclose(index_file);
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
_Bool del_m(size_t id)
{
    size_t obs_index;
    struct observatory obs_struct;
    //-----------------------------------------------------------------------------------//
    // проверка на существование обсерватории
    if(!get_m(id, &obs_index, &obs_struct))
    {
        return 0; // нет обсерватории - нечего удалять
    }
    //-----------------------------------------------------------------------------------//
    // удаление телескопов
    FILE *tel_file = fopen("TELESCOPES.bin","rb+");
    FILE *free_tel_file = fopen("free_TEL.bin","ab+");
    struct telescope curr_tel;
    size_t tel_index = obs_struct.telescope_index;
    for(size_t i=0 ; i<obs_struct.telescopes ; i++)
    {
        fwrite(&tel_index, sizeof(tel_index), 1, free_tel_file); // запись индексов удаляемых телескопов в free_TEL.bin
        fseek(tel_file, (long)(tel_index*sizeof(struct telescope)), SEEK_SET);
        fread(&curr_tel, sizeof(struct telescope), 1, tel_file);
        tel_index = curr_tel.next_telescope;
    }
    fclose(tel_file);
    fclose(free_tel_file);
    //-----------------------------------------------------------------------------------//
    // перезапись в OBSERVATORIES.bin с меткой удаления
    obs_struct.is_removed = 1;
    FILE *obs_file = fopen("OBSERVATORIES.bin","rb+");
    fseek(obs_file, (long)(obs_index*sizeof(struct observatory)), SEEK_SET);
    fwrite(&obs_struct, sizeof(struct observatory), 1, obs_file);
    fclose(obs_file);
    //-----------------------------------------------------------------------------------//
    // запись индекса удаляемой обсерватории в free_OBS.bin
    FILE *free_obs_file = fopen("free_OBS.bin","ab+");
    fwrite(&obs_index, sizeof(obs_index), 1, free_obs_file);
    fclose(free_obs_file);
    //-----------------------------------------------------------------------------------//
    return 1;
}
int del_s(size_t obs_id, size_t tel_id)
{
    size_t obs_index;
    struct observatory obs_struct;
    //-----------------------------------------------------------------------------------//
    // проверка на существование обсерватории
    if(!get_m(obs_id, &obs_index, &obs_struct))
    {
        return -1; // нет обсерватории - нет телескопа для удаления
    }
    //-----------------------------------------------------------------------------------//
    // поиск телескопа для удаления по tel_id
    FILE *tel_file = fopen("TELESCOPES.bin","rb+");
    struct telescope curr_tel;
    size_t tel_index = obs_struct.telescope_index;
    size_t prev_tel_index;
    for(size_t i=0 ; i<obs_struct.telescopes ; i++)
    {
        fseek(tel_file, (long)(tel_index*sizeof(struct telescope)), SEEK_SET);
        fread(&curr_tel, sizeof(struct telescope), 1, tel_file);
        if(curr_tel.id == tel_id) // телескоп найден
        {
            if(i == 0) // первый телескоп - надо изменять telescope_index в OBSERVATORIES.bin
            {
                obs_struct.telescope_index = curr_tel.next_telescope; // смена индекса первого телескопа в обсерватории
            }
            else // не первый - надо изменять next_telescope предыдущего телескопа
            {
                struct telescope prev_tel;
                //-----------------------------------------------------------------------------------//
                // чтение, изменение и перезапись данных предыдущего телескопа
                FILE *prev_tel_file = fopen("TELESCOPES.bin","rb+");
                fseek(prev_tel_file, (long)(prev_tel_index*sizeof(struct telescope)), SEEK_SET);
                fread(&prev_tel, sizeof(struct telescope), 1, prev_tel_file); // чтение данных предыдущего телескопа
                prev_tel.next_telescope = curr_tel.next_telescope; // смена next_telescope предыдущего телескопа
                fseek(prev_tel_file, (long)(-sizeof(struct telescope)), SEEK_CUR);
                fwrite(&prev_tel, sizeof(struct telescope), 1, prev_tel_file);
                fclose(prev_tel_file);
                //-----------------------------------------------------------------------------------//
            }
            //-----------------------------------------------------------------------------------//
            // изменение obs_struct OBSERVATORIES.bin
            obs_struct.telescopes--; // в обсерватории становится на 1 телескоп меньше
            FILE *obs_file = fopen("OBSERVATORIES.bin","rb+");
            fseek(obs_file, (long)(obs_index*sizeof(struct observatory)), SEEK_SET);
            fwrite(&obs_struct, sizeof(struct observatory), 1, obs_file); //
            fclose(obs_file);
            //-----------------------------------------------------------------------------------//
            // запись индекса удаляемого телескопа в free_TEL.bin
            FILE *free_tel_file = fopen("free_TEL.bin","ab+");
            fwrite(&tel_index, sizeof(tel_index), 1, free_tel_file);
            fclose(free_tel_file);
            //-----------------------------------------------------------------------------------//
            fclose(tel_file);
            return 1;
        }
        prev_tel_index = tel_index;
        tel_index = curr_tel.next_telescope;
    }
    //-----------------------------------------------------------------------------------//
    // телескоп не найден
    fclose(tel_file);
    return 0;
}
_Bool update_m(size_t id, struct observatory *input)
{
    size_t obs_index;
    struct observatory obs_struct;
    //-----------------------------------------------------------------------------------//
    // проверка на существование обсерватории
    if(!get_m(id, &obs_index, &obs_struct))
    {
        return 0; // нет обсерватории - нечего редактировать
    }
    //-----------------------------------------------------------------------------------//
    // копирование служебных(неизменяемых) полей в новую структуру
    (*input).id = obs_struct.id;
    (*input).telescopes = obs_struct.telescopes;
    (*input).telescope_index = obs_struct.telescope_index;
    (*input).is_removed = 0;
    //-----------------------------------------------------------------------------------//
    // запись обновленных данных в OBSERVATORIES.bin
    FILE *obs_file = fopen("OBSERVATORIES.bin","rb+");
    fseek(obs_file, (long)(obs_index*sizeof(struct observatory)), SEEK_SET);
    fwrite(input, sizeof(struct observatory), 1, obs_file);
    fclose(obs_file);
    //-----------------------------------------------------------------------------------//
    return 1;
}
int update_s(size_t obs_id, size_t tel_id, struct telescope *input)
{
    size_t tel_index;
    struct telescope tel_struct;
    //-----------------------------------------------------------------------------------//
    // проверка на существование обсерватории и телескопа в ней
    int status = get_s(obs_id, tel_id, &tel_index, &tel_struct);
    if(status == -1) return -1; // нет указанной обсерватории
    else if(status == 0) return 0; // в обсерватории нет указанного телескопа
    //-----------------------------------------------------------------------------------//
    // копирование служебных(неизменяемых) полей в новую структуру
    (*input).id = tel_struct.id;
    (*input).next_telescope = tel_struct.next_telescope;
    //-----------------------------------------------------------------------------------//
    // запись обновленных данных в TELESCOPES.bin
    FILE *tel_file = fopen("TELESCOPES.bin","rb+");
    fseek(tel_file, (long)(tel_index*sizeof(struct telescope)), SEEK_SET);
    fwrite(input, sizeof(struct telescope), 1, tel_file);
    fclose(tel_file);
    //-----------------------------------------------------------------------------------//
    return 1;
}
void reorganise_database()
{
    //-----------------------------------------------------------------------------------//
    // открытие всех старых и новых файлов
    FILE *old_obs_file = fopen("OBSERVATORIES.bin","rb+");
    FILE *old_tel_file = fopen("TELESCOPES.bin","rb+");
    FILE *old_index_file = fopen("index.bin","rb+");
    FILE *new_obs_file = fopen("TEMP_OBSERVATORIES.bin","ab+");
    FILE *new_tel_file = fopen("TEMP_TELESCOPES.bin","ab+");
    FILE *new_index_file = fopen("TEMP_index.bin","ab+");
    //-----------------------------------------------------------------------------------//
    size_t obs_pos = 0, tel_pos = 0;
    size_t curr_tel_index, old_next_tel;
    _Bool first_tel;
    struct observatory curr_obs;
    struct telescope curr_tel;
    struct index_structure curr_index;
    while(fread(&curr_index, sizeof(struct index_structure), 1, old_index_file))
    {
        //-----------------------------------------------------------------------------------//
        // чтение обсерватории из OBSERVATORIES.bin
        fseek(old_obs_file, (long)(curr_index.index*sizeof(struct observatory)), SEEK_SET);
        fread(&curr_obs, sizeof(struct observatory), 1, old_obs_file);
        if(curr_obs.is_removed) continue; // обсерватория "логически" удалена - дальнейшая запись пропускается
        //-----------------------------------------------------------------------------------//
        // чтение и перезапись всех телескопов текущей обсерватории
        if(curr_obs.telescopes > 0)
        {
            first_tel = 1;
            curr_tel_index = curr_obs.telescope_index;
            for(size_t i=0 ; i<curr_obs.telescopes ; i++)
            {
                fseek(old_tel_file, (long)(curr_tel_index*sizeof(struct telescope)), SEEK_SET);
                fread(&curr_tel, sizeof(struct telescope), 1, old_tel_file);
                //-----------------------------------------------------------------------------------//
                // изменение полей next_telescope
                old_next_tel = curr_tel.next_telescope; // сохранение фактического индекса следующего телескопа перед перезаписью
                if(i < curr_obs.telescopes-1) curr_tel.next_telescope = tel_pos+1; // если телескоп не последний - изменяется на следующий
                else curr_tel.next_telescope = 0; // если телескоп последний - изменяется на 0 (не использовать!!!)
                //-----------------------------------------------------------------------------------//
                fwrite(&curr_tel, sizeof(struct telescope), 1, new_tel_file);
                if(first_tel) // телескоп - первый
                {
                    first_tel = 0;
                    curr_obs.telescope_index = tel_pos; // изменение индекса первого телескопа в обсерватории
                }
                tel_pos++;
                curr_tel_index = old_next_tel;
            }
        }
        //-----------------------------------------------------------------------------------//
        // запись структур обсерватории и индекса в новые файлы
        curr_index.index = obs_pos; // изменение индекса обсерватории
        fwrite(&curr_index, sizeof(struct index_structure), 1, new_index_file);
        fwrite(&curr_obs, sizeof(struct observatory), 1, new_obs_file);
        //-----------------------------------------------------------------------------------//
        obs_pos++;
    }
    //-----------------------------------------------------------------------------------//
    // закрытие всех открытых файлов, удаление старых
    fclose(old_obs_file);
    fclose(old_tel_file);
    fclose(old_index_file);
    fclose(new_obs_file);
    fclose(new_tel_file);
    fclose(new_index_file);
    remove("OBSERVATORIES.bin");
    remove("TELESCOPES.bin");
    remove("index.bin");
    rename("TEMP_OBSERVATORIES.bin", "OBSERVATORIES.bin");
    rename("TEMP_TELESCOPES.bin", "TELESCOPES.bin");
    rename("TEMP_index.bin", "index.bin");
    //-----------------------------------------------------------------------------------//
    // обнуление файлов свободных индексов (после реорганизации их нет)
    remove("free_OBS.bin");
    remove("free_TEL.bin");
    FILE *file;
    file = fopen("free_OBS.bin","ab+"); fclose(file);
    file = fopen("free_TEL.bin","ab+"); fclose(file);
    //-----------------------------------------------------------------------------------//
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
        fseek(obs_file, (long)(curr_index.index*sizeof(struct observatory)), SEEK_SET);
        fread(&curr_obs, sizeof(struct observatory), 1, obs_file);
        if(curr_obs.is_removed) continue;
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
        printf("Observatory %zu has no telescopes.\n", id); // в обсерватории нет телескопов
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
            printf(" >> ");
        }
        else
        {
            printf(" >> Unknown command. See \"help\".\n");
        }
    }
}

int main()
{
    FILE *file;
    file = fopen("OBSERVATORIES.bin","ab+"); fclose(file);
    file = fopen("TELESCOPES.bin","ab+"); fclose(file);
    file = fopen("index.bin","ab+"); fclose(file);
    file = fopen("free_OBS.bin","ab+"); fclose(file);
    file = fopen("free_TEL.bin","ab+"); fclose(file);

    struct observatory obs = {100, "Obs1", (float)1000.001,(float)1000.001, (float)1000.001};
    struct telescope tel0={100500, "tel0", (float)25.222222,(float)25.222222,100500};
    struct telescope tel1={100500, "tel1", (float)25.222222,(float)25.222222,100500};
    struct telescope tel2={100500, "tel2", (float)25.222222,(float)25.222222,100500};
    struct telescope tel3={100500, "tel3", (float)25.222222,(float)25.222222,100500};


    insert_m(&obs);
    insert_m(&obs);
    insert_m(&obs);
    insert_m(&obs);
    insert_m(&obs);

    insert_s(1, &tel0);
    insert_s(1, &tel0);
    insert_s(1, &tel0);
    insert_s(1, &tel0);
    insert_s(1, &tel0);

    insert_s(2, &tel0);
    insert_s(2, &tel1);
    insert_s(2, &tel2);
    insert_s(2, &tel3);

    insert_s(3, &tel0);
    insert_s(3, &tel0);

    reorganise_database();
    print_observatories();
    printf("-------------------------------------------------------------------------------------------------");
    print_telescopes(0);
    print_telescopes(1);
    print_telescopes(2);
    print_telescopes(3);
    print_telescopes(4);


    /*
    printf("%i\n", del_s(100500,5));
    printf("%i\n", del_s(1,5));

    printf("%i\n", del_s(2,1));
    print_telescopes(2);
    printf("%i\n", del_s(2,0));
    print_telescopes(2);
    printf("%i\n", del_s(2,1));
    print_telescopes(2);*/


    /*insert_m(&obs);
    insert_m(&obs);
    insert_s(1, &tel);
    insert_s(1, &tel);
    insert_s(1, &tel);
    insert_s(1, &tel);
    insert_s(2, &tel);
    insert_s(2, &tel);
    printf("%i\n",del_m(1));
    print_observatories();
    print_telescopes(0);
    print_telescopes(1);
    print_telescopes(2);*/

    /*Database();
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

