#include <stdio.h>
#include <stdlib.h>
#include "csv.h"

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        printf ("usage: %s old_delimiter new_delimiter\n", argv[0]);
    }
    else
    {
        Row *cur = NULL;
        while (1)
        {
            cur = csv_get_row(stdin, argv[1][0], 10000);
            if (cur == NULL)
            {
                break;
            }
            csv_write_row(stdout, cur, argv[2][0]);
            csv_free_row(cur);
        }
    }
    return 0;
}
