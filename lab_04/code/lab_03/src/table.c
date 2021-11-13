#include <stdio.h>
#include <stdlib.h>

#include "../inc/table.h"
#include "../inc/struct.h"
#include "../inc/defines.h"

void read_amount_nodes(int *count);
int allocate_table(table_t **table);
void create_table(table_t **table);
void free_table(table_t **table);
void free_table_x(int **table_x);
void free_table_y(int **table_y);

table_t *init_table(void)
{
    table_t *table = NULL;
    table = malloc(sizeof(table));
    table->table_x = NULL;
    table->table_y = NULL;
    table->count = 0;

    return table;
}

int read_table(table_t **table)
{
    int error = ERR_OK;
    table_t *temp_table;
    temp_table = init_table();

    read_amount_nodes(&temp_table->count);
    error = allocate_table(&temp_table);

    if (error == ERR_OK)
        create_table(&temp_table);

    if (error == ERR_OK)
        *table = temp_table;
    return error;
}

void read_amount_nodes(int *count)
{
    printf("Введите количество узлов: ");
    while (scanf("%d", count) != 1)
    {
        scanf("%*[^\n]");
        printf("Некорректный ввод. Введите еще раз: ");
    }
}

void read_input_x(double *input_x)
{
    printf("Введите аргумент полинома: ");
    while (scanf("%lf", input_x) != 1)
    {
        scanf("%*[^\n]");
        printf("Некорректный ввод. Введите еще раз: ");
    }
}

int allocate_table(table_t **table)
{
    int error = ERR_OK;
    (*table)->table_x = malloc((*table)->count * sizeof(int));

    if (!(*table)->table_x)
    {
        free_table_x(&(*table)->table_x);
        error = ERR_ALLOCATE;
    }

    if (error == ERR_OK)
    {
        (*table)->table_y = malloc((*table)->count * sizeof(int));
        if (!(*table)->table_y)
        {
            free_table_y(&(*table)->table_y);
            error = ERR_ALLOCATE;
        }
    }
    return error;
}

void create_table(table_t **table)
{
    for (int i = 0; i < (*table)->count; i++)
    {
        (*table)->table_x[i] = i;
        (*table)->table_y[i] = i * i;
    }
}

void print_table(table_t *table)
{
    for (int i = 0; i < (*table).count; i++)
    {
        printf("%d %d\n", (*table).table_x[i], (*table).table_y[i]);
    }
}

void free_table(table_t **table)
{
    if (*table)
    {
        free_table_x(&(*table)->table_x);
        free_table_y(&(*table)->table_y);
        free(*table);
    }
}

void free_table_x(int **table_x)
{
    if (*table_x)
        free(*table_x);
}

void free_table_y(int **table_y)
{
    if (*table_y)
        free(*table_y);
}