/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   mpdependencies.h
 *
 *  Author          :   Marc Geilen (m.c.w.geilen@tue.nl)
 *
 *  Date            :   May 13, 2008
 *
 *  Function        :   Max-Plus based buffer sizing. Analyse and manage storage critical
 * dependencies.
 *
 *  History         :
 *      13-05-08    :   Initial version.
 *
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

#include "extendedmatrix.h"
#include "base/exception/exception.h"

using namespace MaxPlus;

void ExtendedMatrix::put(unsigned int row,
                         unsigned int column,
                         MPTime value,
                         std::unordered_set<int> &bs) {
    this->Matrix::put(row, column, value);
    this->bufferSets[(row * this->getCols()) + column] = bs;
}

ExtendedMatrix
ExtendedMatrix::getExtendedSubMatrix(const std::list<unsigned int> &rowIndices,
                                     const std::list<unsigned int> &colIndices) const {
    auto NR = static_cast<unsigned int>(rowIndices.size());
    auto NC = static_cast<unsigned int>(colIndices.size());
    ExtendedMatrix newExtendedMatrix(NR, NC);

    auto rit = rowIndices.begin();
    for (unsigned int r = 0; r < NR; r++, rit++) {
        unsigned int ri = (*rit);
        auto cit = colIndices.begin();
        for (unsigned int c = 0; c < NC; c++, cit++) {
            unsigned int ci = (*cit);
            std::unordered_set<int> bs = this->getBufferSet(ri, ci);
            newExtendedMatrix.put(r, c, this->get(ri, ci), bs);
        }
    }
    return newExtendedMatrix;
}


std::unique_ptr<Matrix>
ExtendedMatrix::getSubMatrixPtr(const std::list<unsigned int> &rowIndices,
                                     const std::list<unsigned int> &colIndices) const {
    auto NR = static_cast<unsigned int>(rowIndices.size());
    auto NC = static_cast<unsigned int>(colIndices.size());
    auto newExtendedMatrix = std::make_unique<ExtendedMatrix>(NR, NC);

    auto rit = rowIndices.begin();
    for (unsigned int r = 0; r < NR; r++, rit++) {
        unsigned int ri = (*rit);
        auto cit = colIndices.begin();
        for (unsigned int c = 0; c < NC; c++, cit++) {
            unsigned int ci = (*cit);
            std::unordered_set<int> bs = this->getBufferSet(ri, ci);
            newExtendedMatrix->put(r, c, this->get(ri, ci), bs);
        }
    }
    return newExtendedMatrix;
}

std::unique_ptr<Matrix>
ExtendedMatrix::getSubMatrix(const std::list<unsigned int> &indices) const {
    assert(this->getRows() == this->getCols());
    return this->getSubMatrixPtr(indices, indices);
}



Matrix *ExtendedMatrix::makeMatrix(unsigned int nrows, unsigned int nCols) const {
    return new ExtendedMatrix(nrows, nCols); // NOLINT(*-owning-memory) from maxpluslib
}

std::unordered_set<int> ExtendedMatrix::getBufferSet(unsigned int row, unsigned int column) const {
    if ((row >= this->getRows()) || (column >= this->getCols())) {
        throw CException("Index out of bounds in"
                         "Matrix::get");
    }
    return this->bufferSets[(row * this->getCols()) + column];
}
