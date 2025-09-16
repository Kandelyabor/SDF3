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
 *  Function        :   Max-Plus based buffer sizing. Analyze and manage storage critical
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

#include "maxplus/algebra/mpmatrix.h"
#include <unordered_set>

class ExtendedMatrix : public MaxPlus::Matrix {
public:
    ExtendedMatrix(unsigned int nrows, unsigned int nCols) : Matrix(nrows, nCols) {
        unsigned int nEls = this->getRows() * this->getCols();
        this->bufferSets.resize(nEls);
    }

    // Creates a matrix with reserved memory for nRel expected entities
    ExtendedMatrix(unsigned int nRows, unsigned int nCols, unsigned int nRel) :
        Matrix(nRows, nCols, nRel) {
        this->bufferSets.reserve(nRel);
    }
    explicit ExtendedMatrix(unsigned int N) : Matrix(N) {}

    [[nodiscard]] ExtendedMatrix createCopyExtended() const {
        ExtendedMatrix newExtendedMatrix(*this);
        unsigned int nEls = this->getRows() * this->getCols();
        for (unsigned int pos = 0; pos < nEls; pos++) {
            newExtendedMatrix.bufferSets[pos] = this->bufferSets[pos];
        }

        return newExtendedMatrix;
    }

    void put(unsigned int row, unsigned int column, MaxPlus::MPTime value, std::unordered_set<int> &);
    [[nodiscard]] ExtendedMatrix getExtendedSubMatrix(const std::list<unsigned int> &rowIndices,
                                        const std::list<unsigned int> &colIndices) const;

    // factory methods
    [[nodiscard]] virtual Matrix *makeMatrix(unsigned int nrows, unsigned int nCols) const;
    [[nodiscard]] std::unique_ptr<Matrix> getSubMatrixPtr(const std::list<unsigned int> &rowIndices,
                                        const std::list<unsigned int> &colIndices) const override;
    [[nodiscard]] virtual std::unique_ptr<Matrix> getSubMatrix(const std::list<unsigned int> &indices) const;

    [[nodiscard]] std::unordered_set<int> getBufferSet(unsigned int row, unsigned int column) const;

private:
    std::vector<std::unordered_set<int>> bufferSets;
};
