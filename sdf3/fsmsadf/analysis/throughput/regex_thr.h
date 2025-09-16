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

#ifndef FSMSADF_REGEX_THR_H_INCLUDED
#define FSMSADF_REGEX_THR_H_INCLUDED

#include "fsmsadf/base/regexfactory.h"
#include "thrutils.h"
#include <memory>
#include <utility>

namespace FSMSADF {

#ifdef _MSC_VER
#pragma warning(disable : 4250) /* suppress warning C4250: for instance 'BlankS': inherits         \
                                   'Blank::Blank::print' via dominance */
#endif

/*a subclass of regex that contains extra information about scenarios i.e. scenario matrices and
rewards. It is in fact a data structure to represent (max,+)-level definition of FSMSADF.*/
class RegExThr : public virtual RegEx {
public:
    /*transforms an RegExThr to a more compact RegExThr using the notion of Abstractable set of
     * scenario sequences.*/
    [[nodiscard]] virtual std::unique_ptr<RegExThr> computeCompactRegExThr() const = 0;

    /*traverses the expression tree and collects the set of scenarios, scenarioMatrices,
    scenarioRewards and the actual subexpressions that have been transformed to single scenarios
    after compression.*/
    virtual void collectInformation(ThroughputAnalysisInformation &t) = 0;

    // virtual ~RegExThr(){}
};

/*Derived regex classes*/
class BlankT : public Blank, public RegExThr {
public:
    BlankT() = default;
    ~BlankT() override = default;

    // Delete copy/move constructors and assignment operators
    BlankT(const BlankT &) = delete;
    BlankT &operator=(const BlankT &) = delete;
    BlankT(BlankT &&) = delete;
    BlankT &operator=(BlankT &&) = delete;

    [[nodiscard]] std::unique_ptr<RegExThr> computeCompactRegExThr() const override;

    void collectInformation(ThroughputAnalysisInformation &t) override;
};

class PrimitiveT : public Primitive, public RegExThr {
public:
    PrimitiveT(const CString &s, std::unique_ptr<MaxPlus::Matrix> m, CDouble r, CString se) :
        Primitive(s), scenarioMatrix(std::move(m)), reward(r), subExpression(std::move(se)) {
        PrimitiveT::primitiveId++;
    }

    ~PrimitiveT() override = default;

    // Delete copy/move constructors and assignment operators
    PrimitiveT(const PrimitiveT &) = delete;
    PrimitiveT &operator=(const PrimitiveT &) = delete;
    PrimitiveT(PrimitiveT &&) = delete;
    PrimitiveT &operator=(PrimitiveT &&) = delete;


    [[nodiscard]] std::unique_ptr<RegExThr> computeCompactRegExThr() const override;

    [[nodiscard]] CDouble getReward() const { return reward; }

    [[nodiscard]] MaxPlus::Matrix *getMatrix() const { return scenarioMatrix.get(); }

    [[nodiscard]] CString getSubExpression() const { return subExpression; }

    void collectInformation(ThroughputAnalysisInformation &t) override;

    /*keeps track of the number of produced primitives. It is
    used in naming auto generated regular expressions.*/
    static uint primitiveId;

private:
    std::unique_ptr<MaxPlus::Matrix> scenarioMatrix;
    CDouble reward;
    CString subExpression;
};

class SequenceT : public Sequence, public RegExThr {
public:
    SequenceT(std::unique_ptr<RegEx> f, std::unique_ptr<RegEx> s) :
        Sequence(std::move(f), std::move(s)) {}

    ~SequenceT() override = default;

    // Delete copy/move constructors and assignment operators
    SequenceT(const SequenceT &) = delete;
    SequenceT &operator=(const SequenceT &) = delete;
    SequenceT(SequenceT &&) = delete;
    SequenceT &operator=(SequenceT &&) = delete;


    [[nodiscard]] std::unique_ptr<RegExThr> computeCompactRegExThr() const override;

