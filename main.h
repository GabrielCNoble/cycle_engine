#ifndef MAIN_H
#define MAIN_H


#include "scenegraph_types.h"


void gmain(float delta_time);

void ginput(float delta_time);

void resize_selection_list();

void add_selection(pick_record_t *pick);

void drop_selection(pick_record_t *pick);

void copy_selection();

void clear_selection_list();

void create_option_dropdown();

void destroy_option_dropdown();

void open_add_to_world_menu();

void close_add_to_world_menu();

void open_delete_menu();

void close_delete_menu();

void draw_3d_handle(int mode);

void check_3d_handle(int mode, int x, int y);

void init_3d_handle();

void init_editor();

void ginit();

void gpause();

void gresume();

void cache_test();


#include "pew.h"


#endif
