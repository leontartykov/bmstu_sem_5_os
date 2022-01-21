#include <stdio.h>
#include <stdlib.h>
#include "../inc/read.h"
#include "../inc/defines.h"
#include "../inc/struct.h"

void init_table(table_t **table, int table_size);

void read_degree(int *degree)
{
    printf("Введите степень полинома (от 1 до 4): ");
    while (fscanf(stdin, "%d", degree) != 1 || *degree <= -1 || *degree >= 5)
    {
        fflush(stdin);
        printf("Некорректное значение!\n"
                "Введите степень полинома (от 1 до 4): ");
    }
}

int read_table(table_t **table, char *table_name, int *table_size)
{
    int code_error = ERR_OK;
    FILE *file = fopen(table_name, "r");
    if (file != NULL)
    {
        if (fscanf(file, "%d\n", table_size) != 1 || *table_size <= 0)
        {
            printf("Некорректный размер таблицы!\n");
        }
        else
        {
            *table = NULL;
            *table = malloc(sizeof(table_t));
            init_table(table, *table_size);
            for (int i = 0; code_error == ERR_OK && feof(file) == 0; i++)
            {
                if (fscanf(file, "%lf%lf%lf\n", &(*table)->x[i], &(*table)->y[i], &(*table)->dy[i]) != 3)
                    code_error = ERR_NOT_READ_VALUES;
            }
        }
    }
    else
        code_error = ERR_NOT_CORRECT_FILE;

    return code_error;
}

void init_table(table_t **table, int table_size)
{
    (*table)->x = NULL, (*table)->y = NULL, (*table)->dy = NULL;
    (*table)->x = malloc(table_size * sizeof(double));
    (*table)->y = malloc(table_size * sizeof(double));
    (*table)->dy = malloc(table_size * sizeof(double));
}

void read_interpol_argument(double *arg)
{
    printf("Введите значение аргумента, для которого выполняется интерполяция: ");
    while (scanf("%lf", arg) != 1)
    {
        fflush(stdin);
        printf("Некорректное значение!\n"
                "Введите значение аргумента, для которого выполняется интерполяция: ");
    }
}