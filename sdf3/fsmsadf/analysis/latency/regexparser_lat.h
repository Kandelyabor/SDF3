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


#ifndef SDF3_FSMSADF_REGEX_PARSER_LAT_H_INCLUDEDnullptr
#define SDF3_FSMSADF_REGEX_PARSER_LAT_H_INCLUDEDnullptr

#include "fsmsadf/base/regexparserfactory.h"
#include "regex_lat.h"
#include "systemmatrices.h"

namespace FSMSADF
{


#ifdef _MSC_VER
#pragma warning(disable:4250)  /* suppress warning C4250: for instance 'BlankS': inherits 'Blank::Blank::print' via dominance */
#endif


class RegExParserLat : public RegExParser
    {
    public:

        //constructor
        RegExParserLat(const CString& i, const SystemsMatrices& systemsMatrices)
            : RegExParser(i), sm(systemsMatrices)
        {
        }

        //destructor
        ~RegExParserLat() override = default;

        RegExParserLat(const RegExParserLat&) = delete;
        RegExParserLat& operator=(const RegExParserLat&) = delete;
        RegExParserLat(RegExParserLat&&) = delete;
        RegExParserLat& operator=(RegExParserLat&&) = delete;


        //methods to make RegExL types
        [[nodiscard]] std::unique_ptr<RegEx> makeOmegaChoice(std::unique_ptr<RegEx> l, std::unique_ptr<RegEx> r) const override
        {
            return std::make_unique<OmegaChoiceL>(std::move(l), std::move(r));
        }

        [[nodiscard]] std::unique_ptr<RegEx> makeChoice(std::unique_ptr<RegEx> l, std::unique_ptr<RegEx> r) const override
        {
            return std::make_unique<ChoiceL>(std::move(l), std::move(r));
        }

        [[nodiscard]] std::unique_ptr<RegEx> makeSequence(std::unique_ptr<RegEx> f, std::unique_ptr<RegEx> s) const override
        {
            return std::make_unique<SequenceL>(std::move(f), std::move(s));
        }

        [[nodiscard]] std::unique_ptr<RegEx> makeOmegaSequence(std::unique_ptr<RegEx> without, std::unique_ptr<RegEx> with) const override
        {
            return std::make_unique<OmegaSequenceL>(std::move(without), std::move(with));
        }

        [[nodiscard]] std::unique_ptr<RegEx> makeKleeneIteration(std::unique_ptr<RegEx> i) const override
        {
            return std::make_unique<KleeneIterationL>(std::move(i));
        }

        [[nodiscard]] std::unique_ptr<RegEx> makeConstantIteration(std::unique_ptr<RegEx> i, unsigned int e) const override
        {
            return std::make_unique< ConstantIterationL>(std::move(i), e);
        }

        [[nodiscard]]  std::unique_ptr<RegEx> makeBlank() const override
        {
            return std::make_unique<BlankL>();
        }

        [[nodiscard]]  std::unique_ptr<RegEx> makePrimitive(const CString& n) const override
        {
            CString scenarioName = n;

            // check if the scenario name matches a scenario in the graph
            auto smit = sm.systemsMatrices.find(scenarioName);
            if (smit == sm.systemsMatrices.end()) {
                throw ::CException("scenario " + scenarioName + " is not defined");
}

            // copy the state and output matrix of the scenario to the regex
            auto s = std::make_unique<MaxPlus::Matrix>(*(smit->second->getStateMatrix()));
            auto o = std::make_unique<MaxPlus::Matrix>(*(smit->second->getOutputMatrix()));

            return std::make_unique<PrimitiveL>(scenarioName, std::move(s), std::move(o));
        }

    private:

        const SystemsMatrices &sm;

};

} // namespace FSMSADF

#endif