    void collectInformation(ThroughputAnalysisInformation &t) override;
};

class ChoiceT : public Choice, public RegExThr {
public:
    ChoiceT(std::unique_ptr<RegEx> l, std::unique_ptr<RegEx> r) :
        Choice(std::move(l), std::move(r)) {}

    ~ChoiceT() override = default;

    // Delete copy/move constructors and assignment operators
    ChoiceT(const ChoiceT &) = delete;
    ChoiceT &operator=(const ChoiceT &) = delete;
    ChoiceT(ChoiceT &&) = delete;
    ChoiceT &operator=(ChoiceT &&) = delete;

    [[nodiscard]] std::unique_ptr<RegExThr> computeCompactRegExThr() const override;

    void collectInformation(ThroughputAnalysisInformation &t) override;
};

class KleeneIterationT : public KleeneIteration, public RegExThr {
public:
    explicit KleeneIterationT(std::unique_ptr<RegEx> i) : KleeneIteration(std::move(i)) {}

    ~KleeneIterationT() override = default;

    // Delete copy/move constructors and assignment operators
    KleeneIterationT(const KleeneIterationT &) = delete;
    KleeneIterationT &operator=(const KleeneIterationT &) = delete;
    KleeneIterationT(KleeneIterationT &&) = delete;
    KleeneIterationT &operator=(KleeneIterationT &&) = delete;

    [[nodiscard]] std::unique_ptr<RegExThr> computeCompactRegExThr() const override;

    void collectInformation(ThroughputAnalysisInformation &t) override;
};

class ConstantIterationT : public ConstantIteration, public RegExThr {
public:
    ConstantIterationT(std::unique_ptr<RegEx> b, unsigned int e) :
        ConstantIteration(std::move(b), e) {}

    ~ConstantIterationT() override = default;

    // Delete copy/move constructors and assignment operators
    ConstantIterationT(const ConstantIterationT &) = delete;
    ConstantIterationT &operator=(const ConstantIterationT &) = delete;
    ConstantIterationT(ConstantIterationT &&) = delete;
    ConstantIterationT &operator=(ConstantIterationT &&) = delete;

    [[nodiscard]] std::unique_ptr<RegExThr> computeCompactRegExThr() const override;

    void collectInformation(ThroughputAnalysisInformation &t) override;
};

class OmegaSequenceT : public OmegaSequence, public RegExThr {
public:
    OmegaSequenceT(std::unique_ptr<RegEx> without, std::unique_ptr<RegEx> with) :
        OmegaSequence(std::move(without), std::move(with)) {}

    ~OmegaSequenceT() override = default;

    // Delete copy/move constructors and assignment operators
    OmegaSequenceT(const OmegaSequenceT &) = delete;
    OmegaSequenceT &operator=(const OmegaSequenceT &) = delete;
    OmegaSequenceT(OmegaSequenceT &&) = delete;
    OmegaSequenceT &operator=(OmegaSequenceT &&) = delete;

    [[nodiscard]] std::unique_ptr<RegExThr> computeCompactRegExThr() const override;

    void collectInformation(ThroughputAnalysisInformation &t) override;
};

class OmegaChoiceT : public OmegaChoice, public RegExThr {
public:
    OmegaChoiceT(std::unique_ptr<RegEx> l, std::unique_ptr<RegEx> r) :
        OmegaChoice(std::move(l), std::move(r)) {}

    ~OmegaChoiceT() override = default;

    // Delete copy/move constructors and assignment operators
    OmegaChoiceT(const OmegaChoiceT &) = delete;
    OmegaChoiceT &operator=(const OmegaChoiceT &) = delete;
    OmegaChoiceT(OmegaChoiceT &&) = delete;
    OmegaChoiceT &operator=(OmegaChoiceT &&) = delete;


    [[nodiscard]] std::unique_ptr<RegExThr> computeCompactRegExThr() const override;

    void collectInformation(ThroughputAnalysisInformation &t) override;
};

} // namespace FSMSADF

#endif