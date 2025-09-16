/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   regexfactory.cc
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

#include "regexfactory.h"
#include <cmath>
#include <sstream>

// prints a regex on the screen
void Primitive::print() const { std::cout << name; }

void KleeneIteration::print() const {
    std::cout << '(';
    base->print();
    std::cout << ')';
    std::cout << '*';
}

void ConstantIteration::print() const {
    std::cout << '(';
    base->print();
    std::cout << ')';
    std::cout << "^" << exponent;
}

void Choice::print() const {
    auto *r = dynamic_cast<Choice *>((this->getRight()).get());
    std::cout << '(';
    left->print();
    std::cout << ')';
    std::cout << '+';
    if (r != nullptr) {
        right->print();
    } else {
        std::cout << '(';
        right->print();
        std::cout << ')';
    }
}

void Sequence::print() const {
    auto *s = dynamic_cast<Sequence *>((this->getSecond()).get());
    first->print();
    auto *b = dynamic_cast<Blank *>((this->getFirst()).get());
    if (b == nullptr) {
        std::cout << ',';
    }
    if (s != nullptr) {
        second->print();
    } else {
        std::cout << '(';
        second->print();
        std::cout << ')';
    }
}

void OmegaSequence::print() const {
    std::cout << '(';
    withoutOmega->print();
    std::cout << ").(";
    withOmega->print();
    std::cout << ')';
    std::cout << '@';
}

void OmegaChoice::print() const {
    left->print();
    std::cout << '+';
    right->print();
}

// outputs a regex as a stringstream
void Primitive::toString(std::stringstream &result) const { result << name; }

void KleeneIteration::toString(std::stringstream &result) const {
    result << '(';
    base->toString(result);
    result << ')';
    result << '*';
}

void ConstantIteration::toString(std::stringstream &result) const {
    result << '(';
    base->toString(result);
    result << ')';
    result << "^" << exponent;
}

void Choice::toString(std::stringstream &result) const {
    auto *r = dynamic_cast<Choice *>((this->getRight()).get());
    result << '(';
    left->toString(result);
    result << ')';
    result << '+';
    if (r != nullptr) {
        right->toString(result);
    } else {
        result << '(';
        right->toString(result);
        result << ')';
    }
}

void Sequence::toString(std::stringstream &result) const {
    auto *s = dynamic_cast<Sequence *>((this->getSecond()).get());
    first->toString(result);
    auto *b = dynamic_cast<Blank *>((this->getFirst()).get());
    if (b == nullptr) {
        result << ',';
    }
    if (s != nullptr) {
        second->toString(result);
    } else {
        result << '(';
        second->toString(result);
        result << ')';
    }
}
void OmegaSequence::toString(std::stringstream &result) const {
    result << '(';
    withoutOmega->toString(result);
    result << ").(";
    withOmega->toString(result);
    result << ')';
    result << '@';
}

void OmegaChoice::toString(std::stringstream &result) const {
    left->toString(result);
    result << '+';
    right->toString(result);
}

// outputs the regex as a sequence if possible
void Primitive::toSequence(std::stringstream &result) const { result << name; }

void KleeneIteration::toSequence(std::stringstream & /*result*/) const {
    throw CException("The regular expression that contains KleeneIterations cannot be converted to "
                     "a single scenario.");
}

void ConstantIteration::toSequence(std::stringstream &result) const {
    auto *b = dynamic_cast<Primitive *>((this->getBase()).get());
    if (b != nullptr) {
        result << CString(this->exponent) + "*";
        base->toSequence(result);
    } else {
        std::stringstream temp;
        base->toSequence(temp);
        for (unsigned int i = 1; i != this->exponent; i++) {
            result << temp.str();
            result << ",";
        }
        result << temp.str();
    }
}

void Choice::toSequence(std::stringstream & /*result*/) const {
    throw CException("The regular expression that contains KleeneIterations cannot be converted to "
                     "a single scenario.");
}

void Sequence::toSequence(std::stringstream &result) const {
    first->toSequence(result);
    auto *b = dynamic_cast<Blank *>((this->getFirst()).get());
    if (b == nullptr) {
        result << ',';
    }

    second->toSequence(result);
}
void OmegaSequence::toSequence(std::stringstream &result) const { withOmega->toSequence(result); }

void OmegaChoice::toSequence(std::stringstream & /*result*/) const {
    throw CException("The regular expression that contains KleeneIterations cannot be converted to "
                     "a single scenario.");
}

