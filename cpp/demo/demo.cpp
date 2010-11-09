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

#include <iostream>
#include <string>
#include <vector>
#include "csv.hpp"

using namespace std;

int main(int argc, char** argv)
{
    CSV processor;

    if (argc != 3)
    {
        cout << "usage: " << argv[0] << " old_delimiter new_delimiter\n";
    }
    else
    {
        vector<string> row;
        while (1)
        {
            processor.readRow(cin, row, argv[1][0]);
            if (row.empty())
            {
                break;
            }
            //
            for (unsigned int i = 0; i < row.size(); i++)
            {
                cout << i << "\t" << row[i] << endl;
            }

            processor.writeRow(cout, row, argv[2][0]);
        }
    }
    return 0;
}
