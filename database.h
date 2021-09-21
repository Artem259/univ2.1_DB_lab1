#include <stdio.h>
#include <io.h>
#include "structures.h"

#ifndef DATABASE_H
#define DATABASE_H

_Bool get_free_index(char file_name[], size_t *result);
_Bool get_m(size_t id, size_t *output_index, struct observatory *output_struct);
int get_s(size_t obs_id, size_t tel_id, size_t *output_index, struct telescope *output_struct);
void insert_m(struct observatory *input);
_Bool insert_s(size_t id, struct telescope *input);
_Bool del_m(size_t id);
int del_s(size_t obs_id, size_t tel_id);
_Bool update_m(size_t id, struct observatory *input);
int update_s(size_t obs_id, size_t tel_id, struct telescope *input);
void reorganise_database();
void print_m();
void print_s(size_t id);
void Database();

#endif