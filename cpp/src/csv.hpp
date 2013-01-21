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

#ifndef CSV_HPP
#define CSV_HPP

#include <iostream>
#include <string>
#include <vector>

class CSV {
public:
    void readRow(std::istream &input, std::vector<std::string> &row, char delim);
    void writeRow(std::ostream &output, std::vector<std::string> &row, char delim);
};

#endif
