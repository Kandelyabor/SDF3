/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   regex_thr.h
 *
 *  Author          :   Hadi Alizadeh Ara (s.h.seyyed.alizadeh@tue.nl)
 *
 *  Date            :   July 30, 2017
 *
 *  Function        :   defines a regular expression that contains throughput
 *                      analysis information
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

#include "regex_lat.h"
#include <memory>

using namespace MaxPlus;

// initialize the period
MPTime RegExLat::period = MPTime(0); // NOLINT(cert-err58-cpp)

std::unique_ptr<Vector> BlankL::computeLatencyVector() const { return nullptr; }

std::unique_ptr<Vector> PrimitiveL::computeLatencyVector() const {
    // create the following matrix. \mu is the period.
    // 0                    0                  ...                    0
    // \mu                  \mu                ...                  \mu
    // 2\mu                 2\mu               ...                 2\mu
    //  ...                 ...                ...                 ...
    // (nrOutputs-1)*\mu    (nrOutputs-1)*\mu  ...    (nrOutputs-1)*\mu
    Matrix periodMatrix(this->getOutputMatrix()->getRows(), this->getOutputMatrix()->getCols());
    for (uint i = 0; i < periodMatrix.getRows(); i++) {
        for (uint j = 0; j < periodMatrix.getCols(); j++) {
            periodMatrix.put(i, j, MPTime(i) * period);
        }
    }

    // create a vector of zeros
    Vector zeroVec(this->outputMatrix->getRows(), MPTime(0));

    // subtract the period matrix from the output matrix
    Matrix outMinusPeriod = this->getOutputMatrix()->mp_sub(periodMatrix);

    // transpose the resultant matrix
    Matrix outMinusPeriodTransposed = outMinusPeriod.transpose();

    // compute the latency vector
    Vector lat = outMinusPeriodTransposed.mp_multiply(zeroVec);

    // return the result
    return std::make_unique<Vector>(lat);
}

std::unique_ptr<Vector> SequenceL::computeLatencyVector() const {
    // compute the latency of the first expression
    auto *firstExp = dynamic_cast<RegExLat *>((this->getFirst()).get());
    auto firstVec = firstExp->computeLatencyVector();

    // compute the latency of the second expression
    auto *secondExp = dynamic_cast<RegExLat *>((this->getSecond()).get());
    auto secondVec = secondExp->computeLatencyVector();

    // if the first expression is Blank, return the latency of the second expression
    if (firstVec == nullptr) {
        return secondVec;
    }
    // if not

    //\lambda(f.s) = \lambda(f) \oplus \normalizedState(f)^T\otimes\lambda(s)
    // compute the normalized state matrix of the first expression
    auto adj = firstExp->computeNormalizedStateMatrix();

    // get it's transpose
    Matrix adjTrans = adj->transpose();

    // multiply the results by the latency of the second expression
    Vector adjTransSecondVec = adjTrans.mp_multiply(*secondVec);

    // create a zero vector
    auto lat = std::make_unique<Vector>(firstVec->getSize(), MP_MINUS_INFINITY);

    // compute the maximum latency of both expressions
    firstVec->maximum(adjTransSecondVec, *lat);

    // return the results
    return lat;
}

std::unique_ptr<Vector> ChoiceL::computeLatencyVector() const {

    // compute the latency of the first expression
    auto *leftExp = dynamic_cast<RegExLat *>((this->getLeft()).get());
    auto leftVec = leftExp->computeLatencyVector();

    // compute the latency of the second expression
    auto *rightExp = dynamic_cast<RegExLat *>((this->getRight()).get());
    auto rightVec = rightExp->computeLatencyVector();

    // create a zero vector
    auto lat = std::make_unique<Vector>(leftVec->getSize(), MP_MINUS_INFINITY);

    // compute the maximum latency of both expressions
    leftVec->maximum(*rightVec, *lat);

    return lat;
}

