/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   cstring.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   September 26, 2003
 *
 *  Function        :   String class
 *
 *  History         :
 *      26-09-03    :   Initial version.
 *
 * $Id: cstring.cc,v 1.1.1.1.2.8 2010-08-18 07:51:40 mgeilen Exp $
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

#include "cstring.h"
#include <algorithm>
#include <cctype>
#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <regex>
#include <sstream>


//NOLINTBEGIN(*-decay,*vararg,cert-err33-c)

constexpr size_t BufferSize = 32;

/**
 * CString ()
 * Constructor.
 */
CString::CString() = default;

/**
 * CString ()
 * Constructor.
 */
CString::CString(const char s) : std::string(1, s) {}

/**
 * CString ()
 * Constructor.
 */
CString::CString(const char *s) : std::string(s) {}

/**
 * CString ()
 * Constructor.
 */
CString::CString(const std::string &s) : std::string(s) {}

/**
 * CString ()
 * Constructor.
 */
CString::CString(const CString &s) = default;

// /**
//  * CString ()
//  * Constructor.
//  */
// CString::CString(const int n) {
//     char str[BufferSize];
//     sprintf(&str[0], "%i", n);
//     append(std::string(str));
// }

// /**
//  * CString ()
//  * Constructor.
//  */
// CString::CString(const unsigned int n) {
//     char str[BufferSize];
//     sprintf(&str[0], "%u", n);
//     append(std::string(str));
// }

/**
 * CString ()
 * Constructor.
 */
CString::CString(const int32_t n) {
    char str[BufferSize];
    sprintf(&str[0], "%d", n);
    append(std::string(str));
}

/**
 * CString ()
 * Constructor.
 */
CString::CString(const uint32_t n) {
    char str[BufferSize];
    sprintf(&str[0], "%u", n);
    append(std::string(str));
}

/**
 * CString ()
 * Constructor.
 */
CString::CString(const int64_t n) {
    char str[BufferSize];
    sprintf(&str[0], "%" PRId64, n);
    append(std::string(str));
}

/**
 * CString ()
 * Constructor.
 */
CString::CString(const uint64_t n) {
    char str[BufferSize];
    sprintf(&str[0], "%" PRIu64, n);
    append(std::string(str));
}

/**
 * CString ()
 * Constructor.
 */
CString::CString(const double n) {
    char str[BufferSize];
    sprintf(&str[0], "%g", n);
    append(std::string(str));
}

/**
 * ~CString ()
 * Destructor.
 */
CString::~CString() = default;

/**
 * operator+= ()
 * Addition to string
 */
CString &CString::operator+=(const CString &s) {
    append(s);
    return *this;
}

/**
 * operator+= ()
 * Addition to string
 */
CString &CString::operator+=(const char c) {
    push_back(c);
    return *this;
}

// /**
//  * operator+= ()
//  * Addition to string
//  */
// CString &CString::operator+=(const int n) {
//     CString str(n);
//     append(str);

//     return *this;
// }

// /**
//  * operator+= ()
//  * Addition to string
//  */
// CString &CString::operator+=(const unsigned int n) {
//     CString str(n);
//     append(str);

//     return *this;
// }

/**
 * operator+= ()
 * Addition to string
 */
CString &CString::operator+=(const int32_t n) {
    CString str(n);
    append(str);

    return *this;
}

/**
 * operator+= ()
 * Addition to string
 */
CString &CString::operator+=(const uint32_t n) {
    CString str(n);
    append(str);

    return *this;
}

/**
 * operator+= ()
 * Addition to string
 */
CString &CString::operator+=(const int64_t n) {
    CString str(n);
    append(str);

    return *this;
}

/**
 * operator+= ()
 * Addition to string
 */
CString &CString::operator+=(const double n) {
    CString str(n);
    append(str);

    return *this;
}

/**
 * operator const char* ()
 * Type conversion to constant character pointer.
 */
CString::operator const char *() const { return c_str(); }

// /**
//  * operator int ()
//  * Type conversion to integer.
//  */
// CString::operator int() const { return strtol(c_str(), nullptr, 0); }

