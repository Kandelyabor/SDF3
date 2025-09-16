/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   regexparserfactory.h
 *
 *  Author          :   Hadi Alizadeh Ara (s.h.seyyed.alizadeh@tue.nl)
 *
 *  Date            :   July 30, 2017
 *
 *  Function        :   defines a regular expression parser factory
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

#ifndef SDF3_FSMSADF_REGEX_PARSER_FACTORY_H_INCLUDED
#define SDF3_FSMSADF_REGEX_PARSER_FACTORY_H_INCLUDED

#include "regexfactory.h"
#include <memory>
#include <utility>

namespace FSMSADF {

class RegExParser {

public:
    // constructor
    explicit RegExParser(CString i) : input(std::move(i)) {}

    // destructor
    virtual ~RegExParser() = default;

    RegExParser(const RegExParser &) = delete;
    RegExParser &operator=(const RegExParser &) = delete;
    RegExParser(RegExParser &&) = delete;
    RegExParser &operator=(RegExParser &&) = delete;

    /*A recursive decent regular expression parser for
    an \omega regular expression that is given as a string.
    We use they following syntax over omega regular expressions:
    // o =: r@ | r.o | o1+o2 //
    We use they following syntax over regular expressions:
    // r =: s | r1,r2 | r1+r2 | r* | r^N //
    */
    std::unique_ptr<RegEx> parse() { return omegaRegEx(); }

    // methods to make regex data types
    [[nodiscard]] virtual std::unique_ptr<RegEx> makeOmegaChoice(std::unique_ptr<RegEx> l,
                                                                 std::unique_ptr<RegEx> r) const {
        return std::make_unique<OmegaChoice>(std::move(l), std::move(r));
    }

    [[nodiscard]] virtual std::unique_ptr<RegEx> makeChoice(std::unique_ptr<RegEx> l,
                                                            std::unique_ptr<RegEx> r) const {
        return std::make_unique<Choice>(std::move(l), std::move(r));
    }

    [[nodiscard]] virtual std::unique_ptr<RegEx> makeSequence(std::unique_ptr<RegEx> f,
                                                              std::unique_ptr<RegEx> s) const {
        return std::make_unique<Sequence>(std::move(f), std::move(s));
    }

    [[nodiscard]] virtual std::unique_ptr<RegEx>
    makeOmegaSequence(std::unique_ptr<RegEx> without, std::unique_ptr<RegEx> with) const {
        return std::make_unique<OmegaSequence>(std::move(without), std::move(with));
    }

    [[nodiscard]] virtual std::unique_ptr<RegEx>
    makeKleeneIteration(std::unique_ptr<RegEx> i) const {
        return std::make_unique<KleeneIteration>(std::move(i));
    }

    [[nodiscard]] virtual std::unique_ptr<RegEx> makeConstantIteration(std::unique_ptr<RegEx> i,
                                                                       unsigned int e) const {
        return std::make_unique<ConstantIteration>(std::move(i), e);
    }

    [[nodiscard]] virtual std::unique_ptr<RegEx> makeBlank() const {
        return std::make_unique<Blank>();
    }

    [[nodiscard]] virtual std::unique_ptr<RegEx> makePrimitive(const CString &s) const {
        return std::make_unique<Primitive>(s);
    }

private:
    // Recursive descent parsing internals.
    char peek();
    void eat(char c);
    char next();
    bool more();

    std::unique_ptr<RegEx> omegaRegEx();
    std::unique_ptr<RegEx> subOmegaRegEx();
    std::unique_ptr<RegEx> regEx();
    std::unique_ptr<RegEx> term();
    std::unique_ptr<RegEx> factor();
    std::unique_ptr<RegEx> subFactor();
    std::unique_ptr<RegEx> base();

    CString input;
};

} // namespace FSMSADF

#endif