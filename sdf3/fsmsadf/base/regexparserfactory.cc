/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   regex_scenarios.cc
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

#include "regexparserfactory.h"

namespace FSMSADF {

// returns the first char from left in the input string
char RegExParser::peek() {
    CString::iterator ci = this->input.begin();
    while (more() && (isspace(*ci) != 0)) {
        this->input = CString(input.substr(1));
        ci = this->input.begin();
    }
    return *ci;
}

// removes the given char from the left side of the input string
void RegExParser::eat(char c) {
    if (peek() == c) {
        this->input = CString(input.substr(1));
    } else {
        throw CException(CString("Expected: " + std::string(1, c) + "; got : " + peek()));
    }
}

// returns the first char from left in the input string and removes it from the string
char RegExParser::next() {
    char c = peek();
    eat(c);
    return c;
}

// checks if there are any characters left in the input string
bool RegExParser::more() { return input.length() > 0; }

// parse a.b@ + c.d@
std::unique_ptr<RegEx> RegExParser::omegaRegEx() {
    std::unique_ptr<RegEx> subOmegaRegex = subOmegaRegEx();
    if (more() && peek() == '+') {
        eat('+');
        std::unique_ptr<RegEx> omegaRegex = omegaRegEx();
        return makeOmegaChoice(std::move(subOmegaRegex), std::move(omegaRegex));
    }

    return subOmegaRegex;
};

// parse a.b@
std::unique_ptr<RegEx> RegExParser::subOmegaRegEx() {
    std::unique_ptr<RegEx> regex = regEx();
    eat('.');
    std::unique_ptr<RegEx> omegaRegex = this->regEx();
    eat('@');
    return makeOmegaSequence(std::move(regex), std::move(omegaRegex));
};

// parse a+b
std::unique_ptr<RegEx> RegExParser::regEx() {
    std::unique_ptr<RegEx> t = term();
    if (more() && peek() == '+') {
        eat('+');
        std::unique_ptr<RegEx> regex = regEx();
        return makeChoice(std::move(t), std::move(regex));
    }
    return t;
};

// parse a.b
std::unique_ptr<RegEx> RegExParser::term() {
    std::unique_ptr<RegEx> f = makeBlank();
    while (more() && peek() != ')' && peek() != '+' && peek() != '.' && peek() != '@') {
        std::unique_ptr<RegEx> nextFactor = factor();
        f = makeSequence(std::move(f), std::move(nextFactor));
        if (peek() == ',') {
            eat(',');
        }
    }
    return f;
};

// parse a*
std::unique_ptr<RegEx> RegExParser::factor() {
    std::unique_ptr<RegEx> sf = subFactor();
    while (more() && peek() == '*') {
        eat('*');
        sf = makeKleeneIteration(std::move(sf));
    }
    return sf;
};

// parse a^#
std::unique_ptr<RegEx> RegExParser::subFactor() {
    std::unique_ptr<RegEx> b = base();
    if (more() && peek() == '^') {
        eat('^');
        CString exponent;
        if (isdigit(peek()) == 0) {
            throw CException("Expected a digit after ^ ; got: " + CString(peek()));
        }

        while (more() && (isdigit(peek()) != 0)) {
            exponent = exponent + CString(peek());
            next();
        }

        char *pEnd = nullptr;
        auto expVal = std::strtod(exponent.c_str(), &pEnd);
        b = makeConstantIteration(std::move(b), static_cast<unsigned int>(expVal));
    }
    return b;
};

// parse a
std::unique_ptr<RegEx> RegExParser::base() {
    switch (peek()) {
    case '(': {
        eat('(');
        std::unique_ptr<RegEx> r = regEx();
        eat(')');
        return r;
    }
    default: {
        CString name;
        while (more() && peek() != ')' && peek() != '+' && peek() != '.' && peek() != '@'
               && peek() != ',' && peek() != '*' && peek() != '^') {
            name = name + CString(peek());
            next();
        }
        return makePrimitive(name);
    }
    }
};

} // namespace FSMSADF