// /**
//  * operator uint ()
//  * Type conversion to unsigned integer.
//  */
// CString::operator uint() const { return strtoul(c_str(), nullptr, 0); }

/**
 * operator double ()
 * Type conversion to double.
 */
CString::operator double() const { return strtod(c_str(), nullptr); }

/**
 * operator long ()
 * Type conversion to long.
 */
CString::operator int32_t() const { return strtol(c_str(), nullptr, 0); }

/**
 * operator unsigned long ()
 * Type conversion to unsigned long.
 */
CString::operator uint32_t() const { return strtoul(c_str(), nullptr, 0); }

/**
 * operator long long ()
 * Type conversion to long long.
 */
CString::operator int64_t() const {
#ifdef _MSC_VER
    return _strtoi64(c_str(), nullptr, 0);
#else
    return strtoll(c_str(), nullptr, 0);
#endif
}

/**
 * operator unsigned long long ()
 * Type conversion to unsigned long long.
 */

CString::operator uint64_t() const {
#ifdef _MSC_VER
    return _strtoui64(c_str(), nullptr, 0);
#else
    return strtoull(c_str(), nullptr, 0);
#endif
}

/**
 * trim ()
 * Remove whitespace from left-hand and right-hand side of string.
 */
CString &CString::trim() {
    ltrim();
    rtrim();

    return *this;
}

/**
 * ltrim ()
 * Remove whitespace from left-hand side of string.
 */
CString &CString::ltrim() {

    CString::size_type startPos = 0;

    // Find first non-whitespace character in the string (from left)
    while (startPos < length() && (isspace(at(startPos)) != 0)) {
        startPos++;
    }

    if (startPos == length()) {
        assign("");
    } else {
        assign(substr(startPos, length()));
    }
    return *this;
}

/**
 * rtrim ()
 * Remove whitespace from right-hand side of string.
 */
CString &CString::rtrim() {
    CString::size_type endPos = length() - 1;

    // Find first non-whitespace character in the string (from right)
    while (endPos < length() && (isspace(at(endPos)) != 0)) {
        endPos--;
    }

    if (endPos > length()) {
        assign("");
    } else {
        assign(substr(0, endPos + 1));
    }

    return *this;
}

/**
 * split ()
 * Split the string on all occurrences of delim character
 */
CStrings CString::split(const char delim) const {
    CStrings strings;
    CString::size_type curDelim = 0;
    CString::size_type nextDelim = 0;

    do {
        // Position the delimiters
        curDelim = nextDelim;
        nextDelim = find(delim, curDelim);

        // Add substring to list
        if (nextDelim >= curDelim && curDelim != length()) {
            strings.push_back(CString(substr(curDelim, (nextDelim - curDelim))));
        }

        // Advance nextDelim position to always make progress
        if (nextDelim != CString::npos) {
            nextDelim++;
        }
    } while (nextDelim != CString::npos);

    return strings;
}

/**
 * join ()
 * Join the list of strings delimited with delim character
 */
CString CString::join(const CStrings &strl, const char delim) {
    CString res {""};
    bool first = true;
    for (const auto &si : strl) {
        if (!first) {
            res += delim;
        }
        res += si;
        first = false;
    }
    return res;
}

/**
 * join ()
 * Join the list of strings delimited with delim character
 */
CString CString::join(const CStrings &strl, const CString &delim) {
    CString res {""};
    bool first = true;
    for (const auto &si : strl) {
        if (!first) {
            res += delim;
        }
        res += si;
        first = false;
    }
    return res;
}

/**
 * replace ()
 * Replace first n occurrences of string s1 with string s2 starting from
 * position sPos. If n is equal to zero, all occurrences are replaced
 */
CString &
CString::replace(const CString &s1, const CString &s2, const size_type sPos, const uint n) {
    size_type pos = sPos;
    uint nrReplaced = 0;

    for (pos = find(s1, pos); pos != CString::npos; pos = find(s1, pos + s2.length())) {
        std::string::replace(pos, s1.size(), s2.c_str());
        nrReplaced++;

        if (n > 0 && nrReplaced == n) {
            break;
        }
    }

    return *this;
}

