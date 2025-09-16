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

#ifndef FSMSADF_REGEX_SADF_H_INCLUDED
#define FSMSADF_REGEX_SADF_H_INCLUDED

#include "fsmsadf/base/regexfactory.h"
#include "maxplus/algebra/maxplus.h" // IWYU pragma: keep
#include <memory>

#ifdef _MSC_VER
#pragma warning(disable : 4250) /* suppress warning C4250: for instance 'BlankS': inherits         \
                                   'Blank::Blank::print' via dominance */
#endif

/*a subclass of regex that contains state and output matrices of scenarios.
It is in fact a data structure to represent (max,+)-level definition of FSMSADF.*/
class RegExLat : public virtual RegEx {
public:
    /*transforms an RegExLat to a more compact RegExLat using the notion of Abstractable set of
     * scenario sequences.*/
    [[nodiscard]] virtual std::unique_ptr<MaxPlus::Vector> computeLatencyVector() const = 0;

    /*computes the normalized state matrix; required for latency computation.*/
    [[nodiscard]] virtual std::unique_ptr<MaxPlus::Matrix> computeNormalizedStateMatrix() const = 0;

    // the latency is computed with respect to this period.
    static MaxPlus::MPTime period;

    // virtual ~RegExLat(){}
};

/*Derived regex classes*/
class BlankL : public Blank, public RegExLat {
public:
    BlankL() = default;

    ~BlankL() override = default;

    BlankL(const BlankL &) = delete;
    BlankL &operator=(const BlankL &) = delete;
    BlankL(BlankL &&) = delete;
    BlankL &operator=(BlankL &&) = delete;

    [[nodiscard]] std::unique_ptr<MaxPlus::Vector> computeLatencyVector() const override;

    [[nodiscard]] std::unique_ptr<MaxPlus::Matrix> computeNormalizedStateMatrix() const override;
};

class PrimitiveL : public Primitive, public RegExLat {
public:
    PrimitiveL(CString &n, std::unique_ptr<MaxPlus::Matrix> s, std::unique_ptr<MaxPlus::Matrix> o) :
        Primitive(n), stateMatrix(std::move(s)), outputMatrix(std::move(o)) {}

    ~PrimitiveL() override = default;

    PrimitiveL(const PrimitiveL &) = delete;
    PrimitiveL &operator=(const PrimitiveL &) = delete;
    PrimitiveL(PrimitiveL &&) = delete;
    PrimitiveL &operator=(PrimitiveL &&) = delete;

    [[nodiscard]] std::unique_ptr<MaxPlus::Vector> computeLatencyVector() const override;

    [[nodiscard]] MaxPlus::Matrix *getStateMatrix() const { return stateMatrix.get(); }

    [[nodiscard]] MaxPlus::Matrix *getOutputMatrix() const { return outputMatrix.get(); }

    [[nodiscard]] std::unique_ptr<MaxPlus::Matrix> computeNormalizedStateMatrix() const override;

private:
    std::unique_ptr<MaxPlus::Matrix> stateMatrix;
    std::unique_ptr<MaxPlus::Matrix> outputMatrix;
};

class SequenceL : public Sequence, public RegExLat {
public:
    SequenceL(std::unique_ptr<RegEx> f, std::unique_ptr<RegEx> s) :
        Sequence(std::move(f), std::move(s)) {}

    ~SequenceL() override = default;

    SequenceL(const SequenceL&) = delete;
    SequenceL& operator=(const SequenceL&) = delete;
    SequenceL(SequenceL&&) = delete;
    SequenceL& operator=(SequenceL&&) = delete;


    [[nodiscard]] std::unique_ptr<MaxPlus::Vector> computeLatencyVector() const override;

    [[nodiscard]] std::unique_ptr<MaxPlus::Matrix> computeNormalizedStateMatrix() const override;
};

