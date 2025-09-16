/* testing */
/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   fraction.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 13, 2005
 *
 *  Function        :   Fraction class
 *
 *  History         :
 *      13-07-05    :   Initial version.
 *
 * $Id: fraction.h,v 1.3.2.9 2010-08-02 15:26:04 mgeilen Exp $
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

#ifndef BASE_FRACTION_FRACTION_H
#define BASE_FRACTION_FRACTION_H

#include "base/basic_types.h" // IWYU pragma: keep
#include "base/math/cmath.h"
#include "base/string/cstring.h"
#include <cmath>

class CFraction {
public:
    // Constructor
    explicit CFraction(const int num = 0, const int den = 1) : num(num), den(den) {
        if (den != 0) {
            val = static_cast<double>(num) / static_cast<double>(den);
        } else {
            val = 0.0;
        }
    };
    explicit CFraction(const double v) { doubleToFraction(v, INFINITY); };
    explicit CFraction(CString &f) {

        if (f.find('/') == CString::npos) {
            num = static_cast<int32_t>(f);
            den = 1;
        } else {
            num = static_cast<int32_t>(CString(f.substr(0, f.find('/'))));
            den = static_cast<int32_t>(CString(f.substr(f.find('/') + 1)));
        }
        val = (static_cast<double>(num)) / static_cast<double>(den);
    };

    // Destructor
    ~CFraction() = default;

    // copy/move constructors and assignment operators
    CFraction(const CFraction&) = default;
    CFraction& operator=(const CFraction&) = default;
    CFraction(CFraction&&) = default;
    CFraction& operator=(CFraction&&) = default;


    static constexpr double DoubleToFractionPrecision = 1e-5;

    // Fraction contains a real fraction?
    [[nodiscard]] bool isFraction() const { return fraction; };

    // Fractional parts
    [[nodiscard]] int64_t numerator() const { return num; };
    [[nodiscard]] int64_t denominator() const { return den; };

    // Real value
    [[nodiscard]] double value() const { return val; };

    // Convert double to fraction
    void doubleToFraction(const double v, const double precision = 1e-15) {
        if (precision == INFINITY) {
            fraction = false;
            num = 0;
            den = 1;
            val = v;
        } else {
            int64_t i = 0;
            int64_t a = 0;
            int64_t b = 0;

            // Floating point number
            val = v;

            // Separate integer part from the fractional part
            auto whole = static_cast<int64_t>(val + precision);
            val -= static_cast<double>(whole);
            val = fabs(val);
            double frac = val;
            double diff = frac;
            num = 1;
            den = 0;

            // Compute fraction as sum of reciprocals
            while (diff >= precision) {
                val = 1.0 / val;
                i = static_cast<int64_t>(val + precision);
                val -= static_cast<double>(i);
                if (a != 0) {
                    num = i * num + b;
                }
                den = lround(static_cast<double>(num) / frac);
                diff = fabs((static_cast<double>(num) / static_cast<double>(den)) - frac);
                b = a;
                a = num;
            }

            if (num == den) {
                whole++;
                num = 0;
                den = 1;
            } else if (den == 0) {
                num = 0;
                den = 1;
            }

            // Add integer part to numerator
            num = whole * den + num;

            // Compute double value based on fraction
            val = static_cast<double>(num) / static_cast<double>(den);
            fraction = true;
        }
    };

    // Lowest term
    [[nodiscard]] CFraction lowestTerm() const {
        CFraction tmp;
        int64_t g = gcd(num, den);
        tmp.num = num / g;
        tmp.den = den / g;
        tmp.val = (static_cast<double>(tmp.num)) / static_cast<double>(tmp.den);
        return tmp;
    }

    // Operators
    CFraction operator+(const CFraction &rhs) const {
        CFraction tmp;

        if (!isFraction() || !rhs.isFraction()) {
            return CFraction(value() + rhs.value());
        }

        tmp.num = rhs.den * num + den * rhs.num;
        tmp.den = den * rhs.den;
        return tmp.lowestTerm();
    }

    CFraction operator-(const CFraction &rhs) const {
        CFraction tmp;

        if (!isFraction() || !rhs.isFraction()) {
            return CFraction(value() - rhs.value());
        }

        tmp.num = rhs.den * num - den * rhs.num;
        tmp.den = den * rhs.den;
        return tmp.lowestTerm();
    }

    CFraction operator*(const CFraction &rhs) const {
        CFraction tmp;

        if (!isFraction() || !rhs.isFraction()) {
            return CFraction(value() * rhs.value());
        }

        tmp.num = num * rhs.num;
        tmp.den = den * rhs.den;
        return tmp.lowestTerm();
    }

    CFraction operator/(const CFraction &rhs) const {
        CFraction tmp;

        if (!isFraction() || !rhs.isFraction()) {
            return CFraction(value() / rhs.value());
        }

        tmp.num = num * rhs.den;
        tmp.den = den * rhs.num;
        return tmp.lowestTerm();
    }

    bool operator==(const CFraction &rhs) const {
        if (!isFraction() || !rhs.isFraction()) {
            return value() == rhs.value();
        }

        if (den == 0 && rhs.den == 0 && num == rhs.num) {
            return true;
        }

        if (den == 0 || rhs.den == 0) {
            return false;
        }

        CFraction r = rhs.lowestTerm();
        CFraction l = lowestTerm();

        return r.den == l.den && r.num == l.num;
    }

    bool operator!=(const CFraction &rhs) const { return !(*this == rhs); }

    bool operator>(const CFraction &rhs) const {
        int64_t l = lcm(den, rhs.den);

        if (!isFraction() || !rhs.isFraction()) {
            return value() > rhs.value();
        }

        if (den == 0 || rhs.den == 0) {
            return false;
        }
        if (num == INT64_MAX) {
            return true;
        }
        if ((num * (l / den)) > (rhs.num * (l / rhs.den))) {
            return true;
        }

        return false;
    }

    bool operator<(const CFraction &rhs) const { return !(*this == rhs || *this > rhs); }

    std::ostream &print(std::ostream &out) const {
        if (isFraction()) {
            if (denominator() == 0) {
                out << "NaN";
            } else if (denominator() == 1) {
                out << numerator();
            } else {
                out << numerator() << "/" << denominator();
            }
        } else {
            out << value();
        }

        return out;
    }

    friend std::ostream &operator<<(std::ostream &out, CFraction &f) { return f.print(out); };

private:
    bool fraction{true};
    double val{};
    int64_t num{};
    int64_t den{};
};

using CFractions = std::vector<CFraction>;
using CFractionsIter = CFractions::iterator;
using CFractionsCIter = CFractions::const_iterator;

#endif