namespace {


/**
 * istok ()
 * The function returns true if the character is a valid token, else
 * the function returns false.
 */
bool istok(const char c, const char *tok) { return (strchr(tok, c) != nullptr); }

} // namespace

/**
 * stringtok ()
 * Split a string into tokens using the given token delimiter.
 */
void stringtok(CStrings &l, const CString &str, const char *tok) {
    const CString::size_type S = str.size();
    int i = 0;

    // Clear list of strings
    l.clear();

    // Split string
    while (i < S) {
        // eat leading whitespace
        while ((i < S) && (istok(str[i], tok))) {
            ++i;
        }
        if (i == S) {
            return; // nothing left but WS
        }

        // find end of word
        int j = i + 1;
        while ((j < S) && (!istok(str[j], tok))) {
            ++j;
        }

        // add word
        l.emplace_back(str.substr(i, j - i));

        // set up for next loop
        i = j + 1;
    }
}

/**
 * toLower ()
 * The function converts the string to lower-case.
 */
CString &CString::toLower() {
    CString s = *this;
    std::transform(s.begin(), s.end(), s.begin(), static_cast<int (*)(int)>(std::tolower));
    assign(s);

    return *this;
}

/**
 * toUpper ()
 * The function converts the string to upper-case.
 */
CString &CString::toUpper() {
    CString s = *this;
    std::transform(s.begin(), s.end(), s.begin(), static_cast<int (*)(int)>(std::toupper));
    assign(s);

    return *this;
}

/**
 * isNNInteger ()
 * Check if the string represents a non-negative integer number (leading + is not allowed)
 */

bool CString::isNNInteger() {
    // naive implementation, check if the string includes a decimal separator
    // todo: replace with something smarter

    CString::size_type pos = 0;

    // Find first non-whitespace and non-digit character in the string
    while (pos < length() && ((isspace(at(pos)) != 0) || (isdigit(at(pos)) != 0))) {
        pos++;
    }

    // If we didn't find any, the string is a non-negative integer
    return pos == length();
}

/**
 * isNNDouble ()
 * Check if the string represents a float
 */

bool CString::isDouble() {
    try {
        float value = std::stof(*this);
        return true;
    } catch (const std::invalid_argument &e) {
        return false;
    } catch (const std::out_of_range &e) {
        return false;
    }
}

/**
 * isNNDouble ()
 * Check if the string represents a non-negative float
 */

bool CString::isNNDouble() {
    try {
        float value = std::stof(*this);
        return value >= 0;
    } catch (const std::invalid_argument &e) {
        return false;
    } catch (const std::out_of_range &e) {
        return false;
    }
}


/**
 * regexReplace()
 */
CString CString::regexReplace(const CString &regex, const CString &replace) {
    try {
        std::regex pattern(regex);
        CString res = CString(regex_replace(*this, pattern, std::string(replace)));
        return res;
    } catch (std::runtime_error &e) {
        /// @todo add cexceptions to string class?
        std::cout << "Failed regex: " << e.what() << std::endl;
        //      throw CException(CString("Failed to compile and execute regex
        //      expression.")+e.what());
        return *this;
    }
}

/**
 * regexReplaceMultiline()
 */
CString CString::regexReplaceMultiLine(const CString &regex, const CString &replace) {
    try {
        std::regex pattern(regex);

        std::stringstream ss(*this);
        std::ostringstream result;
        std::string line;

        while (std::getline(ss, line, '\n')) {
            std::string res = regex_replace(line, pattern, std::string(replace));
            result << res << '\n';
        }

        return CString(result.str());
    } catch (std::runtime_error &e) {
        /// @todo add cexceptions to string class?
        std::cout << "Failed regex: " << e.what() << std::endl;
        //      throw CException(CString("Failed to compile and execute regex
        //      expression.")+e.what());
        return *this;
    }
}

//NOLINTEND(*-decay,*vararg,cert-err33-c)