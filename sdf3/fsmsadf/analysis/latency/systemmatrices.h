/*
*  TU Eindhoven
*  Eindhoven, The Netherlands
*
*  Name            :   regexparser_thr.h
*
*  Author          :   Hadi Alizadeh Ara (s.h.seyyed.alizadeh@tue.nl)
*
*  Date            :   July 30, 2017
*
*  Function        :   defines a regular expression parser that produces a regex
*                      that contains throughput analysis information
*
*  History         :
*      30-07-17    :   Initial version.
*
* $Id:   $
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*
* In other words, you are welcome to use, share and improve this program.
* You are forbidden to forbid anyone else to use, share and improve
* what you give them.   Happy coding!
*/


#ifndef FSMSADF_SYSTEMMATRICES_H_INCLUDED
#define FSMSADF_SYSTEMMATRICES_H_INCLUDED

#include "base/string/cstring.h"
#include "maxplus/algebra/mpmatrix.h"
#include <memory>

/*
* A class to describe a scenario
*/

class SystemMatrices
{
public:

    SystemMatrices() = default;

    //constructor
    SystemMatrices(std::unique_ptr<MaxPlus::Matrix> s, std::unique_ptr<MaxPlus::Matrix> o) : stateMatrix(std::move(s)), outputMatrix(std::move(o))
    {
    }

    //destructor
    ~SystemMatrices() = default;

    SystemMatrices(const SystemMatrices&) = delete;
    SystemMatrices& operator=(const SystemMatrices&) = delete;
    SystemMatrices(SystemMatrices&&) = delete;
    SystemMatrices& operator=(SystemMatrices&&) = delete;

    // Getters for the matrices
    [[nodiscard]] const MaxPlus::Matrix* getStateMatrix() const
    {
        return stateMatrix.get();
    }

    [[nodiscard]] const MaxPlus::Matrix* getOutputMatrix() const
    {
        return outputMatrix.get();
    }

private:
    std::unique_ptr<MaxPlus::Matrix> stateMatrix;
    std::unique_ptr<MaxPlus::Matrix> outputMatrix;
    //Matrix* inputMatrix;
    //Matrix* feedForwardMatrix;
};

/*
* A class to describe a set of scenarios
*/
class SystemsMatrices
{
public:

    //constructor
    SystemsMatrices() = default;

    //copy constructor
    SystemsMatrices(const SystemsMatrices&) = default;

    //copy assignment operator
    SystemsMatrices& operator=(const SystemsMatrices&) = default;

    //move constructor
    SystemsMatrices(SystemsMatrices&&)  noexcept = default;

    //move assignment operator
    SystemsMatrices& operator=(SystemsMatrices&&) = default;

    //destructor
    ~SystemsMatrices() = default;

    std::map<CString, std::unique_ptr<SystemMatrices>> systemsMatrices;
};

#endif