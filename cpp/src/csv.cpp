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

#include "csv.hpp"

using namespace std;

void CSV::readRow(std::istream &input, vector<string> &row, char delim = ',')
{
    bool in_quote = false;
    bool has_escape_quote = false;
    row.clear();  
    string line;
    string cur = "";

    while(getline(input, line))
    {
        cur = "";
        for (unsigned int i = 0; i < line.size(); i++)
        {
            if (line[i] == '"')
            {
                if (in_quote)
                {
                    if (!has_escape_quote)
                    {
                        if (line[i + 1] == '"' && i + 2 < line.size() )
                        {
                            has_escape_quote = true;
                        }
                        else
                        {
                            in_quote = false;
                        }
                    }
                    else
                    {
                        cur.push_back('"');
                        has_escape_quote = false;
                    }
                }
                else
                {
                    in_quote = true;
                }
            }
            else if(line[i] == delim)
            {
                if(in_quote)
                {
                    cur.push_back(line[i]);
                }
                else
                {
                    row.push_back(cur);
                    cur = "";
                }
            }
            else
            {
                cur.push_back(line[i]);
            }
        }
        
        if (!in_quote)
        {
            if (cur != "")
            {
                if (cur[cur.size() - 1] == '\n')
                {
                    cur.erase(cur.size() - 1, 1);
                }
                row.push_back(cur);
            }
            break;
        }
    }
    return; 
}

void CSV::writeRow(std::ostream &output, vector<string> &row, char delim = ',')
{
    bool has_delim = false;
    bool has_quote = false;
    char new_line = '\n';
    string output_line = "";
    for (unsigned int i = 0; i < row.size(); i++)
    {
        has_delim = false;
        has_quote = false;
        for (unsigned int j = 0; j < row[i].size(); j++)
        {
            if (row[i][j] == delim)
            {
                has_delim = 1;
            }
            else if (row[i][j] == '"')
            {
                output_line.push_back('"');
                has_quote = 1;
            }

            output_line.push_back(row[i][j]);
                
        }

        if (has_quote || has_delim)
        {
            output_line = '"' + output_line + '"';
        }

        if (i < row.size() - 1)
        {
            output_line.push_back(delim);
        }
        
        output.write(output_line.c_str(), output_line.size());
    }

    output.put(new_line);
}

