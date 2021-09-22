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

    size_t telescope_index; // индекс первого телескопа
    _Bool is_removed; // удален?
};
struct index_structure
{
    size_t id; // ключ
    size_t index; // позиция в файле
};
struct telescope
{
    size_t id; // KEY
    char name[40];
    float diameter; // (>0)
    float focal_length; // (>0)

    size_t next_telescope;
};

#endif
