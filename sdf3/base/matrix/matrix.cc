/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   matrix.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 September 2006
 *
 *  Function        :   Sparse Matrices
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

#include "matrix.h"

#include <cmath>

/**
 * SparseMAtrix constructor
 */
SparseMatrix::SparseMatrix(const CId Size) {

    // set size
    Matrix.resize(Size);

    // create row vectors
    for (CId i = 0; i != Size; i++) {
        Matrix[i] = std::make_unique<SparseVector>();
    }
}

/**
 * SparseVector destructor
 */
SparseVector::~SparseVector() {
    while (Root->Next != nullptr) {
        removeFirstElement();
    }
    Root.reset();
}

/**
 * SparseMatrix destructor
 */
SparseMatrix::~SparseMatrix() = default;

/**
 * SparseVector::addElement
 */
void SparseVector::addElement(const CId Index, const CDouble Value) {
    auto Element = std::make_unique<SparseVectorElement>(Index, Value);
    bool ElementAdded = false;

    // find place to insert
    SparseVectorElement *i = nullptr;
    for (i = Root.get(); !ElementAdded && i->Next != nullptr; i = i->Next.get()) {
        if (i->Next->Index > Index) {
            Element->Next = std::move(i->Next);
            i->Next = std::move(Element);
            ElementAdded = true;
        }
    }

    // if not added, it goes at the end
    if (!ElementAdded) {
        i->Next = std::move(Element);
    }
}

/**
 * SparseVector::getElement
 */
SparseVectorElement *SparseVector::getElement(const CId Index) {

    for (SparseVectorElement *i = Root->Next.get(); i != nullptr; i = i->Next.get()) {
        if (i->Index == Index) {
            return i;
        }
    }
    return nullptr;
}

/**
 * SparseVector::removeFirstElement
 */
void SparseVector::removeFirstElement() {
    std::unique_ptr<SparseVectorElement> Element = std::move(Root->Next);
    Root->Next = std::move(Element->Next);
}

/**
 * SparseMatrix::computeEigenVector
 * Function to compute eigen vector
 */
