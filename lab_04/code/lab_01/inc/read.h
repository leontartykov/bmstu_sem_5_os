#ifndef _READ_H_
#define _READ_H_

#include "../inc/struct.h" 

void read_degree(int *degree);
int read_table(table_t **table, char *table_name, int *table_size);
void read_interpol_argument(double *arg);

#endif