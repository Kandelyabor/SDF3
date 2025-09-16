/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   regex_scenarios.h
 *
 *  Author          :   Hadi Alizadeh Ara (s.h.seyyed.alizadeh@tue.nl)
 *
 *  Date            :   July 30, 2017
 *
 *  Function        :   defines a regular expression that contains scenario information
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

#ifndef FSMSADF_REGEX_SCENARIOS_H_INCLUDED
#define FSMSADF_REGEX_SCENARIOS_H_INCLUDED

#include <utility>

#include "regexfactory.h"

#ifdef _MSC_VER
#pragma warning(disable : 4250) /* suppress warning C4250: for instance 'BlankS': inherits         \
                                   'Blank::Blank::print' via dominance */
#endif

/* A datatype for storing the required information from scenarios.*/
class RegExPersistentTokens {
public:
    CStrings persistentTokensOfExpression;
    CStrings persistentTokensFinalOfExpression;
};

/*a subclass of regex that contains extra information about scenarios.*/
class RegExScenarios : public virtual RegEx {
public:
    /*consistency check for FSMSADF. It checks the consistency in a recursive fashion.*/
    [[nodiscard]] virtual std::unique_ptr<RegExPersistentTokens> checkConsistency() const = 0;

    // virtual ~RegExScenarios(){}
};

/*Derived classes from regex factory*/
class BlankS : public Blank, public RegExScenarios {
public:
    BlankS() = default;

    ~BlankS() override = default;

    BlankS(const BlankS &) = delete;
    BlankS &operator=(const BlankS &) = delete;
    BlankS(BlankS &&) = delete;
    BlankS &operator=(BlankS &&) = delete;

    [[nodiscard]] std::unique_ptr<RegExPersistentTokens> checkConsistency() const override;
};

class PrimitiveS : public Primitive, public RegExScenarios {
public:
    PrimitiveS(const CString &s, CStrings pts, CStrings ptfs) :
        Primitive(s),
        persistentTokensOfScenario(std::move(pts)),
        persistentTokensFinalOfScenario(std::move(ptfs)) {}

    ~PrimitiveS() override = default;

    PrimitiveS(const PrimitiveS &) = delete;
    PrimitiveS &operator=(const PrimitiveS &) = delete;
    PrimitiveS(PrimitiveS &&) = delete;
    PrimitiveS &operator=(PrimitiveS &&) = delete;

    [[nodiscard]] std::unique_ptr<RegExPersistentTokens> checkConsistency() const override;

    CStrings getPersistentTokens() { return persistentTokensOfScenario; }

    CStrings getPersistentFinalTokens() { return persistentTokensFinalOfScenario; }

private:
    CStrings persistentTokensOfScenario;
    CStrings persistentTokensFinalOfScenario;
};

class SequenceS : public Sequence, public RegExScenarios {
public:
    SequenceS(std::unique_ptr<RegEx> f, std::unique_ptr<RegEx> s) :
        Sequence(std::move(f), std::move(s)) {}

    ~SequenceS() override = default;

    SequenceS(const SequenceS &) = delete;
    SequenceS &operator=(const SequenceS &) = delete;
    SequenceS(SequenceS &&) = delete;
    SequenceS &operator=(SequenceS &&) = delete;

    [[nodiscard]] std::unique_ptr<RegExPersistentTokens> checkConsistency() const override;
};

class ChoiceS : public Choice, public RegExScenarios {
public:
    ChoiceS(std::unique_ptr<RegEx> l, std::unique_ptr<RegEx> r) :
        Choice(std::move(l), std::move(r)) {}

    ~ChoiceS() override = default;

    ChoiceS(const ChoiceS &) = delete;
    ChoiceS &operator=(const ChoiceS &) = delete;
    ChoiceS(ChoiceS &&) = delete;
    ChoiceS &operator=(ChoiceS &&) = delete;

    [[nodiscard]] std::unique_ptr<RegExPersistentTokens> checkConsistency() const override;
};

class KleeneIterationS : public KleeneIteration, public RegExScenarios {
public:
    explicit KleeneIterationS(std::unique_ptr<RegEx> i) : KleeneIteration(std::move(i)) {}

    ~KleeneIterationS() override = default;

    KleeneIterationS(const KleeneIterationS &) = delete;
    KleeneIterationS &operator=(const KleeneIterationS &) = delete;
    KleeneIterationS(KleeneIterationS &&) = delete;
    KleeneIterationS &operator=(KleeneIterationS &&) = delete;

    [[nodiscard]] std::unique_ptr<RegExPersistentTokens> checkConsistency() const override;
};

class ConstantIterationS : public ConstantIteration, public RegExScenarios {
public:
    ConstantIterationS(std::unique_ptr<RegEx> b, unsigned int e) :
        ConstantIteration(std::move(b), e) {}

    ~ConstantIterationS() override = default;

    ConstantIterationS(const ConstantIterationS &) = delete;
    ConstantIterationS &operator=(const ConstantIterationS &) = delete;
    ConstantIterationS(ConstantIterationS &&) = delete;
    ConstantIterationS &operator=(ConstantIterationS &&) = delete;

    [[nodiscard]] std::unique_ptr<RegExPersistentTokens> checkConsistency() const override;
};

class OmegaSequenceS : public OmegaSequence, public RegExScenarios {
public:
    OmegaSequenceS(std::unique_ptr<RegEx> without, std::unique_ptr<RegEx> with) :
        OmegaSequence(std::move(without), std::move(with)) {}

    ~OmegaSequenceS() override = default;

    OmegaSequenceS(const OmegaSequenceS &) = delete;
    OmegaSequenceS &operator=(const OmegaSequenceS &) = delete;
    OmegaSequenceS(OmegaSequenceS &&) = delete;
    OmegaSequenceS &operator=(OmegaSequenceS &&) = delete;

    [[nodiscard]] std::unique_ptr<RegExPersistentTokens> checkConsistency() const override;
};

class OmegaChoiceS : public OmegaChoice, public RegExScenarios {
public:
    OmegaChoiceS(std::unique_ptr<RegEx> l, std::unique_ptr<RegEx> r) :
        OmegaChoice(std::move(l), std::move(r)) {}

    ~OmegaChoiceS() override = default;

    OmegaChoiceS(const OmegaChoiceS &) = delete;
    OmegaChoiceS &operator=(const OmegaChoiceS &) = delete;
    OmegaChoiceS(OmegaChoiceS &&) = delete;
    OmegaChoiceS &operator=(OmegaChoiceS &&) = delete;

    [[nodiscard]] std::unique_ptr<RegExPersistentTokens> checkConsistency() const override;
};

#endif