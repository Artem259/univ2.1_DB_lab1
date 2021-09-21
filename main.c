#include "database.h"

int main()
{
    FILE *file;
    file = fopen("files/OBSERVATORIES.bin","ab+"); fclose(file);
    file = fopen("files/TELESCOPES.bin","ab+"); fclose(file);
    file = fopen("files/index.bin","ab+"); fclose(file);
    file = fopen("files/free_OBS.bin","ab+"); fclose(file);
    file = fopen("files/free_TEL.bin","ab+"); fclose(file);

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

    printf("\n");
    print_m();
    printf("\n");
    print_s(2);

    reorganise_database();

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

