#ifndef TABLE_H
#define TABLE_H

#include "../inc/struct.h"

table_t *init_table(void);
int read_table(table_t **table);
void read_input_x(double *input_x);
void print_table(table_t *table);

#endif