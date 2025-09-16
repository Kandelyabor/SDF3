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

#include "regex_thr.h"
#include <memory>
#include <sstream>

namespace FSMSADF {

using namespace MaxPlus;

uint PrimitiveT::primitiveId = 0;

std::unique_ptr<RegExThr> BlankT::computeCompactRegExThr() const {
    return std::make_unique<BlankT>();
}

std::unique_ptr<RegExThr> PrimitiveT::computeCompactRegExThr() const {
    std::stringstream id;
    id << PrimitiveT::primitiveId;
    return std::make_unique<PrimitiveT>(CString("S") + CString(id.str()),
                                        std::make_unique<Matrix>(*(this->getMatrix())),
                                        this->getReward(),
                                        this->getSubExpression());
}

std::unique_ptr<RegExThr> SequenceT::computeCompactRegExThr() const {
    auto *firstExp = dynamic_cast<RegExThr *>((this->getFirst()).get());
    std::unique_ptr<RegExThr> compactFirst = firstExp->computeCompactRegExThr();

    auto *secondExp = dynamic_cast<RegExThr *>((this->getSecond()).get());
    std::unique_ptr<RegExThr> compactSecond = secondExp->computeCompactRegExThr();

    auto *firstPrimitiveBlank = dynamic_cast<BlankT *>(compactFirst.get());
    auto *secondPrimitiveBlank = dynamic_cast<BlankT *>(compactSecond.get());

    if (secondPrimitiveBlank != nullptr) {
        std::cout << "second empty" << std::endl;
    }

    if (firstPrimitiveBlank != nullptr) {
        return compactSecond;
    }

    auto *firstPrimitive = dynamic_cast<PrimitiveT *>(compactFirst.get());
    auto *secondPrimitive = dynamic_cast<PrimitiveT *>(compactSecond.get());

    if ((firstPrimitive != nullptr) && (secondPrimitive != nullptr)) {
        Matrix matrix = secondPrimitive->getMatrix()->mp_multiply(*(firstPrimitive->getMatrix()));
        std::stringstream id;
        id << PrimitiveT::primitiveId;
        auto newPrimitive = std::make_unique<PrimitiveT>(
                CString("S" + id.str()),
                std::make_unique<Matrix>(matrix),
                firstPrimitive->getReward() + secondPrimitive->getReward(),
                firstPrimitive->getSubExpression() + "," + secondPrimitive->getSubExpression());

        return newPrimitive;
    }

    return std::make_unique<SequenceT>(std::move(compactFirst), std::move(compactSecond));
}

std::unique_ptr<RegExThr> ChoiceT::computeCompactRegExThr() const {
    auto *leftExp = dynamic_cast<RegExThr *>((this->getLeft()).get());
    std::unique_ptr<RegExThr> compactLeft = leftExp->computeCompactRegExThr();

    auto *rightExp = dynamic_cast<RegExThr *>((this->getRight()).get());
    std::unique_ptr<RegExThr> compactRight = rightExp->computeCompactRegExThr();

    auto *leftPrimitive = dynamic_cast<PrimitiveT *>(compactLeft.get());
    auto *rightPrimitive = dynamic_cast<PrimitiveT *>(compactRight.get());

    if ((leftPrimitive != nullptr) && (rightPrimitive != nullptr)
        && leftPrimitive->getReward() == rightPrimitive->getReward()) {
        auto matrix = std::make_unique<Matrix>(*(leftPrimitive->getMatrix()));
        (rightPrimitive->getMatrix())->maximum(*(leftPrimitive->getMatrix()), *matrix);
        std::stringstream id;
        id << PrimitiveT::primitiveId;
        auto newPrimitive = std::make_unique<PrimitiveT>(CString("S" + id.str()),
                                            std::move(matrix),
                                            rightPrimitive->getReward(),
                                            leftPrimitive->getSubExpression() + "+"
                                                    + rightPrimitive->getSubExpression());

        return newPrimitive;
    }
    return std::make_unique<ChoiceT>(std::move(compactLeft), std::move(compactRight));
}

std::unique_ptr<RegExThr> KleeneIterationT::computeCompactRegExThr() const {
    auto *baseExp = dynamic_cast<RegExThr *>(this->getBase().get());
    std::unique_ptr<RegExThr> compactBase = baseExp->computeCompactRegExThr();

    return std::make_unique<KleeneIterationT>(std::move(compactBase));
}

std::unique_ptr<RegExThr> ConstantIterationT::computeCompactRegExThr() const {
    auto *baseExp = dynamic_cast<RegExThr *>(this->getBase().get());
    std::unique_ptr<RegExThr> compactBase = baseExp->computeCompactRegExThr();

    auto *primitiveBase = dynamic_cast<PrimitiveT *>(compactBase.get());

    if (primitiveBase != nullptr) {
        Matrix matrix = primitiveBase->getMatrix()->mp_power(this->getExponent());
        std::stringstream id;
        std::stringstream exp;
        id << PrimitiveT::primitiveId;
        exp << static_cast<uint>(this->getExponent());
        auto newPrimitive =
                std::make_unique<PrimitiveT>(CString("S" + id.str()),
                               std::make_unique<Matrix>(matrix),
                               this->getExponent() * primitiveBase->getReward(),
                               "(" + primitiveBase->getSubExpression() + ")^" + CString(exp.str()));

        return newPrimitive;
    }
    return std::make_unique<ConstantIterationT>(std::move(compactBase), this->getExponent());
}

std::unique_ptr<RegExThr> OmegaSequenceT::computeCompactRegExThr() const {
    auto *withoutExp = dynamic_cast<RegExThr *>(this->getWithoutOmega().get());
    std::unique_ptr<RegEx> compactWithout = withoutExp->computeCompactRegExThr();

    auto *withExp = dynamic_cast<RegExThr *>(this->getWithOmega().get());
    std::unique_ptr<RegEx> compactWith = withExp->computeCompactRegExThr();

    return std::make_unique<OmegaSequenceT>(std::move(compactWithout), std::move(compactWith));
}

std::unique_ptr<RegExThr> OmegaChoiceT::computeCompactRegExThr() const {
    auto *leftExp = dynamic_cast<RegExThr *>((this->getLeft()).get());
    std::unique_ptr<RegEx> compactLeft = leftExp->computeCompactRegExThr();

    auto *rightExp = dynamic_cast<RegExThr *>((this->getRight()).get());
    std::unique_ptr<RegEx> compactRight = rightExp->computeCompactRegExThr();

    return std::make_unique<OmegaChoiceT>(std::move(compactLeft), std::move(compactRight));
}

void BlankT::collectInformation(ThroughputAnalysisInformation &t) {}

void PrimitiveT::collectInformation(ThroughputAnalysisInformation &t) {
    t.store(this->getName(), std::make_unique<Matrix>(*(this->scenarioMatrix)), this->reward, this->subExpression);
}

void SequenceT::collectInformation(ThroughputAnalysisInformation &t) {
    auto *firstExp = dynamic_cast<RegExThr *>((this->getFirst()).get());
    firstExp->collectInformation(t);

    auto *secondExp = dynamic_cast<RegExThr *>((this->getSecond()).get());
    secondExp->collectInformation(t);
}

void ChoiceT::collectInformation(ThroughputAnalysisInformation &t) {
    auto *leftExp = dynamic_cast<RegExThr *>((this->getLeft()).get());
    leftExp->collectInformation(t);

    auto *rightExp = dynamic_cast<RegExThr *>((this->getRight()).get());
    rightExp->collectInformation(t);
}

void KleeneIterationT::collectInformation(ThroughputAnalysisInformation &t) {
    auto *baseExp = dynamic_cast<RegExThr *>(this->getBase().get());
    baseExp->collectInformation(t);
}

void ConstantIterationT::collectInformation(ThroughputAnalysisInformation &t) {
    auto *baseExp = dynamic_cast<RegExThr *>(this->getBase().get());
    baseExp->collectInformation(t);
}

void OmegaSequenceT::collectInformation(ThroughputAnalysisInformation &t) {
    auto *withoutExp = dynamic_cast<RegExThr *>(this->getWithoutOmega().get());
    withoutExp->collectInformation(t);

    auto *withExp = dynamic_cast<RegExThr *>(this->getWithOmega().get());
    withExp->collectInformation(t);
}

void OmegaChoiceT::collectInformation(ThroughputAnalysisInformation &t) {
    auto *leftExp = dynamic_cast<RegExThr *>((this->getLeft()).get());
    leftExp->collectInformation(t);

    auto *rightExp = dynamic_cast<RegExThr *>((this->getRight()).get());
    rightExp->collectInformation(t);
}

} // namespace FSMSADF
