#ifndef STRUCTURES_H
#define STRUCTURES_H

struct observatory
{
    size_t id; // KEY
    char name[40];
    float latitude; // широта (-90 south; 90 north)
    float longitude; // долгота (-180 west; 180 east)
    float altitude; // высота
    size_t telescopes; // количество телескопов

    size_t telescope_index;
};
struct index_structure
{
    size_t id; // ключ
    size_t index; // позиция в файле

    _Bool is_removed;
};
struct telescope
{
    size_t id; // KEY
    char name[40];
    float diameter;
    float focal_length;

    size_t next_telescope;
};

#endif