class ChoiceL : public Choice, public RegExLat {
public:
    ChoiceL(std::unique_ptr<RegEx> l, std::unique_ptr<RegEx> r) :
        Choice(std::move(l), std::move(r)) {}

    ~ChoiceL() override = default;

    ChoiceL(const ChoiceL&) = delete;
    ChoiceL& operator=(const ChoiceL&) = delete;
    ChoiceL(ChoiceL&&) = delete;
    ChoiceL& operator=(ChoiceL&&) = delete;


    [[nodiscard]] std::unique_ptr<MaxPlus::Vector> computeLatencyVector() const override;

    [[nodiscard]] std::unique_ptr<MaxPlus::Matrix> computeNormalizedStateMatrix() const override;
};

class KleeneIterationL : public KleeneIteration, public RegExLat {
public:
    explicit KleeneIterationL(std::unique_ptr<RegEx> i) : KleeneIteration(std::move(i)) {}

    ~KleeneIterationL() override = default;

    KleeneIterationL(const KleeneIterationL&) = delete;
    KleeneIterationL& operator=(const KleeneIterationL&) = delete;
    KleeneIterationL(KleeneIterationL&&) = delete;
    KleeneIterationL& operator=(KleeneIterationL&&) = delete;

    [[nodiscard]] std::unique_ptr<MaxPlus::Vector> computeLatencyVector() const override;

    [[nodiscard]] std::unique_ptr<MaxPlus::Matrix> computeNormalizedStateMatrix() const override;
};

class ConstantIterationL : public ConstantIteration, public RegExLat {
public:
    ConstantIterationL(std::unique_ptr<RegEx> b, unsigned int e) :
        ConstantIteration(std::move(b), e) {}

    ~ConstantIterationL() override = default;

    ConstantIterationL(const ConstantIterationL&) = delete;
    ConstantIterationL& operator=(const ConstantIterationL&) = delete;
    ConstantIterationL(ConstantIterationL&&) = delete;
    ConstantIterationL& operator=(ConstantIterationL&&) = delete;

    [[nodiscard]] std::unique_ptr<MaxPlus::Vector> computeLatencyVector() const override;

    [[nodiscard]] std::unique_ptr<MaxPlus::Matrix> computeNormalizedStateMatrix() const override;
};

class OmegaSequenceL : public OmegaSequence, public RegExLat {
public:
    OmegaSequenceL(std::unique_ptr<RegEx> without, std::unique_ptr<RegEx> with) :
        OmegaSequence(std::move(without), std::move(with)) {}

    ~OmegaSequenceL() override = default;

    OmegaSequenceL(const OmegaSequenceL&) = delete;
    OmegaSequenceL& operator=(const OmegaSequenceL&) = delete;
    OmegaSequenceL(OmegaSequenceL&&) = delete;
    OmegaSequenceL& operator=(OmegaSequenceL&&) = delete;

    [[nodiscard]] std::unique_ptr<MaxPlus::Vector> computeLatencyVector() const override;

    [[nodiscard]] std::unique_ptr<MaxPlus::Matrix> computeNormalizedStateMatrix() const override;
};

class OmegaChoiceL : public OmegaChoice, public RegExLat {
public:
    OmegaChoiceL(std::unique_ptr<RegEx> l, std::unique_ptr<RegEx> r) :
        OmegaChoice(std::move(l), std::move(r)) {}

    ~OmegaChoiceL() override = default;

    OmegaChoiceL(const OmegaChoiceL&) = delete;
    OmegaChoiceL& operator=(const OmegaChoiceL&) = delete;
    OmegaChoiceL(OmegaChoiceL&&) = delete;
    OmegaChoiceL& operator=(OmegaChoiceL&&) = delete;


    [[nodiscard]] std::unique_ptr<MaxPlus::Vector> computeLatencyVector() const override;

    [[nodiscard]] std::unique_ptr<MaxPlus::Matrix> computeNormalizedStateMatrix() const override;
};

#endif