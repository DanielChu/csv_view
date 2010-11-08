/**
 *  Copyright (C) 2010  Daniel Chu, YouGo Software
 *                      email: daniel.chu@yougo.com.au
 *
 *  This file is part of csv_view.
 *
 *  csv_view is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  csv_view is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with csv_view.  If not, see <http://www.gnu.org/licenses/>.
 **/

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