std::unique_ptr<EdgeLabeledScenarioFSMInterface>
Blank::convertToFSM(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const {
    fsmBuilder.setLatestInterface(nullptr);
    return nullptr;
}

std::unique_ptr<EdgeLabeledScenarioFSMInterface>
Primitive::convertToFSM(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const {

    std::unique_ptr<EdgeLabeledScenarioFSMInterface> newInterface =
            fsmBuilder.buildFromScenario(this->getName());
    fsmBuilder.setLatestInterface(newInterface.get());
    return newInterface;
}

std::unique_ptr<EdgeLabeledScenarioFSMInterface>
Sequence::convertToFSM(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const {

    std::unique_ptr<EdgeLabeledScenarioFSMInterface> firstInterface = first->convertToFSM(fsmBuilder);
    std::unique_ptr<EdgeLabeledScenarioFSMInterface> secondInterface = second->convertToFSM(fsmBuilder);

    if (firstInterface != nullptr) {
        std::unique_ptr<EdgeLabeledScenarioFSMInterface> newInterface =
                fsmBuilder.seqComposition(firstInterface.get(), secondInterface.get());
        fsmBuilder.setLatestInterface(newInterface.get());
        return newInterface;
    }

    fsmBuilder.setLatestInterface(secondInterface.get());
    return secondInterface;
}

std::unique_ptr<EdgeLabeledScenarioFSMInterface>
Choice::convertToFSM(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const {

    std::unique_ptr<EdgeLabeledScenarioFSMInterface> firstInterface = left->convertToFSM(fsmBuilder);
    std::unique_ptr<EdgeLabeledScenarioFSMInterface> secondInterface = right->convertToFSM(fsmBuilder);
    std::unique_ptr<EdgeLabeledScenarioFSMInterface> newInterface =
            fsmBuilder.parComposition(firstInterface.get(), secondInterface.get());

    fsmBuilder.setLatestInterface(newInterface.get());
    return newInterface;
}

std::unique_ptr<EdgeLabeledScenarioFSMInterface>
KleeneIteration::convertToFSM(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const {
    std::unique_ptr<EdgeLabeledScenarioFSMInterface> baseInterface = base->convertToFSM(fsmBuilder);
    std::unique_ptr<EdgeLabeledScenarioFSMInterface> newInterface = fsmBuilder.starComposition(baseInterface.get());

    fsmBuilder.setLatestInterface(newInterface.get());
    return newInterface;
}

std::unique_ptr<EdgeLabeledScenarioFSMInterface>
ConstantIteration::convertToFSM(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const {

    std::unique_ptr<EdgeLabeledScenarioFSMInterface> baseInterface = base->convertToFSM(fsmBuilder);
    ;
    std::unique_ptr<EdgeLabeledScenarioFSMInterface> resultInterface = std::move(baseInterface);

    for (unsigned int i = 1; i != this->exponent; i++) {
        baseInterface = base->convertToFSM(fsmBuilder);
        std::unique_ptr<EdgeLabeledScenarioFSMInterface> newInterface =
                fsmBuilder.seqComposition(resultInterface.get(), baseInterface.get());
        resultInterface.reset();
        baseInterface.reset();
        resultInterface = std::move(newInterface);
    }

    fsmBuilder.setLatestInterface(resultInterface.get());
    return resultInterface;
}

/*This conversion ignores the non-omega part of the OmegaSequence. This is because the non-omega
part is represents a set of finite sequences which does not alter the throughput of the FSMSADF. A
complete conversion would require a NFA class that supports Final states. Currently
EdgeLabeledScenarioFSM does not support final states.*/
std::unique_ptr<EdgeLabeledScenarioFSMInterface>
OmegaSequence::convertToFSM(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const {
    std::unique_ptr<EdgeLabeledScenarioFSMInterface> baseInterface =
            this->withOmega->convertToFSM(fsmBuilder);
    std::unique_ptr<EdgeLabeledScenarioFSMInterface> newInterface =
            fsmBuilder.omegaComposition(baseInterface.get());
    baseInterface.reset();

    fsmBuilder.setLatestInterface(newInterface.get());
    return newInterface;
}

std::unique_ptr<EdgeLabeledScenarioFSMInterface>
OmegaChoice::convertToFSM(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const {

    std::unique_ptr<EdgeLabeledScenarioFSMInterface> firstInterface = left->convertToFSM(fsmBuilder);
    std::unique_ptr<EdgeLabeledScenarioFSMInterface> secondInterface = right->convertToFSM(fsmBuilder);
    std::unique_ptr<EdgeLabeledScenarioFSMInterface> newInterface =
            fsmBuilder.parComposition(firstInterface.get(), secondInterface.get());

    firstInterface.reset();
    secondInterface.reset();

    fsmBuilder.setLatestInterface(newInterface.get());
    return newInterface;
}

// check if the entered regular expression is deterministic
bool Blank::checkDeterministic() const { return true; }

bool Primitive::checkDeterministic() const { return true; }

bool Sequence::checkDeterministic() const {
    bool f = first->checkDeterministic();
    bool s = second->checkDeterministic();

    return f && s;
}

bool Choice::checkDeterministic() const { return false; }

bool KleeneIteration::checkDeterministic() const { return false; }

bool ConstantIteration::checkDeterministic() const { return base->checkDeterministic(); }

bool OmegaSequence::checkDeterministic() const {
    bool with = withOmega->checkDeterministic();
    bool without = withoutOmega->checkDeterministic();

    return with && without;
}

bool OmegaChoice::checkDeterministic() const { return false; }

// converts deterministic regular expressions to deterministic finite automata without epsilon
// transitions
std::unique_ptr<EdgeLabeledScenarioFSMInterface>
Blank::convertToDFA(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const {
    fsmBuilder.setLatestInterface(nullptr);
    return nullptr;
}

std::unique_ptr<EdgeLabeledScenarioFSMInterface>
Primitive::convertToDFA(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const {

    std::unique_ptr<EdgeLabeledScenarioFSMInterface> newInterface =
            fsmBuilder.buildFromScenario(this->getName());
    fsmBuilder.setLatestInterface(newInterface.get());
    return newInterface;
}

std::unique_ptr<EdgeLabeledScenarioFSMInterface>
Sequence::convertToDFA(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const {

    std::unique_ptr<EdgeLabeledScenarioFSMInterface> firstInterface = first->convertToDFA(fsmBuilder);
    std::unique_ptr<EdgeLabeledScenarioFSMInterface> secondInterface = second->convertToDFA(fsmBuilder);

    if (firstInterface != nullptr) {
        std::unique_ptr<EdgeLabeledScenarioFSMInterface> newInterface =
                fsmBuilder.epsilonFreeSeqComposition(firstInterface.get(), secondInterface.get());
        firstInterface.reset();
        secondInterface.reset();
        fsmBuilder.setLatestInterface(newInterface.get());
        return newInterface;
    }

    fsmBuilder.setLatestInterface(secondInterface.get());
    return secondInterface;
}

std::unique_ptr<EdgeLabeledScenarioFSMInterface>
Choice::convertToDFA(EdgeLabeledScenarioFSMBuilder & /*fsmBuilder*/) const {
    throw CException("The entered regex is not deterministic.");
    return nullptr;
}

std::unique_ptr<EdgeLabeledScenarioFSMInterface>
KleeneIteration::convertToDFA(EdgeLabeledScenarioFSMBuilder & /*fsmBuilder*/) const {
    throw CException("The entered regex is not deterministic.");
    return nullptr;
}

std::unique_ptr<EdgeLabeledScenarioFSMInterface>
ConstantIteration::convertToDFA(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const {

    std::unique_ptr<EdgeLabeledScenarioFSMInterface> baseInterface = base->convertToDFA(fsmBuilder);
    ;
    std::unique_ptr<EdgeLabeledScenarioFSMInterface> resultInterface = std::move(baseInterface);

    for (unsigned int i = 1; i != this->exponent; i++) {
        baseInterface = base->convertToDFA(fsmBuilder);
        std::unique_ptr<EdgeLabeledScenarioFSMInterface> newInterface =
                fsmBuilder.epsilonFreeSeqComposition(resultInterface.get(), baseInterface.get());
        resultInterface.reset();
        baseInterface.reset();
        resultInterface = std::move(newInterface);
    }

    fsmBuilder.setLatestInterface(resultInterface.get());
    return resultInterface;
}

/*This conversion ignores the non-omega part of the OmegaSequence. This is because the non-omega
part is represents a set of finite sequences which does not alter the throughput of the FSMSADF. A
complete conversion would require a NFA class that supports Final states. Currently
EdgeLabeledScenarioFSM does not support final states.*/
std::unique_ptr<EdgeLabeledScenarioFSMInterface>
OmegaSequence::convertToDFA(EdgeLabeledScenarioFSMBuilder &fsmBuilder) const {
    std::unique_ptr<EdgeLabeledScenarioFSMInterface> baseInterface =
            this->withOmega->convertToDFA(fsmBuilder);
    std::unique_ptr<EdgeLabeledScenarioFSMInterface> newInterface =
            fsmBuilder.epsilonFreeOmegaComposition(baseInterface.get());
    baseInterface.reset();

    fsmBuilder.setLatestInterface(newInterface.get());
    return newInterface;
}

std::unique_ptr<EdgeLabeledScenarioFSMInterface>
OmegaChoice::convertToDFA(EdgeLabeledScenarioFSMBuilder & /*fsmBuilder*/) const {
    throw CException("The entered regex is not deterministic.");
    return nullptr;
}

// compute the length of the entered expression when represented by Polish Notation
unsigned int Blank::computeLength() const { return 0; }

// assuming its name can be represented by one character
unsigned int Primitive::computeLength() const { return 1; }

unsigned int KleeneIteration::computeLength() const {
    unsigned int b = this->base->computeLength();
    return b + 1;
}

// assuming a binary representation for a number
unsigned int ConstantIteration::computeLength() const {
    unsigned int b = this->base->computeLength();

    auto rep = static_cast<float>(log(this->exponent) / log(2));
    int repCeil = static_cast<int>(ceil(rep));
    if (repCeil == static_cast<int>(rep)) {
        b = b + repCeil + 2;
    } else {
        b = b + repCeil + 1;
}
    return b;
}

unsigned int Sequence::computeLength() const {
    unsigned int f = this->first->computeLength();
    unsigned int s = this->second->computeLength();

    auto *b = dynamic_cast<Blank *>((this->getFirst()).get());
    if (b != nullptr) {
        return s;
}
    return f + s + 1;
}

unsigned int Choice::computeLength() const {
    unsigned int l = this->left->computeLength();
    unsigned int r = this->right->computeLength();
    return l + r + 1;
}

unsigned int OmegaSequence::computeLength() const {
    unsigned int with = this->withOmega->computeLength();
    unsigned int without = this->withoutOmega->computeLength();
    return with + without + 2;
}

unsigned int OmegaChoice::computeLength() const {
    unsigned int l = this->left->computeLength();
    unsigned int r = this->right->computeLength();
    return l + r + 1;
}

std::vector<std::string> Blank::extractAlphabet() const {
    std::vector<std::string> s;
    return s;
}

std::vector<std::string> Primitive::extractAlphabet() const {
    std::vector<std::string> s;
    s.push_back(this->getName());
    return s;
}

std::vector<std::string> KleeneIteration::extractAlphabet() const {
    return this->base->extractAlphabet();
}

std::vector<std::string> ConstantIteration::extractAlphabet() const {
    return this->base->extractAlphabet();
}

std::vector<std::string> Sequence::extractAlphabet() const {
    std::vector<std::string> f = this->first->extractAlphabet();
    std::vector<std::string> s = this->second->extractAlphabet();
    f.insert(f.end(), s.begin(), s.end());

    // remove repeated letters
    std::vector<std::string>::iterator it;
    it = std::unique(f.begin(), f.end());
    f.resize(std::distance(f.begin(), it));

    return f;
}

std::vector<std::string> Choice::extractAlphabet() const {
    std::vector<std::string> l = this->left->extractAlphabet();
    std::vector<std::string> r = this->right->extractAlphabet();
    l.insert(l.end(), r.begin(), r.end());

    // remove repeated letters
    std::vector<std::string>::iterator it;
    it = std::unique(l.begin(), l.end());
    l.resize(std::distance(l.begin(), it));

    return l;
}

std::vector<std::string> OmegaSequence::extractAlphabet() const {
    std::vector<std::string> with = this->withOmega->extractAlphabet();
    std::vector<std::string> without = this->withoutOmega->extractAlphabet();
    with.insert(with.end(), without.begin(), without.end());

    // remove repeated letters
    std::vector<std::string>::iterator it;
    it = std::unique(with.begin(), with.end());
    with.resize(std::distance(with.begin(), it));

    return with;
}

std::vector<std::string> OmegaChoice::extractAlphabet() const {
    std::vector<std::string> l = this->left->extractAlphabet();
    std::vector<std::string> r = this->right->extractAlphabet();
    l.insert(l.end(), r.begin(), r.end());

    // remove repeated letters
    std::vector<std::string>::iterator it;
    it = std::unique(l.begin(), l.end());
    l.resize(std::distance(l.begin(), it));

    return l;
}

const Primitive *Blank::moveToNextLeaf() const { return nullptr; }

const Primitive *Primitive::moveToNextLeaf() const { return this; }

const Primitive *KleeneIteration::moveToNextLeaf() const { return this->base->moveToNextLeaf(); }

const Primitive *ConstantIteration::moveToNextLeaf() const { return this->base->moveToNextLeaf(); }

const Primitive *Choice::moveToNextLeaf() const {
    if (this->left->moveToNextLeaf() != nullptr) {
        return left->moveToNextLeaf();
    }
        return this->right->moveToNextLeaf();
}

const Primitive *Sequence::moveToNextLeaf() const {
    if (this->first->moveToNextLeaf() != nullptr) {
        return first->moveToNextLeaf();
    }
        return this->second->moveToNextLeaf();
}

const Primitive *OmegaChoice::moveToNextLeaf() const {
    if (this->left->moveToNextLeaf() != nullptr) {
        return left->moveToNextLeaf();
    }
        return this->right->moveToNextLeaf();
}

const Primitive *OmegaSequence::moveToNextLeaf() const {
    if (this->withoutOmega->moveToNextLeaf() != nullptr) {
        return withoutOmega->moveToNextLeaf();
    }
        return this->withOmega->moveToNextLeaf();
}