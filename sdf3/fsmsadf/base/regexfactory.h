/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   regexfactory.h
 *
 *  Author          :   Hadi Alizadeh Ara (s.h.seyyed.alizadeh@tue.nl)
 *
 *  Date            :   July 30, 2017
 *
 *  Function        :   defines a factory method for omega regular expressions
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

#ifndef SDF3_FSMSADF_REGEXFACTORY_H_INCLUDED
#define SDF3_FSMSADF_REGEXFACTORY_H_INCLUDED

#include "fsmutils.h"

class Primitive;

/*
An abstract data type to represent a regular expression.
*/
class RegEx {
public:
    RegEx() = default;

    // outputs the regular on the screen
    virtual void print() const = 0;

    // outputs the the regular expression as a string stream
    virtual void toString(std::stringstream &result) const = 0;

    // converts regular expressions to non-deterministic finite automaton with 'epsilon' edges
    // (NFA-e)
    virtual std::unique_ptr<EdgeLabeledScenarioFSMInterface>
    convertToFSM(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const = 0;

    // converts deterministic regular expressions to non-deterministic epsilon free finite automaton
    virtual std::unique_ptr<EdgeLabeledScenarioFSMInterface>
    convertToDFA(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const = 0;

    // to a single sequence
    virtual void toSequence(std::stringstream &result) const = 0;

    // check if the expression is deterministic
    [[nodiscard]] virtual bool checkDeterministic() const = 0;

    // compute the length of the expression
    [[nodiscard]] virtual unsigned int computeLength() const = 0;

    // extract the alphabet of the expression
    [[nodiscard]] virtual std::vector<std::string> extractAlphabet() const = 0;

    // move to the next leaf
    [[nodiscard]] virtual const Primitive *moveToNextLeaf() const = 0;

    // virtual destructor
    virtual ~RegEx() = default;

    RegEx(const RegEx&) = delete;
    RegEx& operator=(const RegEx&) = delete;
    RegEx(RegEx&&) = delete;
    RegEx& operator=(RegEx&&) = delete;

};

// blank regex
class Blank : public virtual RegEx {
public:
    Blank() = default;
    ~Blank() override = default;

    Blank(const Blank&) = delete;
    Blank& operator=(const Blank&) = delete;
    Blank(Blank&&) = delete;
    Blank& operator=(Blank&&) = delete;

    void toString(std::stringstream &result) const override {}
    void toSequence(std::stringstream &result) const override {}
    void print() const override {}
    std::unique_ptr<EdgeLabeledScenarioFSMInterface>
    convertToFSM(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const override;
    std::unique_ptr<EdgeLabeledScenarioFSMInterface>
    convertToDFA(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const override;
    [[nodiscard]] bool checkDeterministic() const override;
    [[nodiscard]] unsigned int computeLength() const override;
    [[nodiscard]] std::vector<std::string> extractAlphabet() const override;
    [[nodiscard]] const Primitive *moveToNextLeaf() const override;
};

// primitive regex e.g. a
class Primitive : public virtual RegEx {
public:
    explicit Primitive(CString s) : name(std::move(s)) {}

    ~Primitive() override = default;

    Primitive(const Primitive&) = delete;
    Primitive& operator=(const Primitive&) = delete;
    Primitive(Primitive&&) = delete;
    Primitive& operator=(Primitive&&) = delete;

    void toString(std::stringstream &result) const override;
    void toSequence(std::stringstream &result) const override;
    void print() const override;
    [[nodiscard]] bool checkDeterministic() const override;
    std::unique_ptr<EdgeLabeledScenarioFSMInterface>
    convertToFSM(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const override;
    std::unique_ptr<EdgeLabeledScenarioFSMInterface>
    convertToDFA(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const override;
    [[nodiscard]] unsigned int computeLength() const override;
    [[nodiscard]] std::vector<std::string> extractAlphabet() const override;
    [[nodiscard]] virtual CString getName() const { return name; }
    [[nodiscard]] const Primitive *moveToNextLeaf() const override;

private:
    CString name;
};

// kleene iteration regex e.g. a*
class KleeneIteration : public virtual RegEx {
public:
    explicit KleeneIteration(std::unique_ptr<RegEx> i) : base(std::move(i)) {}

    ~KleeneIteration() override = default;

    KleeneIteration(const KleeneIteration&) = delete;
    KleeneIteration& operator=(const KleeneIteration&) = delete;
    KleeneIteration(KleeneIteration&&) = delete;
    KleeneIteration& operator=(KleeneIteration&&) = delete;

    void toString(std::stringstream &result) const override;
    void toSequence(std::stringstream &result) const override;
    [[nodiscard]] bool checkDeterministic() const override;
    void print() const override;
    std::unique_ptr<EdgeLabeledScenarioFSMInterface>
    convertToFSM(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const override;
    std::unique_ptr<EdgeLabeledScenarioFSMInterface>
    convertToDFA(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const override;
    [[nodiscard]] unsigned int computeLength() const override;
    [[nodiscard]] std::vector<std::string> extractAlphabet() const override;
    [[nodiscard]] const Primitive *moveToNextLeaf() const override;
    [[nodiscard]] const std::unique_ptr<RegEx> &getBase() const { return this->base; };

private:
    std::unique_ptr<RegEx> base;
};

// constant iteration regex e.g. (base)^exponent
class ConstantIteration : public virtual RegEx {
public:
    ConstantIteration(std::unique_ptr<RegEx> i, unsigned int e) : base(std::move(i)), exponent(e) {}

    ~ConstantIteration() override = default;

    ConstantIteration(const ConstantIteration&) = delete;
    ConstantIteration& operator=(const ConstantIteration&) = delete;
    ConstantIteration(ConstantIteration&&) = delete;
    ConstantIteration& operator=(ConstantIteration&&) = delete;

    void toString(std::stringstream &result) const override;
    void toSequence(std::stringstream &result) const override;
    [[nodiscard]] bool checkDeterministic() const override;
    void print() const override;
    std::unique_ptr<EdgeLabeledScenarioFSMInterface>
    convertToFSM(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const override;
    std::unique_ptr<EdgeLabeledScenarioFSMInterface>
    convertToDFA(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const override;
    [[nodiscard]] unsigned int computeLength() const override;
    [[nodiscard]] std::vector<std::string> extractAlphabet() const override;
    [[nodiscard]] const Primitive *moveToNextLeaf() const override;
    [[nodiscard]] const std::unique_ptr<RegEx> &getBase() const { return this->base; };
    [[nodiscard]] unsigned int getExponent() const { return this->exponent; };

private:
    std::unique_ptr<RegEx> base;
    unsigned int exponent;
};

// choice regex e.g. a+b
class Choice : public virtual RegEx {
public:
    Choice(std::unique_ptr<RegEx> l, std::unique_ptr<RegEx> r) :
        left(std::move(l)), right(std::move(r)) {}

    ~Choice() override = default;

    Choice(const Choice&) = delete;
    Choice& operator=(const Choice&) = delete;
    Choice(Choice&&) = delete;
    Choice& operator=(Choice&&) = delete;

    void toString(std::stringstream &result) const override;
    void toSequence(std::stringstream &result) const override;
    [[nodiscard]] bool checkDeterministic() const override;
    void print() const override;
    std::unique_ptr<EdgeLabeledScenarioFSMInterface>
    convertToFSM(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const override;
    std::unique_ptr<EdgeLabeledScenarioFSMInterface>
    convertToDFA(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const override;
    [[nodiscard]] unsigned int computeLength() const override;
    [[nodiscard]] std::vector<std::string> extractAlphabet() const override;
    [[nodiscard]] const Primitive *moveToNextLeaf() const override;
    [[nodiscard]] const std::unique_ptr<RegEx> &getLeft() const { return this->left; };
    [[nodiscard]] const std::unique_ptr<RegEx> &getRight() const { return this->right; };

private:
    std::unique_ptr<RegEx> left;
    std::unique_ptr<RegEx> right;
};

// sequence regex e.g. a,b
class Sequence : public virtual RegEx {
public:
    Sequence(std::unique_ptr<RegEx> f, std::unique_ptr<RegEx> s) :
        first(std::move(f)), second(std::move(s)) {}

    ~Sequence() override = default;

    Sequence(const Sequence&) = delete;
    Sequence& operator=(const Sequence&) = delete;
    Sequence(Sequence&&) = delete;
    Sequence& operator=(Sequence&&) = delete;

    void toString(std::stringstream &result) const override;
    void toSequence(std::stringstream &result) const override;
    [[nodiscard]] bool checkDeterministic() const override;
    void print() const override;
    std::unique_ptr<EdgeLabeledScenarioFSMInterface>
    convertToFSM(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const override;
    std::unique_ptr<EdgeLabeledScenarioFSMInterface>
    convertToDFA(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const override;
    [[nodiscard]] unsigned int computeLength() const override;
    [[nodiscard]] std::vector<std::string> extractAlphabet() const override;
    [[nodiscard]] const Primitive *moveToNextLeaf() const override;
    [[nodiscard]] const std::unique_ptr<RegEx> &getFirst() const { return this->first; };
    [[nodiscard]] const std::unique_ptr<RegEx> &getSecond() const { return this->second; };

private:
    std::unique_ptr<RegEx> first;
    std::unique_ptr<RegEx> second;
};

// omega sequence regex e.g. a.(b)^@ . \omega is represented by symbol '@'
class OmegaSequence : public virtual RegEx {
public:
    OmegaSequence(std::unique_ptr<RegEx> without, std::unique_ptr<RegEx> with) :
        withoutOmega(std::move(without)), withOmega(std::move(with)) {}

    ~OmegaSequence() override = default;

    OmegaSequence(const OmegaSequence&) = delete;
    OmegaSequence& operator=(const OmegaSequence&) = delete;
    OmegaSequence(OmegaSequence&&) = delete;
    OmegaSequence& operator=(OmegaSequence&&) = delete;

    void toString(std::stringstream &result) const override;
    void toSequence(std::stringstream &result) const override;
    [[nodiscard]] bool checkDeterministic() const override;
    void print() const override;
    std::unique_ptr<EdgeLabeledScenarioFSMInterface>
    convertToFSM(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const override;
    std::unique_ptr<EdgeLabeledScenarioFSMInterface>
    convertToDFA(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const override;
    [[nodiscard]] unsigned int computeLength() const override;
    [[nodiscard]] std::vector<std::string> extractAlphabet() const override;
    [[nodiscard]] const Primitive *moveToNextLeaf() const override;
    [[nodiscard]] const std::unique_ptr<RegEx> &getWithOmega() const { return this->withOmega; };
    [[nodiscard]] const std::unique_ptr<RegEx> &getWithoutOmega() const {
        return this->withoutOmega;
    };

private:
    std::unique_ptr<RegEx> withoutOmega;
    std::unique_ptr<RegEx> withOmega;
};

// omega choice regex e.g. a.(b)^@ + c.(d)^@
class OmegaChoice : public virtual RegEx {
public:
    OmegaChoice(std::unique_ptr<RegEx> l, std::unique_ptr<RegEx> r) :
        left(std::move(l)), right(std::move(r)) {}

    ~OmegaChoice() override = default;

    OmegaChoice(const OmegaChoice&) = delete;
    OmegaChoice& operator=(const OmegaChoice&) = delete;
    OmegaChoice(OmegaChoice&&) = delete;
    OmegaChoice& operator=(OmegaChoice&&) = delete;

    void toString(std::stringstream &result) const override;
    void toSequence(std::stringstream &result) const override;
    [[nodiscard]] bool checkDeterministic() const override;
    void print() const override;
    std::unique_ptr<EdgeLabeledScenarioFSMInterface>
    convertToFSM(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const override;
    std::unique_ptr<EdgeLabeledScenarioFSMInterface>
    convertToDFA(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const override;
    [[nodiscard]] unsigned int computeLength() const override;
    [[nodiscard]] std::vector<std::string> extractAlphabet() const override;
    [[nodiscard]] const Primitive *moveToNextLeaf() const override;
    [[nodiscard]] const std::unique_ptr<RegEx> &getLeft() const { return this->left; };
    [[nodiscard]] const std::unique_ptr<RegEx> &getRight() const { return this->right; };

private:
    std::unique_ptr<RegEx> left;
    std::unique_ptr<RegEx> right;
};

#endif