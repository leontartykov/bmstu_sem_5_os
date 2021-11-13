#ifndef _INTERPOLATION_H_
#define _INTERPOLATION_H_

#include "../inc/struct.h"

double newton_interpolate(double *x, double *y, double *matrix, int table_size, int degree, double arg, int index_low, int index_high);
void get_nodes(double *x, int degree, int table_size, double arg, int *index_low, int *index_high);
double create_ermite_polynome(table_t *table, double *matrix_ermite, int degree, double arg, int index_low_ermite, int index_high_ermite,
                            double shift);

void insert_sort_array(double **array,  double **array_1, int size_array);

#endif