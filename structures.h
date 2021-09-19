#ifndef STRUCTURES_H
#define STRUCTURES_H

struct observatory
{
    size_t id; // KEY
    char name[64];
    float latitude; // широта (-90 south; 90 north)
    float longitude; // долгота (-180 west; 180 east)

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
    long auto_id; // KEY
    char name[64];
    float diameter;
    float focal_length;

    size_t next_telescope;
};

#endif