std::unique_ptr<Vector> KleeneIterationL::computeLatencyVector() const {

    // compute the latency of the base expression
    auto *baseExp = dynamic_cast<RegExLat *>((this->getBase()).get());
    auto baseVec = baseExp->computeLatencyVector();

    // compute the normalized state matrix of the expression
    auto adj = this->computeNormalizedStateMatrix();

    // compute its Kleene star
    auto adjStar = std::make_unique<Matrix>(adj->getRows(), adj->getCols());
    bool posCycle = adj->allPairLongestPathMatrix(MPTime(0), true, *adjStar);

    // undefined latency if the adjacency graph of this matrix contains a positive cycle
    if (posCycle) {
        throw ::CException("The latency is not defined. The period is lower than the maximum cycle "
                         "time of the application");
    }

    // compute the latency ((NormStateMatrix)*)^T\otimes \lambda(baseExpression)
    Matrix adjStarTrans = adjStar->transpose();
    Vector lat = adjStarTrans.mp_multiply(*baseVec);

    return std::make_unique<Vector>(lat);
}

std::unique_ptr<Vector> ConstantIterationL::computeLatencyVector() const {
    // compute the latency of the base expression
    auto *baseExp = dynamic_cast<RegExLat *>((this->getBase()).get());
    auto baseVec = baseExp->computeLatencyVector();

    // compute the normalized state matrix of the expression
    auto adj = this->computeNormalizedStateMatrix();
    Matrix adjTrans = adj->transpose();

    // compute \Sigma_{0<=k<n} normMatrix^k
    auto sumTerm = std::make_unique<Matrix>(adjTrans.getRows(), adjTrans.getCols());
    bool posCycle = adjTrans.allPairLongestPathMatrix(MPTime(0), true, *sumTerm);

    if (posCycle) {
        for (uint i = 0; i != adjTrans.getCols(); i++) {
            adjTrans.put(i, i, MP_MAX(adjTrans.get(i, i), 0));
        }
        Matrix newSumTerm = adjTrans.mp_power(this->getExponent() - 1);
        Vector lat = newSumTerm.mp_multiply(*baseVec);
        return std::make_unique<Vector>(lat);
    }

    Vector lat = sumTerm->mp_multiply(*baseVec);
    return std::make_unique<Vector>(lat);
}

std::unique_ptr<Vector> OmegaSequenceL::computeLatencyVector() const {
    // compute the latency of the prefix expression
    auto *withoutExp = dynamic_cast<RegExLat *>((this->getWithoutOmega()).get());
    auto withoutVec = withoutExp->computeLatencyVector();

    // compute the latency of the \omega expression
    auto *withExp = dynamic_cast<RegExLat *>((this->getWithOmega()).get());
    auto withVecBase = withExp->computeLatencyVector();

    // compute the normalized matrix of the \omega expression
    auto withAdj = withExp->computeNormalizedStateMatrix();

    // compute its Kleene star
    auto withAdjStar = std::make_unique<Matrix>(withAdj->getRows(), withAdj->getCols());
    bool posCycle = withAdj->allPairLongestPathMatrix(MPTime(0), true, *withAdjStar);

    // undefined latency if the adjacency graph of this matrix contains a positive cycle
    if (posCycle) {
        throw ::CException("The latency is not defined. The period is lower than the maximum cycle "
                         "time of the application");
    }

    // compute the latency ((NormStateMatrix)*)^T\otimes \lambda(baseExpression)
    Matrix withAdjStarTrans = withAdjStar->transpose();
    Vector withVec = withAdjStarTrans.mp_multiply(*withVecBase);

    // if the prefix expression is empty return the latency of the \omega expression
    if (withoutVec == nullptr) {
        return std::make_unique<Vector>(withVec);
    }
    // else return the maximum latency of both expressions

    auto adj = withoutExp->computeNormalizedStateMatrix();

    Matrix adjTrans = adj->transpose();

    Vector adjTransWithVec = adjTrans.mp_multiply(withVec);

    auto lat = std::make_unique<Vector>(withVec.getSize(), MP_MINUS_INFINITY);

    withoutVec->maximum(adjTransWithVec, *lat);

    return lat;
}