std::vector<CDouble> SparseMatrix::computeEigenVector() {

    constexpr double EPSILON = 1e-10;

    // trivial case
    if (Matrix.size() == 1) {
        std::vector<CDouble> EigenVector(1, 1);
        return EigenVector;
    }

    // Gaussian elimination

    for (CId i = 0; i != Matrix.size() - 1; i++) {

        // Find row j with maximum element starting from row i
        // postpone using last row as much as possible (MG: why?)

        CDouble Maximum = Matrix[i]->getFirstElement()->Value;
        CId Index = i;

        // find pivot element
        bool NonZeroElementFound = false;
        for (CId j = i + 1; j != Matrix.size() - 1; j++) {
            SparseVectorElement *Element = Matrix[j]->getFirstElement();
            if (Element->Index == i) {
                NonZeroElementFound = true;
                if (fabs(Element->Value) > fabs(Maximum)) {
                    Maximum = Element->Value;
                    Index = j;
                }
            }
        }

        // nothing found, use the last row
        if (!NonZeroElementFound) {
            Maximum = Matrix[Matrix.size() - 1]->getFirstElement()->Value;
            Index = static_cast<CId>(Matrix.size()) - 1;
        }

        // Use row Index as pivot

        // Swap current row i with row Index that has maximum element (if necessary) and division by that
        // element
        if (Index != i) {
            // if a swap is necessary
            std::unique_ptr<SparseVector> Temp = std::move(Matrix[i]);
            Matrix[i] = std::move(Matrix[Index]);
            Matrix[Index] = std::move(Temp);
        }

        // Subtraction of rows
        SparseVectorElement *FirstElementOfRow = Matrix[i]->getFirstElement();
        // iterate through rows i+1 to size-1
        for (CId j = i + 1; j != Matrix.size(); j++) {
            // Element iterates through the element of row j
            std::unique_ptr<SparseVectorElement>& Element = Matrix[j]->getFirstElementPtr();
            // only action required if row j has a non-zero value at column i...
            if (Element->Index == i) {
                CDouble Factor = -Element->Value / Maximum;
                SparseVectorElement* PreviouslyAccessedElement = Element.get();
                // iterate through the non-zero elements of row i
                for (SparseVectorElement *ElementOfRow = FirstElementOfRow->Next.get();
                     ElementOfRow != nullptr;) {
                    std::unique_ptr<SparseVectorElement>& ElementToChange = PreviouslyAccessedElement->Next;
                    // if row j still has non-zero elements
                    if (ElementToChange != nullptr) {
                        // test if the next element is at the index we are trying to modify
                        if (ElementToChange->Index == ElementOfRow->Index) {
                            // update its value accordingly
                            ElementToChange->Value =
                                    ElementToChange->Value + Factor * ElementOfRow->Value;
                            // if the result becomes zero remove it
                            if (fabs(ElementToChange->Value) < EPSILON * fabs(Maximum) )
                            {
                                PreviouslyAccessedElement->Next = std::move(ElementToChange->Next);
                            } else {
                                // else move on
                                PreviouslyAccessedElement = ElementToChange.get();
                            }
                            ElementOfRow = ElementOfRow->Next.get();
                        } else if (ElementToChange->Index > ElementOfRow->Index) {
                            // next element in row j comes after next element in i, create new
                            // element
                            auto NewElement = std::make_unique<SparseVectorElement>(
                                    ElementOfRow->Index, Factor * ElementOfRow->Value);
                            NewElement->Next = std::move(ElementToChange);
                            auto *NewElementPtr = NewElement.get();
                            PreviouslyAccessedElement->Next = std::move(NewElement);
                            PreviouslyAccessedElement = NewElementPtr;
                            // move forward
                            ElementOfRow = ElementOfRow->Next.get();
                        } else {
                            // next element in row j comes before element to change, just move on
                            PreviouslyAccessedElement = ElementToChange.get();
                        }
                    } else {
                        // the row j has no more non zero elements, create a new element
                        auto NewElement = std::make_unique<SparseVectorElement>(ElementOfRow->Index,
                                                                   Factor * ElementOfRow->Value);
                        // update all references
                        NewElement->Next = std::move(ElementToChange);
                        auto *NewElementPtr = NewElement.get();
                        PreviouslyAccessedElement->Next = std::move(NewElement);
                        PreviouslyAccessedElement = NewElementPtr;
                        // move forward
                        ElementOfRow = ElementOfRow->Next.get();
                    }
                }
                // the first element of the row becomes always zero and is therefore removed
                Matrix[j]->removeFirstElement();
            }
        }
    }

    // Backtrack solutions

    // create result matrix
    std::vector<CDouble> EigenVector(Matrix.size(), 0);

    //
    for (CId i = static_cast<CId>(Matrix.size()) - 1; i + 1 != 0; i--) {

        CDouble Sum = 0;

        if (i < Matrix.size() - 1) {
            for (SparseVectorElement *ElementOfRow = Matrix[i]->getFirstElement()->Next.get();
                 ElementOfRow != nullptr;
                 ElementOfRow = ElementOfRow->Next.get()) {
                if (ElementOfRow->Index < Matrix.size()) {
                    Sum += ElementOfRow->Value * EigenVector[ElementOfRow->Index];
                }
            }
        }

        CDouble Result = 0.0;
        SparseVectorElement *Element = Matrix[i]->getElement(static_cast<CId>(Matrix.size()));

        if (Element != nullptr) {
            Result = (Element->Value - Sum) / Matrix[i]->getFirstElement()->Value;
        } else {
            Result = -Sum / Matrix[i]->getFirstElement()->Value;
        }

        EigenVector[i] = Result;
    }

    return EigenVector;
}
