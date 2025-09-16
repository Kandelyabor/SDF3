/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   regexparser_scenarios.h
 *
 *  Author          :   Hadi Alizadeh Ara (s.h.seyyed.alizadeh@tue.nl)
 *
 *  Date            :   July 30, 2017
 *
 *  Function        :   defines a regular expression parser that produces a regex
 *                      that contains scenario information
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

#ifndef SDF3_FSMSADF_REGEX_PARSER_SCENARIOS_H_INCLUDED
#define SDF3_FSMSADF_REGEX_PARSER_SCENARIOS_H_INCLUDED

#include "graph.h"
#include "regex_scenarios.h"
#include "regexparserfactory.h"
#include <memory>

namespace FSMSADF {

class RegExParserScenarios : public RegExParser {
public:
    // constructor
    RegExParserScenarios(const CString &i, const FSMSADF::Graph &g) : RegExParser(i), graph(&g) {}

    // destructor
    ~RegExParserScenarios() override = default;

    RegExParserScenarios(const RegExParserScenarios &) = delete;
    RegExParserScenarios &operator=(const RegExParserScenarios &) = delete;
    RegExParserScenarios(RegExParserScenarios &&) = delete;
    RegExParserScenarios &operator=(RegExParserScenarios &&) = delete;

    // methods to make SRegEx types
    [[nodiscard]] std::unique_ptr<RegEx> makeOmegaChoice(std::unique_ptr<RegEx> l,
                                                         std::unique_ptr<RegEx> r) const override {
        return std::make_unique<OmegaChoiceS>(std::move(l), std::move(r));
    }

    [[nodiscard]] std::unique_ptr<RegEx> makeChoice(std::unique_ptr<RegEx> l,
                                                    std::unique_ptr<RegEx> r) const override {
        return std::make_unique<ChoiceS>(std::move(l), std::move(r));
    }

    [[nodiscard]] std::unique_ptr<RegEx> makeSequence(std::unique_ptr<RegEx> f,
                                                      std::unique_ptr<RegEx> s) const override {
        return std::make_unique<SequenceS>(std::move(f), std::move(s));
    }

    [[nodiscard]] std::unique_ptr<RegEx>
    makeOmegaSequence(std::unique_ptr<RegEx> without, std::unique_ptr<RegEx> with) const override {
        return std::make_unique<OmegaSequenceS>(std::move(without), std::move(with));
    }

    [[nodiscard]] std::unique_ptr<RegEx>
    makeKleeneIteration(std::unique_ptr<RegEx> i) const override {
        return std::make_unique<KleeneIterationS>(std::move(i));
    }

    [[nodiscard]] std::unique_ptr<RegEx> makeConstantIteration(std::unique_ptr<RegEx> i,
                                                               unsigned int e) const override {
        return std::make_unique<ConstantIterationS>(std::move(i), e);
    }

    [[nodiscard]] std::unique_ptr<RegEx> makeBlank() const override {
        return std::make_unique<BlankS>();
    }

    [[nodiscard]] std::unique_ptr<RegEx> makePrimitive(const CString &s) const override {
        const CString &scenarioName = s;
        const Scenario *sce = graph->getScenarioRef(scenarioName);

        CStrings pt = FSMSADF::Graph::getPersistentTokensOfScenario(*sce);

        CStrings ptf = FSMSADF::Graph::getPersistentTokensFinalOfScenario(*sce);
        return std::make_unique<PrimitiveS>(scenarioName, pt, ptf);
    }

private:
    const FSMSADF::Graph *graph;
};

} // namespace FSMSADF

#endif