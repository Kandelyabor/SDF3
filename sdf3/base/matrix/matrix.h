/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   matrix.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 September 2006
 *
 *  Function        :   (Square) Sparse Matrices (for doubles)
 *
 *  History         :
 *      29-09-06    :   Initial version.
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

#ifndef MATRIX_H_INCLUDED
#define MATRIX_H_INCLUDED

#include "base/basic_types.h"

/**
 * SparseVectorElement
 * sparse vector as a linked list of elements with index and value
 */
class SparseVectorElement {

public:
    // Constructor
    SparseVectorElement() = default;
    SparseVectorElement(const CId I, const CDouble V) : Index(I), Value(V) {};

    // Instance Variables
    CId Index{0};
    CDouble Value{0.0};
    std::unique_ptr<SparseVectorElement> Next{nullptr};
};

/**
 * SparseVector
 */
class SparseVector {

public:
    // Constructor
    SparseVector() : Root(new SparseVectorElement()) {  };

    // Destructor
    ~SparseVector();

    SparseVector(const SparseVector&) = delete;
    SparseVector& operator=(const SparseVector&) = delete;
    SparseVector(SparseVector&&) = delete;
    SparseVector& operator=(SparseVector&&) = delete;


    // Access to elements
    void addElement(CId Index, CDouble Value);
    SparseVectorElement *getElement(CId Index);
    SparseVectorElement *getFirstElement() { return Root->Next.get(); };
    std::unique_ptr<SparseVectorElement>& getFirstElementPtr() { return Root->Next; };
    void removeFirstElement();

private:
    // Instance Variables
    std::unique_ptr<SparseVectorElement> Root;
};

/**
 * SparseMatrix
 */
class SparseMatrix {

public:
    // Constructor
    explicit SparseMatrix(CId Size);

    // Destructor
    ~SparseMatrix();

    SparseMatrix(const SparseMatrix&) = delete;
    SparseMatrix& operator=(const SparseMatrix&) = delete;
    SparseMatrix(SparseMatrix&&) = delete;
    SparseMatrix& operator=(SparseMatrix&&) = delete;

    // Access to elements
    void set(const CId i, const CId j, const CDouble Value) {
        if (Matrix[i]->getElement(j) == nullptr) {
            Matrix[i]->addElement(j, Value);
        } else {
            Matrix[i]->getElement(j)->Value = Value;
        }
    };

    CDouble get(const CId i, const CId j) {
        if (Matrix[i]->getElement(j) == nullptr) {
            return 0.0;
        }
        return Matrix[i]->getElement(j)->Value;
    };

    // Functions
    std::vector<CDouble> computeEigenVector();

    void print(std::ostream &out) {
        for (CId i = 0; i != Matrix.size(); i++) {
            out << "Row " << i << ": ";
            for (SparseVectorElement *j = Matrix[i]->getFirstElement(); j != nullptr; j = j->Next.get()) {
                out << "(" << j->Index << "," << j->Value << ") ";
            }
            out << "\n";
        }
    };

private:
    // Instance Variables;
    std::vector<std::unique_ptr<SparseVector>> Matrix;
};

#endif
