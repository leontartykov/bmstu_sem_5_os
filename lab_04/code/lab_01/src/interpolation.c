#include <stdio.h>
#include <math.h>
#include "../inc/interpolation.h"
#include "../inc/struct.h"
#include "../inc/defines.h"

int find_index_near(double *x, int table_size, double arg);
void fill_newton_splitted_difference(double *matrix, int index_low, int index_high, 
                              double step, int degree);

void fill_ermite_split_difference(double *x, double *y, double *dy, double *matrix_ermite, int index_low_ermite, int index_high_ermite,
                                    double shift);
void swap_elem(double *first, double *second);

double newton_interpolate(double *x, double *y, double *matrix, int table_size, int degree, double arg, int index_low, int index_high)
{
    double result = 0, temp = 1;

    for (int i = index_low, j = 0; i <= index_high; i++, j++)
        matrix[j] = y[i];

    fill_newton_splitted_difference(matrix, index_low, index_high, x[1] - x[0], degree);

    for (int i = 0, k = 0, p = degree + 1; i <= degree; i++, k += p, p--)
    {
        result += matrix[k] * temp;
        temp *= (arg - x[index_low + i]);
    }

    return result;
}

void get_nodes(double *x, int degree, int table_size, double arg, int *index_low,
               int *index_high)
{
    //найти ближайшую точку
    int index_near = 0;
    index_near = find_index_near(x, table_size, arg);

    int need_space = degree / 2;

    if (need_space + index_near + 1 > table_size) //если не хватает снизу узлов
    {
        *index_low = table_size - 1 - degree;
        *index_high = table_size - 1;
    }
    else if (index_near < need_space) //если не хватает узлов сверху
    {
        *index_low = 0;
        *index_high = *index_low + degree;
    }
    else
    {
        *index_low = index_near - need_space;
        *index_high = *index_low + degree;
    }      
}

int find_index_near(double *x, int table_size, double arg)
{
    int index_near = 0;
    double min_diff = fabs(x[0] - arg);

    for (int i = 1; i < table_size; i++)
    {
        if (fabs(x[i] - arg) < min_diff && fabs(fabs(x[i] - arg) - min_diff) >= EPS)
        {
            index_near = i;
            min_diff = fabs(x[i] - arg);
        }
    }

    return index_near;
}

void fill_newton_splitted_difference(double *matrix, int index_low, int index_high, 
                              double step, int degree)
{
    int k = 1, koeff = 1, p = 0, shift = 1;
    for (int i = index_high - index_low; i > 0; i--)
    {
        for (int j = 0; j < i; j++)
        {
            matrix[degree + p + 1] = (matrix[shift] - matrix[shift - 1]) / (step * koeff);
            shift++, p++;
        }
        koeff++, k++, shift++;
    }
}

double create_ermite_polynome(table_t *table, double *matrix_ermite, int degree, double arg, int index_low_ermite, int index_high_ermite,
                            double shift)
{
    fill_ermite_split_difference((*table).x, (*table).y, (*table).dy, matrix_ermite, index_low_ermite, index_high_ermite, shift);
    double result = 0, temp = 1;
    for (int i = 0, k = 0, p = (index_high_ermite - index_low_ermite + 1)  * 2 + 1, t = 0;
             i < (index_high_ermite - index_low_ermite + 1)  * 2; i++, k += p - 1, p--)
    {
        result += matrix_ermite[k] * temp;
        temp *= (arg - (*table).x[index_low_ermite + t]);
        if (i % 2 != 0)
            t++;    
    }
    return result;
}

void fill_ermite_split_difference(double *x, double *y, double *dy, double *matrix_ermite, int index_low_ermite, int index_high_ermite,
                                    double shift)
{
    int step = (index_high_ermite - index_low_ermite + 1)  * 2, temp_step = step;
    //заполняем повторяющиеся столбцы
    for (int i = 0, j = 0; i <= step; i += 2, j++)
    {
        matrix_ermite[i] = y[index_low_ermite + j];
        matrix_ermite[i + 1] = y[index_low_ermite + j];
    }
    temp_step--;

    //заполняем колонку с производными
    for (int i = step, p = 0, t = 0; i > 1; i--, p++)
    {
        if (p % 2 == 0)
        {
            matrix_ermite[step + p] = dy[t + index_low_ermite];
            t++;
        }     
        else
            matrix_ermite[step + p] = (matrix_ermite[p + 1] - matrix_ermite[p]) / (shift);          
    }
    step += temp_step - 1, temp_step--;

    int k = 1, koeff = 1, p = 0, new_shift = 0;
    for (int i = temp_step, step_2 = (index_high_ermite - index_low_ermite + 1) * 2, t = 1; i > 0; i--, step_2++)
    {
        for (int j = 0; j < i; j++)
        {
            matrix_ermite[step + p + 1] = (matrix_ermite[step_2 + new_shift + 1] - matrix_ermite[step_2 + new_shift]) / (x[index_low_ermite + t] - x[index_low_ermite]);
            new_shift++, p++;
        }
        if (i % 2 != 0)
            t++;
        koeff++, k++;
    }
}

void insert_sort_array(double **array,  double **array_1, int size_array)
{
    double item = 0, item_1 = 0;
    for (int i = 1, j = 0; i < size_array; i++)
    {
        item = (*array)[i], item_1 = (*array_1)[i];
        j = i - 1;
        while (j >= 0 && (*array)[j] > item)
        {
            (*array)[j + 1] = (*array)[j];
            (*array_1)[j + 1] = (*array_1)[j];
            j -= 1;
        }
        (*array)[j + 1] = item;
        (*array_1)[j + 1] = item_1;
    }
}
