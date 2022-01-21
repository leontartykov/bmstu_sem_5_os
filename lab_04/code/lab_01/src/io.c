#include <stdio.h>
#include "../inc/io.h"
#include "../inc/struct.h"

void print_table(table_t *table, int table_size)
{
    printf("Исходная таблица:\n"
            "|     x     |     y     |     dy    |\n"
            "|-----------|-----------|-----------|\n");
    for (int i = 0; i < table_size; i++)
    {
        printf("|%11lf|%11lf|%11lf|\n", table->x[i], table->y[i], table->dy[i]);
    }
}