std::unique_ptr<Vector> OmegaChoiceL::computeLatencyVector() const {
    // compute the latency of the left expression
    auto *leftExp = dynamic_cast<RegExLat *>((this->getLeft()).get());
    auto leftVec = leftExp->computeLatencyVector();

    // compute the latency of the right expression
    auto *rightExp = dynamic_cast<RegExLat *>((this->getRight()).get());
    auto rightVec = rightExp->computeLatencyVector();

    // create the latency vector and initialize it with minus infty elements
    auto lat = std::make_unique<Vector>(leftVec->getSize(), MP_MINUS_INFINITY);

    // take the maximum of the latency vectors of the right and the left expressions
    leftVec->maximum(*rightVec, *lat);

    return lat;
}

std::unique_ptr<Matrix> BlankL::computeNormalizedStateMatrix() const { return nullptr; }

std::unique_ptr<Matrix> PrimitiveL::computeNormalizedStateMatrix() const {
    // get the number of outputs produced in this scenario
    uint nrOutputs = this->getOutputMatrix()->getRows();

    // compute nrOutputs \times period
    MPTime t = MPTime(nrOutputs) * period;

    // subtract the result from all elements of the state matrix
    Matrix ast = this->getStateMatrix()->add(-t);

    return std::make_unique<Matrix>(ast);
}

std::unique_ptr<Matrix> SequenceL::computeNormalizedStateMatrix() const {

    // compute the normalized matrix of the first expression
    auto *firstExp = dynamic_cast<RegExLat *>((this->getFirst()).get());
    auto firstMat = firstExp->computeNormalizedStateMatrix();

    // compute the normalized matrix of the second expression
    auto *secondExp = dynamic_cast<RegExLat *>((this->getSecond()).get());
    auto secondMat = secondExp->computeNormalizedStateMatrix();

    // if the first expression is Blank type return the normalized
    // matrix of the second expression
    if (firstMat == nullptr) {
        return secondMat;
    }
    // else normMatrix(f.s) = normMatrix(s)\otimes normMatrix(f)
    Matrix ast = secondMat->mp_multiply(*firstMat);

    return std::make_unique<Matrix>(ast);
}

std::unique_ptr<Matrix> ChoiceL::computeNormalizedStateMatrix() const {
    // compute the normalized matrix of the left expression
    auto *leftExp = dynamic_cast<RegExLat *>((this->getLeft()).get());
    auto leftMat = leftExp->computeNormalizedStateMatrix();

    // compute the normalized matrix of the right expression
    auto *rightExp = dynamic_cast<RegExLat *>((this->getRight()).get());
    auto rightMat = rightExp->computeNormalizedStateMatrix();

    // compute the maximum of the normalized state matrices of both expressions
    Matrix ast = leftMat->mp_maximum(*rightMat);

    return std::make_unique<Matrix>(ast);
}

std::unique_ptr<Matrix> KleeneIterationL::computeNormalizedStateMatrix() const {

    // compute the normalized state matrix of the base expression
    auto *baseExp = dynamic_cast<RegExLat *>((this->getBase()).get());
    auto baseMat = baseExp->computeNormalizedStateMatrix();

    // compute the Kleene star of the result
    auto ast = std::make_unique<Matrix>(baseMat->getRows(), baseMat->getCols());
    bool posCycle = baseMat->allPairLongestPathMatrix(MPTime(0), true, *ast);

    // the computed star is invalid if there a positive cycle is detected
    if (posCycle) {
        throw ::CException("The latency is not defined. The period is lower than the maximum cycle "
                         "time of the application");
    }

    return ast;
}

std::unique_ptr<Matrix> ConstantIterationL::computeNormalizedStateMatrix() const {
    // compute the normalized state matrix of the base expression
    auto *baseExp = dynamic_cast<RegExLat *>((this->getBase()).get());
    auto baseMat = baseExp->computeNormalizedStateMatrix();

    // compute normMatrix^exponent
    Matrix ast = baseMat->mp_power(this->getExponent());

    return std::make_unique<Matrix>(ast);
}

std::unique_ptr<Matrix> OmegaSequenceL::computeNormalizedStateMatrix() const {
    // normalized matrix for this type of expression is not defined.
    return nullptr;
}

std::unique_ptr<Matrix> OmegaChoiceL::computeNormalizedStateMatrix() const {
    // normalized matrix for this type of expression is not defined.
    return nullptr;
}