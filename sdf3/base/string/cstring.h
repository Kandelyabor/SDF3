/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   cstring.h
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
 * $Id: cstring.h,v 1.2.2.8 2010-08-18 07:51:41 mgeilen Exp $
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

#ifndef BASE_STRING_CSTRING_H
#define BASE_STRING_CSTRING_H

#include "base/basic_types.h"


// Forward class definition
class CString;

// Types
using CStrings = std::list<CString>;
using CStringsIter = CStrings::iterator;

/**
 * CString
 * String container class.
 * Derived from STL library class string.
 */
class CString : public std::string {
public:
    // Constructor
    CString();

    // copy/move constructors and assignment operators
    CString& operator=(const CString&) = default;
    CString(CString&&) = default;
    CString& operator=(CString&&) = default;

    CString(char s); // NOLINT(*explicit*)
    CString(const char *s); // NOLINT(*explicit*)
    CString(const std::string &s); // NOLINT(*explicit*)
    
    CString(const CString &s);

    // Constructor (integer number)
    // explicit CString(int n);
    // explicit CString(unsigned int n);
    explicit CString(int32_t n);
    explicit CString(uint32_t n);
    explicit CString(int64_t n);
    explicit CString(uint64_t n);

    // Constructor (floating number)
    explicit CString(double n);

    // Destructor
    ~CString();

    // Assignment
    CString &operator+=(const CString &s);
    CString &operator+=(char c);
    // CString &operator+=(int n);
    // CString &operator+=(unsigned int n);
    CString &operator+=(int32_t n);
    CString &operator+=(uint32_t n);
    CString &operator+=(int64_t n);
    CString &operator+=(double n);

    // Character access
    char operator[](int n) const { return (c_str())[n]; };

    // Type conversion
    explicit operator const char *() const;
    // explicit operator int() const;
    // explicit operator uint() const;
    explicit operator double() const;
    explicit operator int32_t() const;
    explicit operator uint32_t() const;
    explicit operator int64_t() const;
    explicit operator uint64_t() const;

    // Whitespace
    CString &trim();
    CString &ltrim(); // left-hand side
    CString &rtrim(); // right-hand side

    // Regex
    CString regexReplace(const CString &regex, const CString &replace);
    CString regexReplaceMultiLine(const CString &regex, const CString &replace);

    // Split
    [[nodiscard]] CStrings split(char delim) const;
    static CString join(const CStrings &strl, char delim);
    static CString join(const CStrings &strl, const CString &delim);

    // Replacement
    CString &replace(const CString &s1, const CString &s2, size_type sPos = 0, uint n = 0);

    // Case
    CString &toLower();
    CString &toUpper();

    // TEsting
    bool isDouble();
    bool isNNInteger();
    bool isNNDouble();
};

/**
 * operator+
 * Append operator for CString class.
 */
inline CString operator+(const CString &lhs, const CString &rhs) {
    CString str(lhs);
    str.append(rhs);
    return {str};
}

inline CString operator+(const CString &lhs, const std::string &rhs) {
    CString str(lhs);
    str.append(rhs);
    return {str};
}

inline CString operator+(const CString &lhs, const char *rhs) {
    CString str(lhs);
    str.append(rhs);
    return {str};
}

inline CString operator+(const std::string &lhs, const CString &rhs) {
    CString str(lhs);
    str.append(rhs);
    return {str};
}

inline CString operator+(const char *lhs, const CString &rhs) {
    CString str(lhs);
    str.append(rhs);
    return {str};
}

// Tokenize a string
void stringtok(CStrings &l, const CString &str, const char *tok = " \t\n");

#endif
