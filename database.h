#include <stdio.h>
#include <io.h>
#include "structures.h"

#ifndef DATABASE_H
#define DATABASE_H

_Bool get_free_index(char file_name[], size_t *result);
_Bool get_index_index(size_t id, size_t *output_index_index, size_t *output_obs_index);
_Bool get_m(size_t id, size_t *output_obs_index, size_t *output_index_index, struct observatory *output_struct);
int get_s(size_t obs_id, size_t tel_id, size_t *output_index, struct telescope *output_struct);
void insert_m(struct observatory *input);
_Bool insert_s(size_t id, struct telescope *input);
_Bool del_m(size_t id, size_t *tel_count);
int del_s(size_t obs_id, size_t tel_id);
_Bool update_m(size_t id, struct observatory *input);
int update_s(size_t obs_id, size_t tel_id, struct telescope *input);
void reorganise_database(size_t *output_free_obs, size_t *output_free_tel);
void show_m();
void show_s(size_t id);

_Bool get_size_t(size_t *output);
_Bool get_float(float *output);
void UI_help();
void UI_get_m();
void UI_get_s();
void UI_del_m();
void UI_del_s();
void UI_show_m();
void UI_show_s();
void UI_insert_m();
void UI_insert_s();
void UI_update_m();
void UI_update_s();
void UI_reorganise_database();

void Database();

#endif