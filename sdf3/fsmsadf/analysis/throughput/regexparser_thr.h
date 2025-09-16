/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   regexparser_thr.h
 *
 *  Author          :   Hadi Alizadeh Ara (s.h.seyyed.alizadeh@tue.nl)
 *
 *  Date            :   July 30, 2017
 *
 *  Function        :   defines a regular expression parser that produces a regex
 *                      that contains throughput analysis information
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

#ifndef FSMSADF_REGEXPARSER_THR_H_INCLUDED
#define FSMSADF_REGEXPARSER_THR_H_INCLUDED

#include "fsmsadf/base/regexparserfactory.h"
#include "regex_thr.h"

namespace FSMSADF {

#ifdef _MSC_VER
#pragma warning(disable : 4250) /* suppress warning C4250: for instance 'BlankS': inherits         \
                                   'Blank::Blank::print' via dominance */
#endif

class RegExParserThr : public RegExParser {
public:
    // constructor
    RegExParserThr(const CString &i, const ScenarioMatrices &s, const ScenarioRewards &r) :
        RegExParser(i), sm(&s), sr(&r) {}

    // destructor
    ~RegExParserThr() override = default;

    // Delete copy/move constructors and assignment operators
    RegExParserThr(const RegExParserThr &) = delete;
    RegExParserThr &operator=(const RegExParserThr &) = delete;
    RegExParserThr(RegExParserThr &&) = delete;
    RegExParserThr &operator=(RegExParserThr &&) = delete;

    // methods to make SRegEx types
    [[nodiscard]] std::unique_ptr<RegEx> makeOmegaChoice(std::unique_ptr<RegEx> l,
                                           std::unique_ptr<RegEx> r) const override {
        return std::make_unique<OmegaChoiceT>(std::move(l), std::move(r));
    }

    [[nodiscard]] std::unique_ptr<RegEx> makeChoice(std::unique_ptr<RegEx> l,
                                      std::unique_ptr<RegEx> r) const override {
        return std::make_unique<ChoiceT>(std::move(l), std::move(r));
    }

    [[nodiscard]] std::unique_ptr<RegEx> makeSequence(std::unique_ptr<RegEx> f,
                                        std::unique_ptr<RegEx> s) const override {
        return std::make_unique<SequenceT>(std::move(f), std::move(s));
    }

    [[nodiscard]] std::unique_ptr<RegEx> makeOmegaSequence(std::unique_ptr<RegEx> without,
                                             std::unique_ptr<RegEx> with) const override {
        return std::make_unique<OmegaSequenceT>(std::move(without), std::move(with));
    }

    [[nodiscard]] std::unique_ptr<RegEx> makeKleeneIteration(std::unique_ptr<RegEx> i) const override {
        return std::make_unique<KleeneIterationT>(std::move(i));
    }

    [[nodiscard]] std::unique_ptr<RegEx> makeConstantIteration(std::unique_ptr<RegEx> i,
                                                 unsigned int e) const override {
        return std::make_unique<ConstantIterationT>(std::move(i), e);
    }

    [[nodiscard]] std::unique_ptr<RegEx> makeBlank() const override {
        return std::make_unique<BlankT>();
    }

    [[nodiscard]] std::unique_ptr<RegEx> makePrimitive(const CString &s) const override {
        const CString &scenarioName = s;

        auto smIt = sm->find(scenarioName);
        auto srIt = sr->find(scenarioName);
        if (smIt == sm->end()) {
            throw CException("scenario " + scenarioName + " is not defined");
        }
        auto m = std::make_unique<MaxPlus::Matrix>(*(smIt->second));
        if (srIt == sr->end()) {
            throw CException("scenario " + scenarioName + " does not have a reward");
        }
        CDouble r = srIt->second;
        return std::make_unique<PrimitiveT>(scenarioName, std::move(m), r, scenarioName);
    }

private:
    const FSMSADF::ScenarioMatrices* sm;
    const FSMSADF::ScenarioRewards* sr;
};

} // namespace FSMSADF

#endif